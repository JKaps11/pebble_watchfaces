"""The design space: six named axes, and where a Variant sits on them.

The axes are the shared vocabulary for saying how two Variants differ, so a
designer can say "the mono one with fewer complications" and be understood. They
are a fixed set rather than open-ended variation, which is what makes a Contact
sheet comparable and a Sweep meaningful.

A Variant declares its position in its own source file, on a line like:

    // Axes: time-display=digital, composition=centred, complications=0,
    //       hue=mono, type=Gothic, polarity=dark-on-light

Keeping the declaration in the source rather than in a separate manifest is
deliberate: the label on a Contact sheet has to describe what was actually
built, and a manifest a directory away is the kind of thing that drifts.
Positions are checked against the vocabulary below, so a typo is an error rather
than a mislabelled tile.
"""

import os
import re

# Derived from a published analysis of 358 premium smartwatch faces (Islam et
# al., "Visualizing Information on Smartwatch Faces: A Review and Design Space"),
# trimmed to what transfers to a reflective, non-animated 200x228 display.
# Excluded from that source: UI style (skeuomorphism needs bitmap texture the
# platform cannot spare), animation (80% of surveyed faces have none, and it
# costs battery here), and charts (rare even on better hardware, and they do not
# earn the space at this size).
#
# Ink polarity is not from that source. It is here because Pebble's display is
# reflective rather than emissive, which makes polarity a legibility decision
# and not only an aesthetic one.
AXES = {
    'time-display': ('digital', 'analogue', 'hybrid'),
    'composition': ('centred', 'corner-anchored', 'split', 'asymmetric'),
    'complications': ('0', '1-2', '3-4', '5+'),
    'hue': ('mono', 'one-hue', 'multi'),
    'type': ('LECO', 'Bitham', 'Gothic', 'custom'),
    'polarity': ('dark-on-light', 'light-on-dark'),
}

# Order matters: it is the order positions are read out in a label, and a label
# whose fields move around is harder to scan across six tiles.
AXIS_ORDER = ('time-display', 'composition', 'complications', 'hue', 'type',
              'polarity')

_DECLARATION = re.compile(r'^\s*//\s*Axes:(.*)$', re.MULTILINE)
_CONTINUATION = re.compile(r'^\s*//\s{2,}(\S.*)$')


class AxesError(Exception):
    """A Variant's axis declaration is missing, malformed, or off-vocabulary."""


def positions_in(source_text, where='<source>'):
    """Read a Variant's axis positions out of its source. Returns a dict."""
    match = _DECLARATION.search(source_text)
    if not match:
        raise AxesError(
            '{}: no "// Axes:" declaration. Every Variant must say where it '
            'sits on the design axes: {}.'.format(where, ', '.join(AXIS_ORDER)))

    # A declaration may wrap onto following comment lines, indented under it.
    declaration = match.group(1)
    for line in source_text[match.end():].split('\n')[1:]:
        continued = _CONTINUATION.match(line)
        if not continued:
            break
        declaration += ' ' + continued.group(1)

    found = {}
    for field in (f.strip() for f in declaration.split(',')):
        if not field:
            continue
        axis, separator, position = field.partition('=')
        axis, position = axis.strip(), position.strip()
        if not separator:
            raise AxesError('{}: axis field {!r} is not "axis=position".'.format(
                where, field))
        if axis not in AXES:
            raise AxesError('{}: unknown axis {!r}. Known axes: {}.'.format(
                where, axis, ', '.join(AXIS_ORDER)))
        if position not in AXES[axis]:
            raise AxesError(
                '{}: {!r} is not a position on the {!r} axis. Positions: '
                '{}.'.format(where, position, axis, ', '.join(AXES[axis])))
        found[axis] = position

    missing = [axis for axis in AXIS_ORDER if axis not in found]
    if missing:
        raise AxesError('{}: no position given for {}.'.format(
            where, ', '.join(missing)))
    return found


def positions_of(variant, variants_dir):
    """Read the axis positions declared by a Variant source file."""
    path = os.path.join(variants_dir, '{}.c'.format(variant))
    try:
        with open(path) as handle:
            return positions_in(handle.read(), where=path)
    except FileNotFoundError:
        raise AxesError('No Variant source at {}'.format(path)) from None


def label(variant, positions):
    """The caption under a Contact sheet tile: what it is, and where it sits.

    Wrapped onto three short lines rather than one long one. Six positions set
    end to end are wider than the 200 px tile they sit under, and on a sheet
    three tiles across the captions run into each other and stop belonging to
    any particular picture.
    """
    read_out = [positions[axis] for axis in AXIS_ORDER]
    return '{}\n{}\n{}'.format(variant,
                               ' · '.join(read_out[:3]),
                               ' · '.join(read_out[3:]))


def differing_axes(first, second):
    """Which axes two Variants actually differ on."""
    return tuple(axis for axis in AXIS_ORDER if first[axis] != second[axis])
