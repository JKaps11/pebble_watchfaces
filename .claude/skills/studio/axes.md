# The design space

Six axes. A Spread Batch varies two or three and holds the rest fixed; a Sweep
varies exactly one. The positions are a fixed vocabulary — `studio/tools/studio/axes.py`
validates against it, and a Variant declaring anything else fails before it
renders.

The axes exist so a designer can say *the mono one with fewer complications* and
be understood. Use these words with the user.

| Axis | Positions |
| --- | --- |
| `time-display` | `digital` · `analogue` · `hybrid` |
| `composition` | `centred` · `corner-anchored` · `split` · `asymmetric` |
| `complications` | `0` · `1-2` · `3-4` · `5+` |
| `hue` | `mono` · `one-hue` · `multi` |
| `type` | `LECO` · `Bitham` · `Gothic` · `custom` |
| `polarity` | `dark-on-light` · `light-on-dark` |

## What each one is asking

**`time-display`** — the most basic question about a watchface. Digital was 60%
of the surveyed corpus, analogue 26%, hybrid 13%. Hybrid is the one a designer is
least likely to be able to picture without seeing it.

**`composition`** — where mass sits on the display. The real difference between a
centred design and a corner-anchored one is what the eye does before it reads
anything.

**`complications`** — how much the face carries besides the time. The median
surveyed face carried four, and preference fell off beyond roughly that.

**`hue`** — 43% of surveyed faces used exactly one hue excluding black, white and
grey, and restraint correlated with popularity. `one-hue` means one chromatic
colour; black, white and grey do not count against it.

**`type`** — the typeface carrying the *time*, not the date or labels. The system
set offers only LECO and Bitham large enough to carry a face (Gothic stops at
28 px), which is why `custom` exists; see the bundled faces in
`studio/resources/fonts/README.md`.

**`polarity`** — dark marks on a light ground or the reverse. On Pebble this is
not only a mood decision: the display is reflective rather than emissive, so
polarity changes sunlight legibility. Expect `light-on-dark` Variants to be worth
checking against their Stress contrast measurement.

## Deliberately not axes

**UI style** (skeuomorphic/flat) — skeuomorphism needs bitmap texture the
platform cannot spare. **Animation** — 80% of surveyed faces have none, and it
costs battery here. **Charts** — rare even on more capable hardware, and they do
not earn the space at 200×228.

Do not invent axes. If a brief seems to need one, say so and work within the six.

## Grounding

Axes are adapted from Islam, He, Bezerianos, Blascheck, Lee and Isenberg,
*Visualizing Information on Smartwatch Faces: A Review and Design Space*
(arXiv 2310.16185), a study of 358 premium smartwatch faces, trimmed to what
transfers to a reflective, non-animated display. Ink polarity is not from that
source; it is here because Pebble's display is reflective.
