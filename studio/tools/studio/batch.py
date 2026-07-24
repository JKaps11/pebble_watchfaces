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


def render_path(session, variant, state_name):
    return os.path.join(session_dir(session), 'renders',
                        '{}_{}.png'.format(variant, state_name))


def _render_variant(session, variant):
    """Render one Variant at both states, checking the states actually landed.

    The check is free. A watchface shows the time, so its Canonical render and
    its Stress render can never be the same picture — 10:09 on Tue Jun 16 against
    23:58 on Wed Sep 30. If they come back identical, the emulator ignored the
    injected state and rendered the wall clock twice. That is the silent failure
    worth catching here rather than in the measurements, where it would show up
    as six Variants that mysteriously never clip.
    """
    wanted = [states.CANONICAL, states.STRESS]
    produced = render.render_states(
        variant, wanted,
        lambda state: render_path(session, variant, state.name))

    canonical, stress = produced
    if render.renders_match(canonical, stress):
        render.assert_state_injection_works()
        produced = render.render_states(
            variant, wanted,
            lambda state: render_path(session, variant, state.name))
        if render.renders_match(*produced):
            raise render.StateNotHonoured(
                'Variant {!r} renders identically at the Canonical and Stress '
                'states, so the emulator is ignoring injected state.'.format(
                    variant))
    return produced


def contact_sheet(session, variants, output_path=None):
    """Tile the Canonical renders three across and two down, each one labelled.

    Delegated to ImageMagick rather than done by hand: tiling and captioning is
    exactly what `montage` is for, and the Studio's own image code exists to
    measure renders, not to lay them out.
    """
    output_path = output_path or os.path.join(session_dir(session),
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
                    render_path(session, variant, states.CANONICAL.name)]
    command += ['-tile', '{}x{}'.format(TILE_COLUMNS, TILE_ROWS),
                '-geometry', '+10+10', output_path]

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        raise BatchError('building the contact sheet failed:\n{}'.format(
            result.stderr.strip()))
    return output_path


def _write_manifest(session, variants):
    """Record what was in the Batch, so the report and the sheet agree."""
    manifest = {
        'session': session,
        'variants': [
            {
                'name': variant,
                'axes': axes.positions_of(variant, render.VARIANTS_DIR),
                'renders': {state: os.path.relpath(
                    render_path(session, variant, state), session_dir(session))
                    for state in states.BY_NAME},
            }
            for variant in variants
        ],
    }
    path = os.path.join(session_dir(session), MANIFEST_NAME)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w') as handle:
        json.dump(manifest, handle, indent=2)
        handle.write('\n')
    return manifest


def read_manifest(session):
    """What was in a Batch, as recorded when it was rendered."""
    path = os.path.join(session_dir(session), MANIFEST_NAME)
    try:
        with open(path) as handle:
            return json.load(handle)
    except FileNotFoundError:
        raise BatchError('No Batch recorded at {}'.format(path)) from None


def render_batch(session, variants, on_progress=None):
    """Render a whole Batch into a Session and tile its Contact sheet."""
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

    render.assert_state_injection_works()

    for variant in variants:
        _render_variant(session, variant)
        if on_progress:
            on_progress(variant)

    manifest = _write_manifest(session, variants)
    sheet = contact_sheet(session, variants)
    return {'session': session, 'manifest': manifest, 'contact_sheet': sheet}
