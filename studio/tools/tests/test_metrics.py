"""Tests for the metrics module.

Every expected value here is known by construction, not by judgment: a canvas
that is exactly half black asserts 50% ink coverage, a rectangle ending on the
final column asserts clipping. A test in this file should fail only when a
measurement is genuinely wrong, never because a design changed or taste moved.

Assertions are on the measurements the module returns. Nothing here reaches into
how it arrives at them.
"""

import os
import unittest

from studio import metrics

from fixtures import BLACK, HEIGHT, WIDTH, WHITE, Canvas, FixtureDirectory

# Greys whose contrast ratio against white is a published WCAG reference value,
# so the expected numbers below come from the specification rather than from
# running the same formula the module runs.
GREY_7_00 = (0x59, 0x59, 0x59)  # 7.00:1 — the canonical "just passes AAA" grey
GREY_6_90 = (0x5A, 0x5A, 0x5A)  # 6.90:1 — one step darker than white can afford
GREY_5_74 = (0x66, 0x66, 0x66)  # 5.74:1
GREY_4_54 = (0x76, 0x76, 0x76)  # 4.54:1 — the canonical "just passes AA" grey
GREY_4_48 = (0x77, 0x77, 0x77)  # 4.48:1


class InkCoverageTest(unittest.TestCase):
    def setUp(self):
        self.fixtures = FixtureDirectory(self)

    def measure(self, canvas):
        return metrics.measure(self.fixtures.png(canvas))

    def test_blank_render_has_no_ink(self):
        self.assertEqual(self.measure(Canvas()).ink_coverage, 0.0)

    def test_half_black_canvas_is_half_covered(self):
        canvas = Canvas().rect(0, 0, WIDTH, HEIGHT // 2, BLACK)
        self.assertAlmostEqual(self.measure(canvas).ink_coverage, 0.5)

    def test_coverage_is_the_quotient_of_inked_pixels_over_all_pixels(self):
        canvas = Canvas().rect(10, 10, 40, 57, BLACK)  # 2280 of 45600 px
        self.assertAlmostEqual(self.measure(canvas).ink_coverage, 0.05)


class ClippingTest(unittest.TestCase):
    def setUp(self):
        self.fixtures = FixtureDirectory(self)

    def measure(self, canvas):
        return metrics.measure(self.fixtures.png(canvas))

    def test_a_run_ending_one_pixel_inside_the_right_edge_does_not_clip(self):
        canvas = Canvas().rect(100, 100, WIDTH - 1 - 100, 20, BLACK)
        self.assertFalse(self.measure(canvas).clipping)

    def test_a_run_ending_on_the_final_column_clips(self):
        canvas = Canvas().rect(100, 100, WIDTH - 100, 20, BLACK)
        self.assertTrue(self.measure(canvas).clipping)

    def test_a_run_touching_the_first_column_clips(self):
        canvas = Canvas().rect(0, 100, 60, 20, BLACK)
        self.assertTrue(self.measure(canvas).clipping)

    def test_a_run_touching_the_final_row_clips(self):
        canvas = Canvas().rect(50, HEIGHT - 20, 60, 20, BLACK)
        self.assertTrue(self.measure(canvas).clipping)

    def test_a_run_touching_the_first_row_clips(self):
        canvas = Canvas().rect(50, 0, 60, 20, BLACK)
        self.assertTrue(self.measure(canvas).clipping)

    def test_a_blank_render_does_not_clip(self):
        self.assertFalse(self.measure(Canvas()).clipping)


class SafeMarginTest(unittest.TestCase):
    def setUp(self):
        self.fixtures = FixtureDirectory(self)

    def measure(self, canvas):
        return metrics.measure(self.fixtures.png(canvas))

    def test_margin_is_the_distance_from_the_nearest_edge(self):
        canvas = Canvas().rect(20, 40, 60, 60, BLACK)  # nearest edge is x=20
        self.assertEqual(self.measure(canvas).safe_margin, 20)

    def test_margin_is_measured_against_the_closest_of_the_four_edges(self):
        # 12 px from the bottom, further from the other three.
        canvas = Canvas().rect(40, HEIGHT - 12 - 30, 60, 30, BLACK)
        self.assertEqual(self.measure(canvas).safe_margin, 12)

    def test_a_comfortable_margin_is_not_flagged(self):
        canvas = Canvas().rect(9, 40, 60, 60, BLACK)
        self.assertFalse(self.measure(canvas).safe_margin_violated)

    def test_a_margin_exactly_at_the_threshold_is_not_flagged(self):
        canvas = Canvas().rect(8, 40, 60, 60, BLACK)
        self.assertEqual(self.measure(canvas).safe_margin, 8)
        self.assertFalse(self.measure(canvas).safe_margin_violated)

    def test_a_margin_one_pixel_inside_the_threshold_is_flagged(self):
        canvas = Canvas().rect(7, 40, 60, 60, BLACK)
        self.assertEqual(self.measure(canvas).safe_margin, 7)
        self.assertTrue(self.measure(canvas).safe_margin_violated)

    def test_a_blank_render_has_no_margin_to_report(self):
        measured = self.measure(Canvas())
        self.assertIsNone(measured.safe_margin)
        self.assertFalse(measured.safe_margin_violated)


class ElementTest(unittest.TestCase):
    """Elements are what the other measurements are reported against."""

    def setUp(self):
        self.fixtures = FixtureDirectory(self)

    def measure(self, canvas):
        return metrics.measure(self.fixtures.png(canvas))

    def test_two_separated_runs_are_two_elements(self):
        canvas = (Canvas()
                  .rect(40, 30, 120, 60, BLACK)
                  .rect(40, 140, 120, 20, BLACK))
        self.assertEqual(len(self.measure(canvas).elements), 2)

    def test_glyphs_on_one_line_are_one_element(self):
        # Three marks sharing a line, 4 px apart — a word, not three elements.
        canvas = (Canvas()
                  .rect(40, 30, 20, 40, BLACK)
                  .rect(64, 30, 20, 40, BLACK)
                  .rect(88, 30, 20, 40, BLACK))
        measured = self.measure(canvas)
        self.assertEqual(len(measured.elements), 1)
        self.assertEqual(measured.elements[0].height, 40)

    def test_elements_are_returned_tallest_first(self):
        canvas = (Canvas()
                  .rect(40, 30, 120, 20, BLACK)
                  .rect(40, 140, 120, 60, BLACK))
        heights = [element.height for element in self.measure(canvas).elements]
        self.assertEqual(heights, [60, 20])


class GlanceHierarchyTest(unittest.TestCase):
    def setUp(self):
        self.fixtures = FixtureDirectory(self)

    def measure(self, canvas):
        return metrics.measure(self.fixtures.png(canvas))

    def two_elements(self, primary_height, secondary_height):
        return (Canvas()
                .rect(40, 20, 120, primary_height, BLACK)
                .rect(40, 180, 120, secondary_height, BLACK))

    def test_hierarchy_is_the_quotient_of_the_two_tallest_heights(self):
        measured = self.measure(self.two_elements(60, 20))
        self.assertAlmostEqual(measured.glance_hierarchy, 3.0)

    def test_a_dominant_primary_is_not_flagged(self):
        measured = self.measure(self.two_elements(60, 20))
        self.assertFalse(measured.glance_hierarchy_weak)
        self.assertFalse(measured.no_clear_primary)

    def test_a_ratio_exactly_at_the_flag_threshold_is_not_flagged(self):
        measured = self.measure(self.two_elements(30, 20))  # 1.5x exactly
        self.assertAlmostEqual(measured.glance_hierarchy, 1.5)
        self.assertFalse(measured.glance_hierarchy_weak)

    def test_a_ratio_below_the_flag_threshold_is_flagged(self):
        measured = self.measure(self.two_elements(29, 20))  # 1.45x
        self.assertTrue(measured.glance_hierarchy_weak)
        self.assertFalse(measured.no_clear_primary)

    def test_a_ratio_exactly_at_the_no_primary_threshold_has_a_primary(self):
        measured = self.measure(self.two_elements(24, 20))  # 1.2x exactly
        self.assertAlmostEqual(measured.glance_hierarchy, 1.2)
        self.assertTrue(measured.glance_hierarchy_weak)
        self.assertFalse(measured.no_clear_primary)

    def test_a_ratio_below_the_no_primary_threshold_has_no_primary(self):
        measured = self.measure(self.two_elements(23, 20))  # 1.15x
        self.assertTrue(measured.no_clear_primary)

    def test_a_lone_element_is_unambiguously_primary(self):
        canvas = Canvas().rect(40, 30, 120, 60, BLACK)
        measured = self.measure(canvas)
        self.assertIsNone(measured.glance_hierarchy)
        self.assertFalse(measured.glance_hierarchy_weak)
        self.assertFalse(measured.no_clear_primary)


class ContrastTest(unittest.TestCase):
    def setUp(self):
        self.fixtures = FixtureDirectory(self)

    def measure(self, canvas):
        return metrics.measure(self.fixtures.png(canvas))

    def lone_element(self, colour, background=WHITE):
        return Canvas(background).rect(40, 80, 120, 60, colour)

    def test_black_on_white_is_the_maximum_ratio(self):
        measured = self.measure(self.lone_element(BLACK))
        self.assertAlmostEqual(measured.elements[0].contrast, 21.0, places=2)

    def test_contrast_is_measured_against_the_local_ground_not_a_fixed_white(self):
        # Light-on-dark: the ground is black, so the ratio is still 21:1.
        measured = self.measure(self.lone_element(WHITE, background=BLACK))
        self.assertAlmostEqual(measured.elements[0].contrast, 21.0, places=2)

    def test_a_mid_grey_matches_its_published_ratio(self):
        measured = self.measure(self.lone_element(GREY_4_54))
        self.assertAlmostEqual(measured.elements[0].contrast, 4.54, places=2)

    def test_a_dark_grey_matches_its_published_ratio(self):
        measured = self.measure(self.lone_element(GREY_7_00))
        self.assertAlmostEqual(measured.elements[0].contrast, 7.00, places=2)


class ContrastThresholdTest(unittest.TestCase):
    """The time element is held to 7:1; complications to 4.5:1."""

    def setUp(self):
        self.fixtures = FixtureDirectory(self)

    def measure(self, time_colour, complication_colour):
        canvas = (Canvas()
                  .rect(40, 20, 120, 60, time_colour)       # tallest: the time
                  .rect(40, 160, 120, 20, complication_colour))
        return metrics.measure(self.fixtures.png(canvas))

    def test_a_time_element_exactly_at_the_threshold_passes(self):
        measured = self.measure(GREY_7_00, BLACK)
        self.assertAlmostEqual(measured.elements[0].contrast, 7.00, places=2)
        self.assertTrue(measured.elements[0].contrast_ok)
        self.assertEqual(measured.contrast_failures, ())

    def test_a_time_element_below_the_threshold_fails(self):
        measured = self.measure(GREY_6_90, BLACK)
        self.assertAlmostEqual(measured.elements[0].contrast, 6.90, places=2)
        self.assertFalse(measured.elements[0].contrast_ok)
        self.assertEqual(measured.contrast_failures, (measured.elements[0],))

    def test_a_complication_exactly_at_the_threshold_passes(self):
        measured = self.measure(BLACK, GREY_4_54)
        self.assertAlmostEqual(measured.elements[1].contrast, 4.54, places=2)
        self.assertTrue(measured.elements[1].contrast_ok)

    def test_a_complication_below_the_threshold_fails(self):
        measured = self.measure(BLACK, GREY_4_48)
        self.assertAlmostEqual(measured.elements[1].contrast, 4.48, places=2)
        self.assertFalse(measured.elements[1].contrast_ok)
        self.assertEqual(measured.contrast_failures, (measured.elements[1],))

    def test_a_complication_is_not_held_to_the_time_threshold(self):
        # 5.74:1 would fail as a time element and passes as a complication.
        measured = self.measure(BLACK, GREY_5_74)
        self.assertAlmostEqual(measured.elements[1].contrast, 5.74, places=2)
        self.assertTrue(measured.elements[1].contrast_ok)

    def test_the_tallest_element_is_the_one_held_to_the_time_threshold(self):
        measured = self.measure(GREY_5_74, BLACK)
        self.assertFalse(measured.elements[0].contrast_ok)


class InkDensityTest(unittest.TestCase):
    """Coverage is informational — dense is worth saying, never a failure."""

    def setUp(self):
        self.fixtures = FixtureDirectory(self)

    def covering(self, rows):
        canvas = Canvas().rect(0, 0, WIDTH, rows, BLACK)
        return metrics.measure(self.fixtures.png(canvas))

    def test_a_sparse_render_is_not_dense(self):
        self.assertFalse(self.covering(23).ink_dense)  # 10.1%

    def test_coverage_exactly_at_the_threshold_is_not_dense(self):
        measured = self.covering(HEIGHT * 40 // 100)  # 91 rows -> 39.9%
        self.assertFalse(measured.ink_dense)

    def test_coverage_above_the_threshold_is_dense(self):
        measured = self.covering(HEIGHT // 2)  # 50%
        self.assertTrue(measured.ink_dense)

    def test_density_never_produces_a_failure(self):
        findings = self.covering(HEIGHT // 2).findings
        self.assertTrue(any(f.measurement == 'ink coverage' for f in findings))
        self.assertTrue(all(f.measurement != 'ink coverage' or not f.is_failure
                            for f in findings))


class FindingsTest(unittest.TestCase):
    """The report reads findings, so only failures and flags may appear."""

    def setUp(self):
        self.fixtures = FixtureDirectory(self)

    def measure(self, canvas):
        return metrics.measure(self.fixtures.png(canvas))

    def measurements_reported(self, canvas):
        return {f.measurement for f in self.measure(canvas).findings}

    def test_a_clean_render_reports_nothing(self):
        canvas = (Canvas()
                  .rect(40, 20, 120, 60, BLACK)
                  .rect(40, 160, 120, 20, BLACK))
        self.assertEqual(self.measure(canvas).findings, ())

    def test_clipping_is_reported(self):
        canvas = Canvas().rect(0, 100, 60, 20, BLACK)
        self.assertIn('clipping', self.measurements_reported(canvas))

    def test_a_crowded_edge_is_reported(self):
        canvas = Canvas().rect(7, 100, 60, 20, BLACK)
        self.assertIn('safe margin', self.measurements_reported(canvas))

    def test_a_failing_contrast_is_reported_against_the_element_that_failed(self):
        canvas = (Canvas()
                  .rect(40, 20, 120, 60, GREY_6_90)
                  .rect(40, 160, 120, 20, BLACK))
        findings = [f for f in self.measure(canvas).findings
                    if f.measurement == 'contrast']
        self.assertEqual(len(findings), 1)
        self.assertEqual(findings[0].element, self.measure(canvas).elements[0])
        self.assertTrue(findings[0].is_failure)

    def test_a_weak_hierarchy_is_reported(self):
        canvas = (Canvas()
                  .rect(40, 20, 120, 29, BLACK)
                  .rect(40, 160, 120, 20, BLACK))
        self.assertIn('glance hierarchy', self.measurements_reported(canvas))

    def test_only_clipping_and_contrast_can_make_a_variant_unusable(self):
        # The spec flags weak hierarchy and calls out no clear primary; neither
        # is a failure. A design with no dominant element may be exactly what
        # was asked for. Clipping and contrast are the two that disqualify.
        canvas = (Canvas()
                  .rect(40, 20, 120, 21, BLACK)     # 1.05x: no clear primary
                  .rect(40, 160, 120, 20, BLACK))
        measured = self.measure(canvas)

        self.assertTrue(measured.no_clear_primary)
        self.assertEqual(
            [f.measurement for f in measured.findings if f.is_failure], [])

    def test_every_finding_says_what_is_wrong(self):
        canvas = Canvas().rect(0, 0, WIDTH, HEIGHT // 2, GREY_6_90)
        for finding in self.measure(canvas).findings:
            self.assertTrue(finding.detail.strip())


class RealRenderSmokeTest(unittest.TestCase):
    """Loose assertions against renders the emulator actually produced.

    Synthetic fixtures are solid rectangles; real ones are glyphs. These exist to
    catch the case where the two diverge — not to pin down numbers, which is what
    the synthetic tests are for. Both states are covered, because the Stress
    render is the one the failing measurements are read from and its longer
    strings are where synthetic rectangles are least like the real thing.
    """

    def measure(self, name):
        return metrics.measure(
            os.path.join(os.path.dirname(__file__), 'renders', name))

    def test_every_measurement_comes_back_for_both_states(self):
        for name in ('plain_canonical.png', 'plain_stress.png'):
            with self.subTest(render=name):
                measured = self.measure(name)
                self.assertGreater(measured.ink_coverage, 0)
                self.assertIsNotNone(measured.safe_margin)
                self.assertIsNotNone(measured.glance_hierarchy)
                self.assertIsNotNone(measured.clipping)
                for element in measured.elements:
                    self.assertGreater(element.contrast, 0)

    def test_glyph_runs_are_found_as_elements_not_as_glyphs(self):
        # "10:09" and "Tue Jun 16" — two elements, not a dozen.
        self.assertEqual(len(self.measure('plain_canonical.png').elements), 2)

    def test_the_longest_date_still_reads_as_one_element(self):
        # "Wed Sep 30" at the Stress state: three words, still one run.
        self.assertEqual(len(self.measure('plain_stress.png').elements), 2)

    def test_known_good_renders_report_no_failures(self):
        for name in ('plain_canonical.png', 'plain_stress.png'):
            with self.subTest(render=name):
                self.assertEqual(
                    [f.detail for f in self.measure(name).findings
                     if f.is_failure], [])


if __name__ == '__main__':
    unittest.main()
