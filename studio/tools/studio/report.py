"""Assemble one Session report from the measurements of a Batch.

The report reports failures rather than printing every value for every Variant.
Six Variants' worth of passing numbers is a wall, a wall gets skipped, and a
report that gets skipped is worse than none.

Which render a measurement comes from is not arbitrary. Clipping and contrast are
taken from the Stress render, because that is where they fail — a date that fits
at "Tue Jun 16" and runs off the edge at "Wed Sep 30" is exactly the case worth
catching. Hierarchy, ink coverage and safe margin are taken from the Canonical
render, because that is the design being compared on the Contact sheet.
"""

import os

from studio import axes, batch, metrics, render, states

REPORT_NAME = 'report.md'

# Where each measurement is taken from. metrics owns the split; this is the
# mapping from that split to the render it implies.
MEASUREMENT_SOURCE = dict(
    [(name, states.STRESS.name) for name in metrics.STRESS_MEASUREMENTS]
    + [(name, states.CANONICAL.name) for name in metrics.CANONICAL_MEASUREMENTS])


def findings_for(session, variant):
    """Every finding against one Variant, each tagged with the state it came from.

    Returns a list of (state_name, Finding), most severe first.
    """
    found = []
    for state_name in (states.STRESS.name, states.CANONICAL.name):
        measured = metrics.measure(
            batch.render_path(session, variant, state_name))
        found += [(state_name, finding) for finding in measured.findings
                  if MEASUREMENT_SOURCE.get(finding.measurement) == state_name]

    found.sort(key=lambda pair: not pair[1].is_failure)  # failures first
    return found


def _variant_section(session, variant, critique=None):
    positions = axes.positions_of(variant, render.VARIANTS_DIR)
    lines = ['### {}'.format(variant),
             '',
             '`{}`'.format(' · '.join(positions[axis]
                                      for axis in axes.AXIS_ORDER)),
             '']

    found = findings_for(session, variant)
    if not found:
        lines += ['Nothing measured against it.', '']
    else:
        for state_name, finding in found:
            lines.append('- **{}** — {} _(at the {} state)_'.format(
                finding.measurement, finding.detail, state_name))
        lines.append('')

    # The measured half is all this module produces. The written judgment — does
    # it match the brief, does it read as intentional — is the skill's to author,
    # and lands in this slot.
    if critique:
        lines += [critique.strip(), '']
    return lines


def build(session, critiques=None, variants=None):
    """Write the Session's report next to its Contact sheet. Returns the path."""
    critiques = critiques or {}
    if variants is None:
        variants = [entry['name']
                    for entry in batch.read_manifest(session)['variants']]

    failing = []
    lines = ['# Session {}'.format(session),
             '',
             '![Contact sheet]({})'.format(batch.CONTACT_SHEET_NAME),
             '',
             '## Findings',
             '',
             'Only failures and flags appear here. A Variant with nothing '
             'against it measured clean.',
             '']

    sections = []
    for variant in variants:
        found = findings_for(session, variant)
        if any(finding.is_failure for _, finding in found):
            failing.append(variant)
        sections += _variant_section(session, variant, critiques.get(variant))

    if failing:
        lines += ['**Not viable as they stand:** {}.'.format(
            ', '.join('`{}`'.format(name) for name in failing)), '']
    lines += sections

    path = os.path.join(batch.session_dir(session), REPORT_NAME)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w') as handle:
        handle.write('\n'.join(lines).rstrip() + '\n')
    return path
