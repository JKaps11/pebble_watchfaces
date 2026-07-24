# Studio

A design sandbox for exploring what a watchface should look like before one is built for
real. **Nothing here ships.** The Studio has a throwaway UUID, is never published, and is
safe to delete at any time — nothing under `watchfaces/` may depend on it.

The Studio reads `shared/c/` and `shared/components/` the same way a watchface does, and
never modifies them (ADR-0006). A Variant that needs a shared component to behave
differently copies it into `src/c/` and modifies the copy.

## Layout

```
package.json          Throwaway UUID, emery only
wscript               Compiles exactly one Variant, chosen by STUDIO_VARIANT
src/c/main.c          Entry point; dispatches to the selected Variant
src/c/variant.h       The Variant interface
src/c/variants/*.c    One Variant per file
tools/studio/         Host tooling: render, metrics, contact sheet, report
tools/tests/          Host tests for the metrics module
sessions/             Committed Session outputs: renders, contact sheets, reports
```

## Rendering

Renders come from the real Emery emulator, not mockups (ADR-0005), so what a designer
approves is what the watch shows. A warm cycle is about 3.5 seconds.

```sh
python3 -m studio render plain canonical    # one Variant at one state
python3 -m studio batch <session> a b c d e f
```

Run from `tools/`, or with `PYTHONPATH=tools`. See `tools/README.md`.

## Building a single Variant by hand

```sh
STUDIO_VARIANT=plain pebble build
pebble install --emulator emery
pebble screenshot --emulator emery --no-open out.png
```

`STUDIO_VARIANT` defaults to `plain`. Only the named Variant is compiled, so a
half-finished sibling cannot stop the selected one rendering.
