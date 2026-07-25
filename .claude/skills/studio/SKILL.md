---
name: studio
description: Explore watchface designs in the Studio. Use when the user wants to see what a watchface idea could look like, asks for design options or variations, wants to compare watchface layouts side by side, or wants to refine one design decision (type, size, complication count, polarity). Triggers on "design a watchface", "what could this look like", "show me options", "spread", "sweep".
---

# Studio

A design sandbox for exploring what a watchface should look like before one is
built for real. The user describes an idea in a sentence; you return a Contact
sheet and a report; they point at one and either ask for another Batch or stop.
That is the whole interaction.

**Nothing here ships.** Picking a winner records the pick and ends the Session.
Building a real watchface from that direction is separate, manual work outside
this flow — do not offer to do it as part of a Session.

Read `CONTEXT.md` at the repo root for the vocabulary (Variant, Batch, Session,
Contact sheet, Spread, Sweep, Canonical state, Stress state) and use those words
with the user. Read [axes.md](axes.md) before authoring anything — it is the
design space, and a Variant that sits off it will not build.

## Choose the mode

**Spread** places Variants far apart in the design space. For when the direction
is unknown; answers *what could this be?* Vary **two or three** axes, hold the
rest fixed.

**Sweep** holds every axis fixed but one and steps through its positions. For
when the direction is settled and one detail is unresolved; answers *how much of
this?* Vary **exactly one** axis.

A Batch is six but no axis has six positions — the widest have four. So a Sweep
covers every position on its axis and spends the remaining tiles on degrees
within those positions: a `type` sweep might run LECO, Bitham, Gothic and three
different custom faces; a `complications` sweep might run 0, 1, 2, 3, 4 and 6.
The axis position declared on each Variant is still one of the four, and the
tiles still differ only along that axis. Do not pad a Sweep by varying a second
axis — that makes it a small Spread and answers neither question.

If the user does not say, infer: a vague brief means Spread; a brief naming an
existing Variant and something to vary means Sweep. Say which you chose in one
short sentence, and get on with it.

## Run a Session

Work from `studio/tools/` — every command below assumes that directory.

1. **See what already exists.** `python3 -m studio variants` lists each Variant
   with its axis positions. In a Sweep you are starting from one of these, so
   read its source before writing siblings.

2. **Author six Variants** in `studio/src/c/variants/<name>.c`. One per file.
   Follow [authoring.md](authoring.md) — it has the interface, the axis
   declaration, and the traps.

3. **Render the Batch.** A Batch is six; the command refuses any other number.

   ```sh
   python3 -m studio batch <session-name> <six variant names>
   ```

   Name the Session for the date and the brief: `2026-07-24-mono-sweep`. This
   takes about two minutes and prints the Contact sheet and report paths.

4. **Write the critique.** Create `studio/sessions/<session>/critique.json`:

   ```json
   { "variant-name": "Two or three sentences." }
   ```

   Then `python3 -m studio report <session>` to fold it in.

5. **Show the user the Contact sheet** and the findings from the report. Lead
   with the picture.

6. **Another Batch, or a pick.** A Session is one sitting against one brief and
   holds as many Batches as it takes. Run `batch` again with the *same* Session
   name — it lands in `batch-2/` beside `batch-1/`, and the emulator stays warm.
   Only start a new Session when the brief itself changes.

   A pick ends the Session, and may name a Variant from any of its Batches:

   ```sh
   python3 -m studio pick <session> <variant> --why "..."
   ```

## Writing the critique

Two to three sentences per Variant. **No more** — the report must not become
longer than looking at the pictures.

Cover what measurement cannot: whether it matches the brief, whether it reads as
intentional or generic, how it compares to the rest of the Batch. Do not restate
the numbers; the measured findings are already in the report, and repeating them
is what makes a report get skipped.

Be willing to say a Variant is weak. Six Variants where every one is described as
promising is worth nothing to someone choosing between them.

## What the report already tells you

The measured half is generated. Contrast and clipping come from the Stress
render; hierarchy, ink coverage and safe margin from the Canonical render. Only
failures and flags appear.

Treat a failure as disqualifying, and say so plainly: a Variant that clips its
date at the Stress state is not viable however good it looks on the sheet. That
is the entire reason the Stress render exists.

## Rules

- **Never modify anything under `shared/` or `watchfaces/`** (ADR-0006). A
  Variant that needs a shared component to behave differently copies it into
  `studio/src/c/` and modifies the copy.
- **Contact sheets, reports and Variant sources are committed.** Build output is
  not. Renders are 1-2 KB, so keeping the record is free — never delete a Variant
  to tidy up; the point is being able to go back to something passed over.
- **The labels must match what was built.** They are read from each Variant's
  own source, so a Variant whose declaration disagrees with its code will mislead
  the user on the sheet.
- **Do not judge a render by describing it from the code.** Look at the Contact
  sheet.
