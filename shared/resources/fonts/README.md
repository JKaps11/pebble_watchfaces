# Fonts that ship

Every face here is bundled into a `.pbw` and distributed, which is the whole
difference between this directory and `studio/resources/fonts/`. The Studio's own
README says licensing is a shipping concern rather than a Studio concern and
becomes a real problem only if and when a design using one gets built for real.
This is where that happens, so the obligations are recorded rather than assumed.

| Font | Category | Licence | Used by | Source |
| --- | --- | --- | --- | --- |
| Barlow Condensed Bold | condensed grotesk | OFL 1.1 | [pinhigh](../../../watchfaces/pinhigh), [teebox](../../../watchfaces/teebox) | [google/fonts](https://github.com/google/fonts/tree/main/ofl/barlowcondensed) |
| Archivo Black | heavy display | OFL 1.1 | [goalmouth](../../../watchfaces/goalmouth), [liftoff](../../../watchfaces/liftoff) | [google/fonts](https://github.com/google/fonts/tree/main/ofl/archivoblack) |

Both are SIL Open Font License 1.1, which permits bundling and modification and
**requires the licence text to travel with the font**. That is why each one's
`OFL-*.txt` sits beside it here, and why neither may be moved without its
licence file going too. Subsetting a font — which every watchface below does —
counts as modification and is permitted; what is not permitted is shipping the
result with the licence left behind.

The OFL also forbids selling the fonts on their own and forbids using the
Reserved Font Names in a derivative. Neither applies to bundling a subset into a
watchface, but both are reasons not to treat this directory as a general asset
dump.

## Adding one

Put the TTF here with its licence file, add it to `resources.media` in the
consuming watchface's `package.json` as a `font` entry named `FONT_<NAME>_<SIZE>`,
keep the `characterRegex` tight, and add a row above naming the licence and every
watchface that uses it.

`characterRegex` is `[0-9: APM]` for every face carrying the time, which is the
digits, the separator, and enough for a twelve-hour suffix. The whole charset at
display size costs tens of kilobytes against Emery's 256 KB resource budget, and
a date or a label should use a Pebble system font instead — those carry a full
alphabet at no cost to the bundle.
