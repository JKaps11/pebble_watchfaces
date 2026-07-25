# Curated fonts for the type-character axis

Pebble ships 30 system fonts, but only two families are large enough to carry a
watchface — LECO (geometric numerals) and Bitham (humanist); Gothic stops at
28 px. Without these, every digital Variant is one of two typefaces and the type
axis collapses, so Batches converge however the other axes vary.

The set leans toward pixel and segment designs, which render most sharply at
small sizes on this display.

| Font | Category | Licence | Source |
| --- | --- | --- | --- |
| Silkscreen Regular | pixel / bitmap | OFL 1.1 | [google/fonts](https://github.com/google/fonts/tree/main/ofl/silkscreen) |
| Barlow Condensed Bold | condensed grotesk | OFL 1.1 | [google/fonts](https://github.com/google/fonts/tree/main/ofl/barlowcondensed) |
| Share Tech Mono | monospace | OFL 1.1 | [google/fonts](https://github.com/google/fonts/tree/main/ofl/sharetechmono) |
| Archivo Black | heavy display | OFL 1.1 | [google/fonts](https://github.com/google/fonts/tree/main/ofl/archivoblack) |
| Wallpoet | seven-segment / LCD | OFL 1.1 | [google/fonts](https://github.com/google/fonts/tree/main/ofl/wallpoet) |

Every licence was checked when the font was added, and each one's `OFL-*.txt` is
kept beside it. All five are SIL Open Font License 1.1, which permits bundling
and modification and requires the licence to travel with the font — which is why
those files are here.

Licensing is otherwise a shipping concern rather than a Studio concern: nothing
built here is distributed, so a face that turned out to be unusable in a shipped
watchface would still be fine to explore with. That becomes a real problem only
if and when a design using one gets built for real.

## Adding one

Add it to `resources.media` in `../../package.json` as a `font` entry named
`FONT_<NAME>_<SIZE>`, keep the `characterRegex` tight, and record it above with
its licence. Variants then reach it through `studio_font()`.

The `characterRegex` is `[0-9: APM]` throughout. The whole charset at 44 px would
cost tens of kilobytes each against Emery's 256 KB resource budget, and the type
axis is about the face carrying the time — a date or a label should use a system
font.
