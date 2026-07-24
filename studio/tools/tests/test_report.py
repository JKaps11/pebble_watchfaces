"""Tests for the Session report.

The renders here are synthetic, so what each Variant "looks like" is chosen by
the test and the right answer is known. What matters is that a finding reaches
the report only from the render it belongs to: a design that fits at the
Canonical state and clips at the Stress state must be reported as clipping, and
one that crowds the edge only in the Stress render must not be reported for it.
"""

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


if __name__ == '__main__':
    unittest.main()
