# Research record: watchface design tooling

**Status:** superseded as a proposal · retained as a research record · 2026-07-24

This began as a proposal for design tooling across the whole repo. Its recommendations have
since been superseded — the design-exploration parts became the Studio ([spec #5]), and the
decisions that survived were recorded properly as ADR-0005 and ADR-0006.

What is kept here is the part that does not live anywhere else: **the measurements and the
research** the decisions were made from. A future reader wanting to know *why* emulator
renders beat mockups, or where the design axes came from, should find the evidence rather
than take the ADRs on faith.

Everything below marked **[measured]** was verified on the development machine on 2026-07-24.

---

## 1. Measured: the render loop is fast

`watchfaces/tracer`, Emery emulator:

| Step | Cold | Warm |
| --- | --- | --- |
| `pebble build` (no source change) | — | **0.99s** |
| `pebble install --emulator emery` | 6.94s (boots QEMU) | **1.15s** |
| `pebble screenshot` | — | **1.17s** |
| `pebble emu-set-time` | — | 0.92s |
| **Total edit→pixels** | ~9s | **~3.5s** |

This single measurement decided the shape of everything that followed. The usual argument for
HTML/SVG mockups is iteration speed; there is very little speed to win, so mockups lose their
justification and their fidelity cost becomes pure loss. Recorded as **ADR-0005**.

It also rules out a host-side renderer. PebbleOS *does* have host-side graphics tests with
committed PNG fixtures (`tests/fw/graphics/`, ~40 test files), so compiling `applib/graphics`
for the host is feasible — but it would buy perhaps 3s per iteration against a large build,
a fidelity risk, and permanent upstream-tracking burden.

## 2. Measured: screenshots are raw framebuffers

```
emery  → 200×228 PNG      gabbro → 260×260 PNG      flint → 144×168 PNG
```

No bezel, no chrome, no scaling. Pixel (0,0) of the PNG is pixel (0,0) of the `GRect` drawn
into. This is what makes programmatic design review possible at all — a script can state
exactly where an element sits, rather than approximately.

Gabbro's framebuffer is a **square** although its display is round; corner pixels are drawn
and never seen. Relevant only if that platform ever comes into scope.

## 3. Measured: the emulator is fully injectable

`pebble emu-set-time $(date -d '2027-03-05 09:41:00' +%s)` renders "09:41 / Fri Mar 5" —
correct weekday, full date and time control.

**Trap:** the `HH:MM:SS` form sets time-of-day only. The Unix-seconds form sets both but is
interpreted as UTC, so the watch's timezone offset shifts it. Get this wrong and a date-
dependent test silently exercises the wrong date.

Also available: `emu-battery`, `emu-bt-connection`, `emu-steps`, `emu-distance`,
`emu-calories`, `emu-active-time`, `emu-sleep`, `emu-heart-rate`, `emu-set-content-size`,
`emu-time-format`, `emu-accel`, `emu-compass`, `emu-tap`, `emu-button`, `emu-app-config`.

Every input a watchface can render is settable from the command line. This is what makes the
Canonical/Stress state pair in [spec #5] possible.

## 4. Measured: all seven platform emulators are installed

`~/.local/share/pebble-sdk/SDKs/4.17/sdk-core/pebble/{aplite,basalt,chalk,diorite,emery,
flint,gabbro}/qemu` all present. Cross-platform rendering needs no downloads — so Emery-only
is a scope decision (issue #1), not a capability limit.

Host tooling available: ImageMagick (`montage`, `compare`, `identify`), `ffmpeg`, `uv`
(so `uv run --with pillow` covers image analysis). No system PIL or numpy.

## 5. Measured: latent multi-platform build bug

Setting `targetPlatforms` to more than one platform fails:

```
ld: multiple definition of `__pbl_app_info';
    flint/appinfo.auto.c.o … gabbro/appinfo.auto.c.o: first defined here
```

The build passes the same `app_sources` **list object** to `ctx.pbl_build()` on every loop
iteration; `pbl_build` appends each platform's generated sources into it, so platform *n+1*
inherits platform *n*'s. The stock wscript calls `ant_glob` fresh inside the loop instead.
Passing `list(app_sources)` per iteration builds all three modern platforms cleanly —
verified. Tracked as its own ticket; dormant while Emery-only.

## 6. Research: where the design axes came from

Islam, He, Bezerianos, Blascheck, Lee & Isenberg, *Visualizing Information on Smartwatch
Faces: A Review and Design Space* (arXiv 2310.16185) — systematic analysis of 358 premium
watch faces. Findings that shaped [spec #5]:

- Time display: **digital 60.3% · analogue 26.3% · hybrid 13.4%**
- UI style: flat 36.9% · skeuomorphic 33.8% · semi-flat 29.3%
- **Median 4 complications**; top-50-ranked faces average 4+, preference declines past that
- **43.3% use exactly one hue** (excluding black/white/grey) — restraint correlated with popularity
- 80% have no animation; animation is decorative where present
- Battery is the #1 complication, then health/fitness, weather/planetary, device/location
- Text-only representation dominates; charts are rare even on more capable hardware

Axes were trimmed to those that transfer to a reflective, non-animated 200×228 display.
Dropped: UI style (skeuomorphism needs bitmap texture), animation (battery cost), charts
(don't earn the space). **Ink polarity** was added — not from this source — because a
reflective display makes polarity a legibility decision, not only an aesthetic one.

Legibility metrics come from dial practice, where **contrast, visual hierarchy and clutter**
determine glanceability in that order. Contrast thresholds derive from WCAG's 4.5:1 and 3:1
floors, raised for the time element because a reflective display is read in sunlight.

## 7. Research: what the ecosystem does

- **Core Devices ships an official `pebble-watchface-agent-skill`** — natural language →
  watchface, 9-phase workflow, C templates, build/emulate/screenshot. A *generator*: strong
  at scaffolding, thin on design (its "Design" phase is one line about planning for 200×228).
- **Jonas Hietala's June 2026 write-up** on designing a personal Pebble watchface is the
  closest prior art. Workflow: Claude-generated HTML prototypes, dozens of variations, then
  build in C. His own caveat is the important part — prototypes were "rough… meant to give a
  direction, not to be a perfect representation," and his first *built* version was "so
  boring" that the real design work restarted there. Read as evidence for mockups as ideation
  and against them as an approval surface. Cited in ADR-0005.
- **TIC-TTMM** (Albert Salamon, Bronze A' Design Award 2025–26) — 12 faces, 4 analogue /
  4 abstract / 4 digital, each proposing "a different visual language for reading time." The
  best available reference for a coherent *family* of Pebble faces.
- **Official Pebble design guidance is thin**: font ≥18 for small text, 28 for large,
  Bitham-Bold 42 for time, "only as much information as is immediately required," colour as
  meaning. Four bullet points, not a design system.

## 8. What this proposal recommended that was not pursued

Recorded so the ideas are findable, and so they are not mistaken for live plans:

- **Design tokens (`shared/design/`), a `DESIGN.md` design language, and on-device specimen
  sheets** for shipped watchfaces — deferred; see the backlog issue.
- **A state matrix and committed golden screenshots** for shipped watchfaces — conflicts with
  issue #1's recorded scope; resolved there.
- **Multi-platform design review** — out of scope per issue #1; resolved there.
- **A curated reference corpus** — dropped in favour of letting committed Contact sheets and
  recorded picks accumulate into one.
- **A declarative layout DSL / JSON face format** — large build, constrains expression to
  whatever the DSL captures, and Pebble faces get their character from custom drawing a DSL
  would not express.
- **Switching to Alloy/JS** for iteration speed — ADR-0001 settled this, and §1 removes the
  main argument for reopening it.

---

[spec #5]: https://github.com/JKaps11/pebble_watchfaces/issues/5
