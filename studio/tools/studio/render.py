"""Build a Variant, put it on the emulator at a given state, and screenshot it.

Renders come from the real emulator rather than mockups (ADR-0005), so what a
designer approves is what the watch shows. The cycle costs about 3.5 seconds
warm against about 9 cold, which is why the emulator is never killed between
renders.
"""

import os
import subprocess
import tempfile
import time

from studio import png, states

PLATFORM = 'emery'

# The Studio project root, two directories up from this file.
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__))))

VARIANTS_DIR = os.path.join(PROJECT_ROOT, 'src', 'c', 'variants')

# `pebble install` returns before the installed Variant has finished launching,
# and asynchronously puts the emulated clock back to real time on the way. State
# injected into that window is silently lost — the Variant comes up afterwards
# showing the wall-clock time, which looks like a successful render of the wrong
# state. Waiting the launch out is the only cure. An install that had to boot the
# emulator first needs considerably longer.
LAUNCH_SETTLE_SECONDS = 3.0
COLD_SETTLE_SECONDS = 8.0

# Repeated installs and large jumps of the emulated clock eventually wedge the
# emulator: installs start reporting "App install failed" and screenshots time
# out. Nothing recovers it short of a restart, so a Variant that fails to render
# is retried once on a fresh emulator before giving up. Losing a whole Batch to
# one wedged emulator is worse than spending ten seconds.
RENDER_ATTEMPTS = 3
INSTALL_ATTEMPTS = 2
RETRY_PAUSE_SECONDS = 2.0


# A Variant whose render at each state is fixed and known, kept as committed
# reference images. It is the canary for the failure described on
# `assert_state_injection_works` below.
#
# This is not visual regression testing of designs, which the Studio deliberately
# does not do — Variants are meant to change. `plain` is harness equipment: it is
# never explored, never varied, and exists so that "did the emulator honour the
# state?" has an answer that is checked rather than eyeballed.
REFERENCE_VARIANT = 'plain'
REFERENCE_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                             'reference')


class RenderError(Exception):
    pass


class StateNotHonoured(RenderError):
    """The emulator rendered something other than the state it was given."""


def available_variants():
    return sorted(name[:-2] for name in os.listdir(VARIANTS_DIR)
                  if name.endswith('.c'))


def _pebble(*args, env=None, attempts=1):
    """Run one `pebble` command against the emulator.

    Never run two of these at once. The tool keeps its emulator bookkeeping in a
    single shared file in the temp directory, and a second invocation racing the
    first reads half-written JSON and dies.
    """
    for attempt in range(1, attempts + 1):
        result = subprocess.run(
            ('pebble',) + args + ('--emulator', PLATFORM),
            cwd=PROJECT_ROOT, env=env, capture_output=True, text=True)
        if result.returncode == 0:
            return result
        if attempt < attempts:
            time.sleep(RETRY_PAUSE_SECONDS)
    raise RenderError('pebble {} failed after {} attempt(s):\n{}'.format(
        ' '.join(args), attempts,
        result.stderr.strip() or result.stdout.strip()))


def build(variant):
    """Compile the Studio with `variant` selected."""
    if variant not in available_variants():
        raise RenderError('No Variant named {!r}. Available: {}'.format(
            variant, ', '.join(available_variants())))
    env = dict(os.environ, STUDIO_VARIANT=variant)
    result = subprocess.run(('pebble', 'build'), cwd=PROJECT_ROOT, env=env,
                            capture_output=True, text=True)
    if result.returncode != 0:
        raise RenderError('build of Variant {!r} failed:\n{}'.format(
            variant, result.stdout.strip()))


def _settings(state):
    """Everything except the clock. Each of these survives an install.

    `emu-set-content-size` is deliberately not here: it leaves the watch sitting
    in its settings menu, so every screenshot after it is of the menu rather
    than the Variant. Not injecting it leaves the default size, which is what
    the Canonical state asks for anyway.
    """
    battery = ['emu-battery', '--percent', str(state.battery_percent)]
    if state.charging:
        battery.append('--charging')
    return [
        ['emu-time-format', '--format', state.time_format],
        battery,
    ]


def _clock(state):
    """The clock, which must be injected last and re-injected on every render.

    Every other `emu-` command puts the emulated clock back to real time as a
    side effect, and so does `install`. Setting the time first and the battery
    second silently renders whatever the wall clock says — which looks like a
    working render, and quietly destroys the Stress state, whose whole purpose
    is the date it carries.

    `emu-set-time` takes Unix seconds as UTC and the watch adds its timezone
    offset back on, so the seconds handed over must be the local wall-clock time
    converted through the host's zone — which is what `.timestamp()` does.
    Passing a naive UTC value renders a shifted date instead.
    """
    return [['emu-set-time', str(int(state.when.timestamp()))]]


def emulator_pid():
    """The running Emery emulator's pid, or None if it is not up."""
    found = subprocess.run(
        ('pgrep', '-f', 'machine pebble-{}'.format(PLATFORM)),
        capture_output=True, text=True).stdout.split()
    return found[0] if found else None


def restart_emulator():
    """Kill the emulator, then boot a fresh one and wait for it."""
    subprocess.run(('pebble', 'kill'), cwd=PROJECT_ROOT, capture_output=True,
                   text=True)
    time.sleep(RETRY_PAUSE_SECONDS)
    ensure_emulator()


