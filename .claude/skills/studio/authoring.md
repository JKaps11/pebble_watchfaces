# Authoring a Variant

One Variant per file in `studio/src/c/variants/<name>.c`. Only the selected one
is compiled, so a half-finished sibling cannot stop the others rendering.

Read an existing Variant before writing new ones — `plain.c` is the simplest,
`dense.c` and `halves.c` show complications and custom drawing.

## The shape of one

```c
// <name> — one line on what it is for.
//
// Axes: time-display=digital, composition=centred, complications=1-2,
//       hue=mono, type=Bitham, polarity=light-on-dark
//
// A sentence or two on the question this Variant exists to answer.

#include <pebble.h>
#include "../variant.h"
#include "datetime_format.h"   // shared/c, read-only

static TextLayer *s_time_layer;

static void prv_load(Window *window) { /* build the layer tree */ }
static void prv_unload(Window *window) { /* tear it down */ }
static void prv_tick(struct tm *now) { /* reformat and redraw */ }

STUDIO_VARIANT("<name>", prv_load, prv_unload, prv_tick)
```

The name in `STUDIO_VARIANT` must match the filename. The entry point calls
`tick` once on load, so a render never waits for a minute boundary.

## The axis declaration is not a comment

It is read out of the source to label the tile on the Contact sheet, and it is
validated — an unknown axis or an off-vocabulary position is an error before
anything renders. Keep it truthful: if you change the font a Variant uses, change
the `type` position with it, or the sheet will tell the user something false.

Positions are listed in [axes.md](axes.md).

## Traps

**Complications that cannot be injected.** Step count and Bluetooth do not
respond to the emulator on this setup — a health or connectivity complication
renders the same in both states, so the Stress render will not stress it. Prefer
complications derived from the clock or the battery: date, week number, battery
percentage. See `studio/tools/studio/states.py`.

**Custom fonts are digits only.** The bundled faces are subset to `[0-9: APM]`
to fit the resource budget. Use them for the time; use a system font for the date
or any label. Load with `studio_font(RESOURCE_ID_FONT_WALLPOET_44)` from
`../studio_font.h` — never `fonts_load_custom_font` directly, or the font leaks.

A consequence worth knowing: the Stress state's long strings — `Wed Sep 30` — are
always set in a system font, so a custom-face Variant is stress-tested on its
date rather than on its numerals. The type axis is still being explored honestly;
the widest thing on screen just is not the custom face.

**Numerals-only system fonts.** `FONT_KEY_LECO_42_NUMBERS` and
`FONT_KEY_BITHAM_42_MEDIUM_NUMBERS` have digits and punctuation only. A date set
in one renders blank.

**Both states must differ.** A Variant whose Canonical and Stress renders come
out identical is treated as a failed render, because that is what an ignored
state injection looks like. Any Variant showing the time satisfies this.

**Emery is 200×228.** Nothing else is a target. Keep 8 px clear of every edge or
the safe-margin measurement flags it, and check the Stress state — `Wed Sep 30`
at `23:58` is the longest and widest everything gets.

## Never

Modify anything under `shared/` or `watchfaces/` (ADR-0006). If a Variant needs a
shared component to behave differently, copy it into `studio/src/c/` and modify
the copy. Duplication inside the Studio is harmless; Studio code is disposable.
