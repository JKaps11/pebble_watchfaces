"""The /studio skill's reference must agree with the code that enforces it.

The skill reads axes.md to decide what to author; axes.py rejects anything off
the vocabulary. If those two drift, the skill writes Variants that will not
build, and the failure lands a minute later with no obvious cause. ADR-0004
already records the regret of standing up an interface ahead of its consumer —
this is the cheap guard against the same thing happening to a document.
"""

import os
import re
import unittest

from studio import axes

SKILL_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(
        os.path.abspath(__file__))))),
    '.claude', 'skills', 'studio')


def read(name):
    with open(os.path.join(SKILL_DIR, name)) as handle:
        return handle.read()


class SkillIsPresentTest(unittest.TestCase):
    def test_the_skill_and_the_documents_it_links_are_committed(self):
        for name in ('SKILL.md', 'axes.md', 'authoring.md'):
            self.assertTrue(os.path.exists(os.path.join(SKILL_DIR, name)),
                            'missing skill file: ' + name)

    def test_every_document_the_skill_links_to_exists(self):
        for link in re.findall(r'\]\((\w[\w.-]*\.md)\)', read('SKILL.md')):
            self.assertTrue(os.path.exists(os.path.join(SKILL_DIR, link)),
                            'SKILL.md links to a missing file: ' + link)


class AxesReferenceMatchesTheCodeTest(unittest.TestCase):
    def setUp(self):
        self.reference = read('axes.md')

    def test_every_axis_is_documented(self):
        for axis in axes.AXIS_ORDER:
            self.assertIn('`{}`'.format(axis), self.reference,
                          'axes.md does not document the {} axis'.format(axis))

    def test_every_position_is_documented(self):
        for axis, positions in axes.AXES.items():
            for position in positions:
                self.assertIn('`{}`'.format(position), self.reference,
                              'axes.md does not document {}={}'.format(
                                  axis, position))

    def test_the_reference_invents_no_axes_the_code_would_reject(self):
        # Anything in the reference's table that axes.py does not know would be
        # authored by the skill and then refused at render time.
        table_rows = re.findall(r'^\| `([\w-]+)` \| (.+) \|$', self.reference,
                                re.MULTILINE)
        self.assertTrue(table_rows, 'no axis table found in axes.md')

        for axis, positions_cell in table_rows:
            self.assertIn(axis, axes.AXES,
                          'axes.md documents an axis the code rejects: ' + axis)
            documented = set(re.findall(r'`([\w+-]+)`', positions_cell))
            self.assertEqual(documented, set(axes.AXES[axis]),
                             'positions for {} disagree with the code'.format(axis))


if __name__ == '__main__':
    unittest.main()
