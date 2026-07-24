# Pebble Watchfaces

A monorepo of Pebble watchfaces, plus a design sandbox for exploring what a watchface
should look like before one is built for real.

## Language

### Building watchfaces

**Watchface**:
A Pebble app that renders the current time as the watch's idle screen. Each one lives in its
own directory under `watchfaces/`.
_Avoid_: face, watchapp, clockface

**Complication**:
Any element on a watchface that shows something other than the time — battery, date, step
count, weather. Borrowed from horology, where it means any dial function beyond timekeeping.
_Avoid_: widget, indicator, module

### Designing watchfaces

**Studio**:
The design sandbox at `studio/`. Exists to explore how a watchface should look; nothing built
here ships. Reads shared code but never modifies it.
_Avoid_: sandbox, playground, prototypes, scratch

**Variant**:
One candidate watchface design rendered in the Studio. Disposable by nature — a variant is
evidence about a direction, not a draft of a shipping watchface.
_Avoid_: prototype, sketch, mockup, candidate

**Batch**:
A set of variants generated together and compared side by side. The unit of exploration.
_Avoid_: run, set, round

**Session**:
One sitting against a single brief. Contains one or more batches.
_Avoid_: iteration, cycle

**Contact sheet**:
The single image tiling a batch's variants together, so they can be compared in one look.
Borrowed from photography, where it means a proof sheet of every frame on a roll.
_Avoid_: grid, montage, gallery, sheet

### The design space

**Design axis**:
One named dimension a watchface design can vary along, with a fixed set of positions. The
axes are the shared vocabulary for saying how two variants differ.
_Avoid_: dimension, knob, parameter, variable

**Design space**:
The full set of design axes taken together. A variant occupies one position in it.

**Spread**:
Generating a batch whose variants sit far apart in the design space, to find a direction.
Answers "what could this be?"
_Avoid_: explore, diverge, wide

**Sweep**:
Generating a batch whose variants are identical except along one axis, to settle a detail.
Answers "how much of this?"
_Avoid_: refine, converge, narrow, tune

**Ink polarity**:
Whether a watchface is dark marks on a light ground or the reverse. A design axis in its own
right on Pebble, because the display is reflective rather than emissive.
_Avoid_: theme, dark mode, inverted

### Judging a watchface

**Canonical state**:
The single fixed time and device state every variant in a batch is rendered at, so that the
contact sheet compares designs rather than circumstances.
_Avoid_: default state, baseline, reference time

**Stress state**:
A deliberately worst-case time and device state — longest date strings, widest digits, every
complication at its ugliest — rendered per variant to expose clipping and contrast failures.
It feeds the measurements only; it is never shown on the contact sheet.
_Avoid_: edge case, worst case, torture test

**Glance hierarchy**:
How clearly a watchface establishes one dominant element the eye lands on first. The property
that decides whether time can be read without stopping to look.
_Avoid_: visual hierarchy, focal point, emphasis
