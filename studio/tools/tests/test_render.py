"""Tests for the render path. These drive the real emulator, so they are slow.

Run with STUDIO_EMULATOR_TESTS=1; skipped otherwise, so `make test` stays fast
and works on a machine with no Pebble tooling.

    STUDIO_EMULATOR_TESTS=1 make test

What is being asserted is that the emulator honoured the state it was given.
That is not visual regression testing of designs — the Studio deliberately does
not do that, because Variants are meant to change. The reference Variant is
harness equipment whose correct render is fixed and known, and it is here
because the failure it catches is silent: the emulator keeps reporting success
while rendering the real wall-clock time instead of the state asked for.
"""

import os
import tempfile
import unittest

from studio import render, states

REQUIRES_EMULATOR = unittest.skipUnless(
    os.environ.get('STUDIO_EMULATOR_TESTS'),
    'set STUDIO_EMULATOR_TESTS=1 to run tests that drive the emulator')


class VariantSelectionTest(unittest.TestCase):
    def test_the_reference_variant_is_available(self):
        self.assertIn(render.REFERENCE_VARIANT, render.available_variants())

    def test_an_unknown_variant_is_rejected_before_anything_is_built(self):
        with self.assertRaises(render.RenderError) as raised:
            render.build('no-such-variant')
        self.assertIn('no-such-variant', str(raised.exception))

    def test_every_state_has_a_committed_reference_render(self):
        for state_name in states.BY_NAME:
            self.assertTrue(os.path.exists(render.reference_render(state_name)),
                            'missing reference render for ' + state_name)


class ClockInjectionTest(unittest.TestCase):
    """The clock is the input that goes wrong silently, so it is pinned down."""

    def test_the_clock_is_injected_as_local_wall_clock_time(self):
        # emu-set-time takes UTC seconds and the watch adds its offset back on,
        # so what goes over the wire must be the local time converted through
        # the host's zone. Getting this wrong shifts the rendered date.
        command, = render._clock(states.STRESS)
        self.assertEqual(command[0], 'emu-set-time')
        self.assertEqual(int(command[1]), int(states.STRESS.when.timestamp()))

    def test_the_clock_is_set_after_everything_else(self):
        # Every other emu- command puts the clock back to real time as a side
        # effect, so the clock has to go last or the state is silently lost.
        applied = render._settings(states.STRESS) + render._clock(states.STRESS)
        self.assertEqual(applied[-1][0], 'emu-set-time')
        self.assertNotIn('emu-set-time', [command[0] for command in applied[:-1]])


@REQUIRES_EMULATOR
class RenderAgainstReferenceTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Start from an emulator known to be honouring state. It degrades as
        # renders accumulate, and these tests compare against fixed references,
        # so without this they fail for the emulator's reasons rather than the
        # code's — which is the definition of a flaky test.
        render.assert_state_injection_works()

    def test_each_state_renders_to_its_known_good_reference(self):
        with tempfile.TemporaryDirectory() as scratch:
            produced = render.render_states(
                render.REFERENCE_VARIANT, list(states.BY_NAME),
                lambda state: os.path.join(scratch, state.name + '.png'))

            for path, state_name in zip(produced, states.BY_NAME):
                with self.subTest(state=state_name):
                    self.assertTrue(
                        render.renders_match(
                            path, render.reference_render(state_name)),
                        'render at the {} state does not match its reference — '
                        'the emulator did not honour the injected state'.format(
                            state_name))

    def test_rendering_one_state_then_the_other_does_not_leak(self):
        # Rendering Stress and then Canonical must give a correct Canonical
        # render, not one still carrying the Stress battery level.
        with tempfile.TemporaryDirectory() as scratch:
            path = os.path.join(scratch, 'canonical.png')
            render.render(render.REFERENCE_VARIANT, 'stress',
                          os.path.join(scratch, 'stress.png'))
            render.render(render.REFERENCE_VARIANT, 'canonical', path)
            self.assertTrue(
                render.renders_match(path,
                                     render.reference_render('canonical')))


if __name__ == '__main__':
    unittest.main()