def ensure_emulator():
    """Boot the emulator if it is not already up, and wait until it answers.

    Booting must not be left to `pebble install`. An install that has to start
    the emulator races its boot: the install reports failure, and every command
    afterwards — screenshots included — times out against an emulator that looks
    healthy in the process table. Booting first makes install a warm operation
    every time.

    The probe is a screenshot thrown away, rather than the more obvious `ping`,
    because `ping` puts a notification on the watch and it stays there —
    surviving the install — so every render comes back a picture of the word
    "Ping" instead of the Variant. A screenshot boots the emulator, returns only
    once it answers, and leaves the screen alone.

    Returns whether a boot was needed.
    """
    if emulator_pid():
        return False
    with tempfile.TemporaryDirectory() as scratch:
        _pebble('screenshot', '--no-open', os.path.join(scratch, 'boot.png'),
                attempts=INSTALL_ATTEMPTS)
    time.sleep(COLD_SETTLE_SECONDS)
    return True


def apply_state(state):
    """Put the emulator into `state`. Call after install, never before.

    Every injectable input is set every time, not just the ones that differ — a
    4% battery left over from a Stress render would otherwise contaminate every
    render after it.

    An earlier version skipped the settings when the emulator was already known
    to hold the state, and re-set only the clock. It was wrong every single time
    it took that path: the injections are what give `install`'s asynchronous
    clock reset time to land before the clock is set for real, and with them
    skipped the reset arrived afterwards and the render showed the wall-clock
    time. Since the shortcut has to wait just as long to be correct, it saves
    nothing, and applying everything is both simpler and honest about the cost.
    """
    for command in _settings(state) + _clock(state):  # clock always last
        _pebble(*command)


def _screenshot(output_path):
    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    _pebble('screenshot', '--no-open', os.path.abspath(output_path))
    return output_path


def render_states(variant, wanted, output_path_for):
    """Render one Variant at several states, installing it only once.

    Install is the expensive, race-prone step — it resets the emulated clock
    asynchronously, which is what the settle below waits out. Doing it once per
    Variant rather than once per render both halves the cost of a two-state
    Variant and removes a chance to get the state wrong.

    `output_path_for` maps a State to where its PNG should go. Returns the paths
    written, in order.
    """
    wanted = [states.by_name(s) if isinstance(s, str) else s for s in wanted]
    build(variant)

    for attempt in range(1, RENDER_ATTEMPTS + 1):
        try:
            ensure_emulator()
            _pebble('install', attempts=INSTALL_ATTEMPTS)
            time.sleep(LAUNCH_SETTLE_SECONDS)
            written = []
            for state in wanted:
                apply_state(state)
                written.append(_screenshot(output_path_for(state)))
            return written
        except RenderError:
            # A wedged emulator fails every command from here on, so start over
            # on a fresh one rather than reporting a render that never happened.
            if attempt == RENDER_ATTEMPTS:
                raise
            restart_emulator()


def render(variant, state, output_path):
    """Render `variant` at `state` and write the framebuffer PNG to `output_path`."""
    return render_states(variant, [state], lambda _: output_path)[0]


def reference_render(state_name):
    """Where the known-good render of the reference Variant lives."""
    return os.path.join(REFERENCE_DIR, '{}_{}.png'.format(
        REFERENCE_VARIANT, state_name))


def renders_match(one, other):
    """Whether two framebuffer PNGs are the same picture."""
    first, second = png.read(one), png.read(other)
    return (first.width == second.width and first.height == second.height
            and first.rows == second.rows)


def state_injection_works():
    """Render the reference Variant and check the emulator honoured the states.

    The emulator degrades as a Session goes on. After enough installs it keeps
    accepting `emu-set-time` and reporting success while continuing to render the
    real wall-clock time — a failure with no error and a perfectly good-looking
    picture, which is the worst kind. Downstream that would quietly invalidate
    every Stress measurement, because the Stress state exists for the long date
    strings it carries and the wall clock supplies whatever today happens to be.

    So the harness checks rather than assumes: it renders a Variant whose correct
    output is already known and compares.
    """
    with tempfile.TemporaryDirectory() as scratch:
        produced = render_states(
            REFERENCE_VARIANT, list(states.BY_NAME),
            lambda state: os.path.join(scratch, '{}.png'.format(state.name)))
        return all(renders_match(path, reference_render(state_name))
                   for path, state_name in zip(produced, states.BY_NAME))


def assert_state_injection_works():
    """Make sure the emulator is honouring injected state, restarting it if not.

    Raises StateNotHonoured if a freshly booted emulator still will not, since at
    that point every render is untrustworthy and continuing would produce a Batch
    of confident, wrong pictures.
    """
    if state_injection_works():
        return
    restart_emulator()
    if not state_injection_works():
        raise StateNotHonoured(
            'The emulator is not honouring injected state even after a restart: '
            'a render of the {!r} Variant does not match its known-good '
            'reference in {}. Renders taken now would show the real wall-clock '
            'time rather than the Canonical or Stress state.'.format(
                REFERENCE_VARIANT, REFERENCE_DIR))
