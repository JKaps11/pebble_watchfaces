# Explore watchface designs with emulator renders, not mockups

The obvious way to explore watchface designs quickly is to mock them up in HTML or SVG at
200x228 — instant, and expressive without fighting C. We measured the alternative instead:
a warm build/install/screenshot cycle against the QEMU emulator costs about 3.5 seconds
(`pebble build` 1.0s, `pebble install --emulator emery` 1.15s, `pebble screenshot` 1.17s),
and produces a raw framebuffer PNG that is exactly what the watch displays. We chose emulator
renders for the Studio because a mockup cannot use Pebble's system fonts, its 64-colour
palette, or its text wrapping, so approving a mockup means approving something the watch will
not show — and at 3.5 seconds, fidelity is nearly free. The accepted cost is that every
variant must be written as real C drawing code, which makes experimental ideas slower to try
than a browser sketch would be. Revisit if variant authoring, rather than rendering, becomes
the bottleneck.

## Consequences

Prior art points the other way and was considered: a widely-read June 2026 write-up on
designing a personal Pebble watchface used Claude-generated HTML prototypes to explore dozens
of variations. That author also reported the prototypes were "rough… meant to give a
direction, not to be a perfect representation," and that the real design work restarted once
the first version was built in C. We read that as evidence for mockups as an ideation aid and
against them as the surface a design is approved on.
