"""Tests for assembling a Batch.

The rendering itself is covered by the emulator-backed tests in test_render.py.
What is asserted here is everything that can be got wrong without an emulator:
that a malformed Batch is refused before a minute of rendering is spent on it,
that the Contact sheet shows the Canonical renders and only those, and that the
labels come from what each Variant actually declares.
"""

import json
import os
import tempfile
import unittest
from unittest import mock

from studio import axes, batch, png, render, states

from fixtures import HEIGHT, WIDTH

REQUIRES_EMULATOR = unittest.skipUnless(
    os.environ.get('STUDIO_EMULATOR_TESTS'),
    'set STUDIO_EMULATOR_TESTS=1 to run tests that drive the emulator')

SIX = ['plain', 'dense', 'dial', 'nocturne', 'corner', 'halves']


class BatchShapeTest(unittest.TestCase):
    """A Batch is six. The sheet is three across and two down."""

    def test_the_tiling_holds_exactly_one_batch(self):
        self.assertEqual(batch.TILE_COLUMNS * batch.TILE_ROWS, batch.BATCH_SIZE)

    def test_too_few_variants_is_refused(self):
        with self.assertRaises(batch.BatchError) as raised:
            batch.render_batch('session', SIX[:5])
        self.assertIn('6', str(raised.exception))

    def test_too_many_variants_is_refused(self):
        with self.assertRaises(batch.BatchError):
            batch.render_batch('session', SIX + ['plain'])

    def test_an_unknown_variant_is_refused_before_rendering(self):
        with self.assertRaises(batch.BatchError) as raised:
            batch.render_batch('session', SIX[:5] + ['no-such-variant'])
        self.assertIn('no-such-variant', str(raised.exception))



class SeveralBatchesPerSessionTest(unittest.TestCase):
    """A Session is one sitting against a brief and may hold several Batches.

    Asking for another when the first feels thin is the ordinary way to use the
    Studio, so the second must not land on top of the first.
    """

    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        patched = mock.patch.object(batch, 'SESSIONS_DIR', directory.name)
        patched.start()
        self.addCleanup(patched.stop)
        self.session = 'test-session'

    def test_a_fresh_session_starts_at_batch_one(self):
        self.assertEqual(batch.batch_numbers(self.session), [])
        self.assertEqual(batch.next_batch_number(self.session), 1)

    def test_each_batch_gets_its_own_directory(self):
        self.assertNotEqual(batch.batch_dir(self.session, 1),
                            batch.batch_dir(self.session, 2))

    def test_renders_from_different_batches_do_not_collide(self):
        self.assertNotEqual(batch.render_path(self.session, 1, 'plain', 'canonical'),
                            batch.render_path(self.session, 2, 'plain', 'canonical'))

    def test_the_next_batch_follows_the_ones_already_recorded(self):
        for number in (1, 2):
            os.makedirs(batch.batch_dir(self.session, number))
        self.assertEqual(batch.batch_numbers(self.session), [1, 2])
        self.assertEqual(batch.next_batch_number(self.session), 3)

    def test_reading_a_manifest_defaults_to_the_latest_batch(self):
        for number in (1, 2):
            os.makedirs(batch.batch_dir(self.session, number))
            with open(os.path.join(batch.batch_dir(self.session, number),
                                   batch.MANIFEST_NAME), 'w') as handle:
                json.dump({'batch': number, 'variants': []}, handle)

        self.assertEqual(batch.read_manifest(self.session)['batch'], 2)
        self.assertEqual(batch.read_manifest(self.session, 1)['batch'], 1)


class ContactSheetTest(unittest.TestCase):
    """The sheet compares designs, so it must show one state and label each tile."""

    def setUp(self):
        self.session = 'test-session'
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        patched = mock.patch.object(batch, 'SESSIONS_DIR', self.directory.name)
        patched.start()
        self.addCleanup(patched.stop)

        # Stand in for renders: one solid colour per Variant per state, so the
        # sheet's contents can be identified by the colours that end up in it.
        # Drawn at Emery's real framebuffer size, because the sheet's proportions
        # follow the tile size and a 4x4 stand-in would be all caption.
        self.colours = {}
        for index, variant in enumerate(SIX):
            for state_name in states.BY_NAME:
                colour = (10 * index + (0 if state_name == 'canonical' else 5),
                          20, 30)
                self.colours[(variant, state_name)] = colour
                path = batch.render_path(self.session, 1, variant, state_name)
                os.makedirs(os.path.dirname(path), exist_ok=True)
                png.write(
                    png.Image(WIDTH, HEIGHT,
                              tuple((colour,) * WIDTH for _ in range(HEIGHT))),
                    path)

    def test_the_sheet_is_built_from_the_canonical_renders(self):
        sheet = batch.contact_sheet(self.session, 1, SIX)
        present = {colour for _, _, colour in png.read(sheet).pixels()}

        for variant in SIX:
            self.assertIn(self.colours[(variant, 'canonical')], present,
                          '{} is missing from the contact sheet'.format(variant))

    def test_stress_renders_never_reach_the_sheet(self):
        sheet = batch.contact_sheet(self.session, 1, SIX)
        present = {colour for _, _, colour in png.read(sheet).pixels()}

        for variant in SIX:
            self.assertNotIn(self.colours[(variant, 'stress')], present,
                             '{} stress render leaked onto the sheet'.format(
                                 variant))

    def test_the_sheet_is_wider_than_it_is_tall_because_it_tiles_three_by_two(self):
        sheet = png.read(batch.contact_sheet(self.session, 1, SIX))
        self.assertGreater(sheet.width, sheet.height)


class LabelSourceTest(unittest.TestCase):
    """Labels must describe what was built, so they come from the source."""

    def test_each_variant_is_labelled_from_its_own_declaration(self):
        for variant in SIX:
            with self.subTest(variant=variant):
                positions = axes.positions_of(variant, render.VARIANTS_DIR)
                label = axes.label(variant, positions)
                self.assertIn(variant, label)
                self.assertIn(positions['polarity'], label)

    def test_the_batch_spans_more_than_one_position_on_several_axes(self):
        # A Batch of six that all sit in the same place is not a Batch worth
        # looking at; this is the property that makes a Spread a Spread.
        declared = [axes.positions_of(v, render.VARIANTS_DIR) for v in SIX]
        varying = [axis for axis in axes.AXIS_ORDER
                   if len({p[axis] for p in declared}) > 1]
        self.assertGreaterEqual(len(varying), 3, 'batch barely varies: ' + str(varying))


@REQUIRES_EMULATOR
class BatchRendersTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        render.assert_state_injection_works()

    def test_a_batch_produces_both_states_for_every_variant_and_a_sheet(self):
        with tempfile.TemporaryDirectory() as scratch:
            with mock.patch.object(batch, 'SESSIONS_DIR', scratch):
                result = batch.render_batch('smoke', SIX)

                for variant in SIX:
                    for state_name in states.BY_NAME:
                        self.assertTrue(
                            os.path.exists(
                                batch.render_path('smoke', 1, variant,
                                                  state_name)),
                            'missing {} render for {}'.format(state_name, variant))

                self.assertTrue(os.path.exists(result['contact_sheet']))
                with open(os.path.join(batch.batch_dir('smoke', 1),
                                       batch.MANIFEST_NAME)) as handle:
                    manifest = json.load(handle)
                self.assertEqual([v['name'] for v in manifest['variants']], SIX)


if __name__ == '__main__':
    unittest.main()
