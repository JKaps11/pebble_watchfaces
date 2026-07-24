"""Tests for reading a Variant's design-axis positions out of its source.

A Contact sheet tile is labelled from this, so a wrong answer here means a
designer comparing six pictures is told the wrong thing about what they are
looking at. Off-vocabulary positions must be errors, not labels.
"""

import unittest

from studio import axes, render

COMPLETE = ('// Axes: time-display=digital, composition=centred, '
            'complications=0, hue=mono, type=Gothic, polarity=dark-on-light')


class ReadingPositionsTest(unittest.TestCase):
    def test_every_axis_is_read(self):
        self.assertEqual(axes.positions_in(COMPLETE), {
            'time-display': 'digital',
            'composition': 'centred',
            'complications': '0',
            'hue': 'mono',
            'type': 'Gothic',
            'polarity': 'dark-on-light',
        })

    def test_a_declaration_is_found_among_other_source(self):
        source = '// plain — the reference Variant.\n//\n{}\n\n#include <pebble.h>\n'.format(
            COMPLETE)
        self.assertEqual(axes.positions_in(source)['type'], 'Gothic')

    def test_a_declaration_may_wrap_across_comment_lines(self):
        source = ('// Axes: time-display=analogue, composition=split,\n'
                  '//       complications=1-2, hue=one-hue, type=LECO,\n'
                  '//       polarity=light-on-dark\n'
                  '#include <pebble.h>\n')
        self.assertEqual(axes.positions_in(source)['polarity'], 'light-on-dark')

    def test_a_missing_declaration_is_an_error(self):
        with self.assertRaises(axes.AxesError):
            axes.positions_in('#include <pebble.h>\n')

    def test_a_missing_axis_is_an_error_naming_it(self):
        source = '// Axes: time-display=digital, composition=centred\n'
        with self.assertRaises(axes.AxesError) as raised:
            axes.positions_in(source)
        self.assertIn('complications', str(raised.exception))

    def test_an_unknown_axis_is_an_error(self):
        with self.assertRaises(axes.AxesError) as raised:
            axes.positions_in(COMPLETE + ', animation=none')
        self.assertIn('animation', str(raised.exception))

    def test_a_position_off_the_vocabulary_is_an_error(self):
        source = COMPLETE.replace('type=Gothic', 'type=Helvetica')
        with self.assertRaises(axes.AxesError) as raised:
            axes.positions_in(source)
        self.assertIn('Helvetica', str(raised.exception))

    def test_a_field_that_is_not_a_pair_is_an_error(self):
        with self.assertRaises(axes.AxesError):
            axes.positions_in(COMPLETE + ', mono')


class LabelTest(unittest.TestCase):
    def test_a_label_names_the_variant_and_its_positions(self):
        label = axes.label('plain', axes.positions_in(COMPLETE))
        self.assertTrue(label.startswith('plain\n'))
        for position in ('digital', 'centred', 'mono', 'Gothic',
                         'dark-on-light'):
            self.assertIn(position, label)

    def test_positions_are_read_out_in_a_fixed_order(self):
        label = axes.label('plain', axes.positions_in(COMPLETE))
        positions = ' · '.join(label.split('\n')[1:]).split(' · ')
        self.assertEqual(len(positions), len(axes.AXIS_ORDER))
        self.assertEqual(positions[0], 'digital')         # time-display first
        self.assertEqual(positions[-1], 'dark-on-light')  # polarity last

    def test_the_caption_wraps_so_it_fits_under_its_tile(self):
        # Six positions on one line are wider than the 200 px render they label.
        label = axes.label('plain', axes.positions_in(COMPLETE))
        self.assertEqual(len(label.split('\n')), 3)


class DifferingAxesTest(unittest.TestCase):
    """Spread varies two or three axes; Sweep exactly one. Both need this."""

    def test_identical_variants_differ_on_nothing(self):
        positions = axes.positions_in(COMPLETE)
        self.assertEqual(axes.differing_axes(positions, positions), ())

    def test_only_the_axes_that_actually_differ_are_reported(self):
        first = axes.positions_in(COMPLETE)
        second = axes.positions_in(
            COMPLETE.replace('type=Gothic', 'type=LECO')
                    .replace('hue=mono', 'hue=multi'))
        self.assertEqual(set(axes.differing_axes(first, second)),
                         {'type', 'hue'})


class EveryVariantDeclaresItselfTest(unittest.TestCase):
    """The labels have to match what was built, so every Variant must declare."""

    def test_every_variant_in_the_studio_has_valid_axis_positions(self):
        for variant in render.available_variants():
            with self.subTest(variant=variant):
                positions = axes.positions_of(variant, render.VARIANTS_DIR)
                self.assertEqual(set(positions), set(axes.AXIS_ORDER))


if __name__ == '__main__':
    unittest.main()
