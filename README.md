# Pebble Watchfaces

A monorepo of Pebble watchfaces, plus the Studio — a design sandbox for exploring what a
watchface should look like before one is built for real.

Everything targets **emery** (Pebble Time 2), a 200×228 reflective display, and is written
in C against the Pebble SDK ([ADR-0001](docs/adr/0001-c-for-watchfaces.md)).

## The watchfaces

| [goalmouth](watchfaces/goalmouth) | [liftoff](watchfaces/liftoff) | [pinhigh](watchfaces/pinhigh) | [teebox](watchfaces/teebox) |
| :---: | :---: | :---: | :---: |
| ![goalmouth](watchfaces/goalmouth/tests/screenshots/main_window.png) | ![liftoff](watchfaces/liftoff/tests/screenshots/main_window.png) | ![pinhigh](watchfaces/pinhigh/tests/screenshots/main_window.png) | ![teebox](watchfaces/teebox/tests/screenshots/main_window.png) |
| The goal down the pitch, a Fennec on the deck between you and it | The same arena, car off the deck and nose to the sky | The green from a few paces off, fringe rolling across the bottom | The same hole from the tee, deck underfoot |

Each carries time, date and battery, and nothing else. `goalmouth`/`liftoff` share a Rocket
League arena; `pinhigh`/`teebox` share a golf course. [tracer](watchfaces/tracer) is the
scaffolding face that proved the build/emulator/screenshot pipeline — kept as a reference,
not a design.

## How a watchface gets made here

1. **A brief** — one sentence about what the face should be.
2. **A Session in the Studio** — Batches of six Variants, rendered on the real emulator and
   tiled into a Contact sheet. A *Spread* varies two or three [design axes](.claude/skills/studio/axes.md)
   to find a direction; a *Sweep* varies exactly one to settle a detail.
3. **A pick** — one Variant off a Contact sheet, with the reason written down
   (`studio/sessions/<session>/pick.json`).
4. **A real watchface** — implemented fresh under `watchfaces/` on top of `shared/`. Nothing
   is carried across as code: a Variant is disposable evidence, not a draft
   ([ADR-0006](docs/adr/0006-studio-reads-shared-never-edits.md)).

Renders come from the QEMU emulator rather than mockups, so what gets approved is what the
watch shows ([ADR-0005](docs/adr/0005-emulator-renders-for-design-exploration.md)). Every
Session's renders, Contact sheets and reports are committed under `studio/sessions/`.

The `/studio` skill drives the whole flow from the brief; see [studio/README.md](studio/README.md)
for the tooling underneath.

## Layout

```
watchfaces/<name>/     One independent Pebble project each — own wscript, package.json, UUID
shared/c/              Pure logic, no Pebble SDK dependency, tested on the host with Unity
shared/components/     Reusable custom-Layer rendering components (SDK-dependent)
shared/resources/      Fonts bundled into watchfaces, with their licences
studio/                The design sandbox — Variants, host tooling, committed Sessions
docs/adr/              Architecture decisions
CONTEXT.md             The domain glossary — the words this repo uses, and the ones it avoids
```

The three tiers — logic, rendering components, per-watchface windows — are
[ADR-0004](docs/adr/0004-three-layer-architecture.md). Pebble's Waf build has no workspace
concept, so each watchface's `wscript` adds `../../shared` by hand
([ADR-0003](docs/adr/0003-monorepo-shared-logic.md)).

## Building & running

Requires the [Pebble tool](https://developer.repebble.com) with the emery emulator.

```sh
cd watchfaces/goalmouth
pebble build
pebble install --emulator emery
pebble install --phone <ip>            # or to a paired phone
```

## Tests

There is no on-device unit testing framework, so testing splits in two
([ADR-0002](docs/adr/0002-testing-strategy.md)): pure logic is pulled into `shared/c/` and
tested on the host, and rendering is verified by screenshot.

```sh
cd shared/tests && make test           # host-side Unity tests for shared logic
cd studio/tools && make test           # Studio host tooling (fast, no emulator)
STUDIO_EMULATOR_TESTS=1 make test      # ...and the emulator-backed render checks
```

## Reading further

- [CONTEXT.md](CONTEXT.md) — the glossary. Worth reading first; the terms (Variant, Batch,
  Session, Contact sheet, Spread, Sweep, Ink polarity) are used precisely everywhere else.
- [docs/adr/](docs/adr/) — why things are the way they are.
- [docs/proposals/0001-watchface-design-system.md](docs/proposals/0001-watchface-design-system.md)
  — superseded as a proposal, kept for the measurements behind ADR-0005 and the research
  behind the design axes.

## Licence

MIT — see [LICENSE](LICENSE). Bundled fonts are SIL Open Font License 1.1 and ship inside
the `.pbw`; their licence files travel with them (`shared/resources/fonts/`,
`studio/resources/fonts/`).
