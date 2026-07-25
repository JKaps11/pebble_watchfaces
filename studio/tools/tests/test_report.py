"""Tests for the Session report.

The renders here are synthetic, so what each Variant "looks like" is chosen by
the test and the right answer is known. What matters is that a finding reaches
the report only from the render it belongs to: a design that fits at the
Canonical state and clips at the Stress state must be reported as clipping, and
one that crowds the edge only in the Stress render must not be reported for it.
"""

import json
import os
import tempfile
import unittest
from unittest import mock

from studio import batch, report, states

from fixtures import BLACK, HEIGHT, WIDTH, Canvas

# Real Variant names, because the report reads each one's declared axis
# positions out of its source. The pictures are this test's invention.
CLEAN = 'plain'
OFFENDING = 'dense'


def clean_canvas():
    """Two well-separated elements: strong hierarchy, comfortable margins."""
    return (Canvas()
            .rect(40, 30, 120, 60, BLACK)
            .rect(40, 150, 120, 20, BLACK))


def clipping_canvas():
    """A run that reaches the final column, as an overlong date would."""
    return clean_canvas().rect(100, 190, WIDTH - 100, 18, BLACK)


class ReportTestCase(unittest.TestCase):
    def setUp(self):
        self.session = 'test-session'
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        patched = mock.patch.object(batch, 'SESSIONS_DIR', directory.name)
        patched.start()
        self.addCleanup(patched.stop)

    def place(self, variant, state_name, canvas):
        path = batch.render_path(self.session, variant, state_name)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        canvas.write(os.path.dirname(path), os.path.basename(path))

    def place_both(self, variant, canonical, stress):
        self.place(variant, states.CANONICAL.name, canonical)
        self.place(variant, states.STRESS.name, stress)

    def report_text(self, variants, critiques=None):
        path = report.build(self.session, critiques=critiques, variants=variants)
        with open(path) as handle:
            return handle.read()


class WhichRenderEachFindingComesFromTest(ReportTestCase):
    def test_a_variant_that_clips_at_stress_is_reported_as_clipping(self):
        self.place_both(OFFENDING, clean_canvas(), clipping_canvas())
        text = self.report_text([OFFENDING])

        self.assertIn('clipping', text)
        self.assertIn(OFFENDING, text)

    def test_clipping_seen_only_in_the_canonical_render_is_not_reported(self):
        # Clipping is a Stress-state measurement. A clean Stress render means
        # the design does not clip, whatever the Canonical render happens to do.
        self.place_both(OFFENDING, clipping_canvas(), clean_canvas())
        self.assertNotIn('clipping', self.report_text([OFFENDING]))

    def test_a_crowded_edge_is_taken_from_the_canonical_render(self):
        crowded = clean_canvas().rect(2, 100, 40, 12, BLACK)
        self.place_both(OFFENDING, crowded, clean_canvas())
        self.assertIn('safe margin', self.report_text([OFFENDING]))

    def test_a_crowded_edge_only_at_stress_is_not_reported(self):
        crowded = clean_canvas().rect(2, 100, 40, 12, BLACK)
        self.place_both(OFFENDING, clean_canvas(), crowded)
        self.assertNotIn('safe margin', self.report_text([OFFENDING]))


class OnlyFailuresAppearTest(ReportTestCase):
    def test_a_clean_variant_produces_no_findings(self):
        self.place_both(CLEAN, clean_canvas(), clean_canvas())
        text = self.report_text([CLEAN])

        self.assertIn(CLEAN, text)
        self.assertIn('Nothing measured against it', text)

    def test_passing_measurements_do_not_fill_the_report(self):
        self.place_both(CLEAN, clean_canvas(), clean_canvas())
        text = self.report_text([CLEAN])

        for noise in ('ink coverage', 'glance hierarchy', 'contrast'):
            self.assertNotIn(noise, text)

    def test_a_failing_variant_is_called_out_as_not_viable(self):
        self.place_both(CLEAN, clean_canvas(), clean_canvas())
        self.place_both(OFFENDING, clean_canvas(), clipping_canvas())
        text = self.report_text([CLEAN, OFFENDING])

        not_viable = text.split('Not viable as they stand:')[1].split('\n')[0]
        self.assertIn(OFFENDING, not_viable)
        self.assertNotIn(CLEAN, not_viable)


class ReportShapeTest(ReportTestCase):
    def test_the_report_sits_beside_the_contact_sheet_and_links_it(self):
        self.place_both(CLEAN, clean_canvas(), clean_canvas())
        path = report.build(self.session, variants=[CLEAN])

        self.assertEqual(os.path.dirname(path),
                         batch.session_dir(self.session))
        with open(path) as handle:
            self.assertIn(batch.CONTACT_SHEET_NAME, handle.read())

    def test_each_variant_is_named_with_where_it_sits_on_the_axes(self):
        self.place_both(CLEAN, clean_canvas(), clean_canvas())
        text = self.report_text([CLEAN])
        self.assertIn('centred', text)

    def test_written_critique_is_carried_alongside_the_measurements(self):
        # The skill authors the judgment; the report leaves it somewhere to go.
        self.place_both(OFFENDING, clean_canvas(), clipping_canvas())
        text = self.report_text(
            [OFFENDING], critiques={OFFENDING: 'Reads as generic.'})

        self.assertIn('Reads as generic.', text)
        self.assertIn('clipping', text)


class CritiqueAndPickTest(ReportTestCase):
    """The skill's half of the report: judgment, and which one won."""

    def setUp(self):
        super().setUp()
        self.place_both(CLEAN, clean_canvas(), clean_canvas())
        self.place_both(OFFENDING, clean_canvas(), clipping_canvas())
        manifest = {'session': self.session,
                    'variants': [{'name': CLEAN}, {'name': OFFENDING}]}
        path = os.path.join(batch.session_dir(self.session), batch.MANIFEST_NAME)
        with open(path, 'w') as handle:
            json.dump(manifest, handle)

    def write_side_file(self, name, contents):
        with open(os.path.join(batch.session_dir(self.session), name),
                  'w') as handle:
            json.dump(contents, handle)

    def test_critique_written_beside_the_renders_survives_a_rebuild(self):
        # Rebuilding a report from the renders must not discard the judgment
        # written about them, which is why critique is data and not prose edited
        # into the Markdown.
        self.write_side_file(report.CRITIQUE_NAME, {CLEAN: 'Quietly confident.'})
        path = report.build(self.session)

        with open(path) as handle:
            self.assertIn('Quietly confident.', handle.read())

    def test_recording_a_pick_names_the_winner_and_closes_the_session(self):
        path = report.record_pick(self.session, CLEAN, why='Best of the six.')

        with open(path) as handle:
            text = handle.read()
        self.assertIn('## Pick', text)
        self.assertIn(CLEAN, text.split('## Pick')[1])
        self.assertIn('Best of the six.', text)

    def test_a_pick_outside_the_batch_is_refused(self):
        with self.assertRaises(batch.BatchError) as raised:
            report.record_pick(self.session, 'dial')
        self.assertIn('dial', str(raised.exception))

    def test_a_session_with_no_pick_has_no_pick_section(self):
        with open(report.build(self.session)) as handle:
            self.assertNotIn('## Pick', handle.read())

    def test_picking_produces_no_further_artefact(self):
        # Picking records the pick and ends the Session. Nothing else.
        before = set(os.listdir(batch.session_dir(self.session)))
        report.record_pick(self.session, CLEAN)
        after = set(os.listdir(batch.session_dir(self.session)))

        self.assertEqual(after - before, {report.PICK_NAME, report.REPORT_NAME})


if __name__ == '__main__':
    unittest.main()
