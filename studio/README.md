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

## Using it

Ask Claude — the `/studio` skill drives the whole flow from a one-sentence brief.
Its reference is in `.claude/skills/studio/`.

Underneath, from `tools/`:

```sh
python3 -m studio variants                        # what exists, and where it sits
python3 -m studio render plain canonical          # one Variant at one state
python3 -m studio batch <session> a b c d e f     # a Batch of six, sheet and report
python3 -m studio report <session>                # rebuild, folding in critique.json
python3 -m studio pick <session> <variant> --why "..."
```

Renders come from the real Emery emulator, not mockups (ADR-0005), so what a designer
approves is what the watch shows. A Batch of six takes about two minutes: injecting state
costs about a second per input and the pebble tool cannot be run concurrently with itself.

## Tests

```sh
cd tools && make test                        # fast, no emulator
STUDIO_EMULATOR_TESTS=1 make test            # also drives the emulator
```

The emulator-backed tests exist because its worst failure is silent — it keeps accepting
injected state while rendering the real wall-clock time, which looks like a perfectly good
render of the wrong thing. `plain` is kept as harness equipment with committed known-good
renders to compare against. If the emulator wedges, `restart_emulator()` wipes its persisted
flash image, which is the only thing that recovers it.

## Building a single Variant by hand

```sh
STUDIO_VARIANT=plain pebble build
pebble install --emulator emery
pebble screenshot --emulator emery --no-open out.png
```

`STUDIO_VARIANT` defaults to `plain`. Only the named Variant is compiled, so a
half-finished sibling cannot stop the selected one rendering.
