"""Render a Batch of Variants and tile them into a Contact sheet.

A Batch is six Variants. Six covers meaningful ground when they are spread far
apart and gives enough steps to settle a question when they are swept along one
axis, while keeping the sheet scannable in one look.

Every Variant is rendered twice, at the Canonical state and the Stress state.
Only the Canonical renders are tiled. The Stress renders stay on disk for the
measurements to read but never reach the sheet, because the sheet's job is to
compare designs rather than circumstances, and a sheet mixing states would
compare neither.
"""

import json
import os
import subprocess

from studio import axes, render, states

SESSIONS_DIR = os.path.join(render.PROJECT_ROOT, 'sessions')

BATCH_SIZE = 6
TILE_COLUMNS = 3
TILE_ROWS = 2

CONTACT_SHEET_NAME = 'contact-sheet.png'
MANIFEST_NAME = 'batch.json'

# Framebuffer PNGs run 0.8-2 KB each, so keeping every render costs effectively
# nothing and gives a designer somewhere to go back to for an idea they passed
# over. Sheets and renders are committed; only build output is not.
SHEET_BACKGROUND = '#e8e8e8'
SHEET_INK = '#111111'
SHEET_POINTSIZE = 10


class BatchError(Exception):
    pass


def session_dir(session):
    return os.path.join(SESSIONS_DIR, session)


def batch_dir(session, number):
    """A Session holds one directory per Batch, numbered in the order run.

    A Session is one sitting against a single brief and may contain several
    Batches — asking for another when the first feels thin is the normal way to
    use this, not an edge case. So a Batch cannot be the unit of storage, or the
    second one silently overwrites the first's sheet and report.
    """
    return os.path.join(session_dir(session), 'batch-{}'.format(number))


def batch_numbers(session):
    """Which Batches a Session already holds, in order."""
    try:
        entries = os.listdir(session_dir(session))
    except FileNotFoundError:
        return []
    return sorted(int(name.split('-')[1]) for name in entries
                  if name.startswith('batch-') and name.split('-')[1].isdigit())


def next_batch_number(session):
    numbers = batch_numbers(session)
    return numbers[-1] + 1 if numbers else 1


def render_path(session, number, variant, state_name):
    return os.path.join(batch_dir(session, number), 'renders',
                        '{}_{}.png'.format(variant, state_name))


def _render_variant(session, number, variant):
    """Render one Variant at both states, checking the states actually landed.

    The check is free. A watchface shows the time, so its Canonical render and
    its Stress render can never be the same picture — 10:09 on Tue Jun 16 against
    23:58 on Wed Sep 30. If they come back identical, the emulator ignored the
    injected state and rendered the wall clock twice. That is the silent failure
    worth catching here rather than in the measurements, where it would show up
    as six Variants that mysteriously never clip.
    """
    wanted = [states.CANONICAL, states.STRESS]
    into = lambda state: render_path(session, number, variant, state.name)
    produced = render.render_states(variant, wanted, into)

    canonical, stress = produced
    if render.renders_match(canonical, stress):
        render.assert_state_injection_works()
        produced = render.render_states(variant, wanted, into)
        if render.renders_match(*produced):
            raise render.StateNotHonoured(
                'Variant {!r} renders identically at the Canonical and Stress '
                'states, so the emulator is ignoring injected state.'.format(
                    variant))
    return produced


def contact_sheet(session, number, variants, output_path=None):
    """Tile the Canonical renders three across and two down, each one labelled.

    Delegated to ImageMagick rather than done by hand: tiling and captioning is
    exactly what `montage` is for, and the Studio's own image code exists to
    measure renders, not to lay them out.
    """
    output_path = output_path or os.path.join(batch_dir(session, number),
                                              CONTACT_SHEET_NAME)
    # -depth 8 because montage would otherwise write 16-bit channels, which is
    # four times the size for a sheet of 64-colour framebuffers and not something
    # the Studio's own PNG reader has any reason to handle.
    command = ['magick', 'montage', '-depth', '8',
               '-pointsize', str(SHEET_POINTSIZE),
               '-background', SHEET_BACKGROUND, '-fill', SHEET_INK]
    for variant in variants:
        positions = axes.positions_of(variant, render.VARIANTS_DIR)
        command += ['-label', axes.label(variant, positions),
                    render_path(session, number, variant,
                                states.CANONICAL.name)]
    command += ['-tile', '{}x{}'.format(TILE_COLUMNS, TILE_ROWS),
                '-geometry', '+10+10', output_path]

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        raise BatchError('building the contact sheet failed:\n{}'.format(
            result.stderr.strip()))
    return output_path


def _write_manifest(session, number, variants):
    """Record what was in the Batch, so the report and the sheet agree."""
    manifest = {
        'session': session,
        'batch': number,
        'variants': [
            {
                'name': variant,
                'axes': axes.positions_of(variant, render.VARIANTS_DIR),
                'renders': {state: os.path.relpath(
                    render_path(session, number, variant, state),
                    batch_dir(session, number))
                    for state in states.BY_NAME},
            }
            for variant in variants
        ],
    }
    path = os.path.join(batch_dir(session, number), MANIFEST_NAME)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w') as handle:
        json.dump(manifest, handle, indent=2)
        handle.write('\n')
    return manifest


def read_manifest(session, number=None):
    """What was in a Batch, as recorded when it was rendered.

    Defaults to the Session's most recent Batch — the one a designer has just
    been looking at.
    """
    if number is None:
        numbers = batch_numbers(session)
        if not numbers:
            raise BatchError('No Batch in Session {!r}'.format(session))
        number = numbers[-1]
    path = os.path.join(batch_dir(session, number), MANIFEST_NAME)
    try:
        with open(path) as handle:
            return json.load(handle)
    except FileNotFoundError:
        raise BatchError('No Batch recorded at {}'.format(path)) from None


def render_batch(session, variants, on_progress=None, number=None):
    """Render a whole Batch into a Session and tile its Contact sheet.

    Numbered within the Session, so asking for another Batch against the same
    brief adds to the record rather than replacing it.
    """
    variants = list(variants)
    if len(variants) != BATCH_SIZE:
        raise BatchError(
            'A Batch is {} Variants, tiled {} across and {} down; got {}.'.format(
                BATCH_SIZE, TILE_COLUMNS, TILE_ROWS, len(variants)))

    unknown = [v for v in variants if v not in render.available_variants()]
    if unknown:
        raise BatchError('No Variant named: {}. Available: {}'.format(
            ', '.join(unknown), ', '.join(render.available_variants())))

    # Read every label before rendering anything: a Variant that has not declared
    # where it sits should fail in a second, not after a minute of rendering.
    for variant in variants:
        axes.positions_of(variant, render.VARIANTS_DIR)

    if number is None:
        number = next_batch_number(session)

    render.assert_state_injection_works()

    for variant in variants:
        _render_variant(session, number, variant)
        if on_progress:
            on_progress(variant)

    manifest = _write_manifest(session, number, variants)
    sheet = contact_sheet(session, number, variants)
    return {'session': session, 'batch': number, 'manifest': manifest,
            'contact_sheet': sheet}
