#pragma once

#include <pebble.h>

#include "datetime_format.h"

// Drawing helpers shared between Variants.
//
// This is not a shared component in the ADR-0003 sense and must not become one:
// it exists because several Variants in the same directory were carrying
// byte-identical copies of the same three things, which is a maintenance cost
// with no upside even in disposable code. Anything here is Studio-only and stays
// Studio-only; if a shipped watchface ever wants it, that is its own piece of
// work against shared/components/ with its own review (ADR-0006).

// Formats the time and date for a broken-out time, in one call.
// Either buffer may be NULL if a Variant does not show that part.
void studio_format(struct tm *now, char *time_text, size_t time_size,
                   char *date_text, size_t date_size);

// Uppercases `text` in place, ASCII only.
//
// `studio_format` gives `Tue Jun 16`, and the FONT_MONO faces are subset to
// uppercase — so any Variant setting a date in one has to fold the case itself
// or the letters come back as blanks. That is a loop worth writing once.
void studio_upper(char *text);

// A point `length` from `centre` at `angle`, for clock hands and tick marks.
// Angle is Pebble's TRIG_MAX_ANGLE scale, zero at twelve o'clock.
GPoint studio_point_at(GPoint centre, int32_t angle, int length);

// The hour and minute hand angles for a time, on the same scale.
// The hour hand accounts for minutes, so it sits between hours rather than
// jumping — at 10:09 that is the difference between a plausible dial and a
// broken one.
int32_t studio_hour_angle(int hour, int minute);
int32_t studio_minute_angle(int minute);

// A transparent, single-colour TextLayer added to `parent`. Variants differ in
// font, colour and alignment; everything else was being repeated verbatim.
TextLayer *studio_text_layer(Layer *parent, GRect frame, GFont font,
                             GColor colour, GTextAlignment alignment);

// A filled ellipse, which Pebble's graphics API does not offer and a scene needs
// constantly: a putting green, a bunker, the sun sitting on a horizon. Drawn as
// scanlines, so it costs one fill per row and nothing else.
void studio_fill_ellipse(GContext *ctx, GPoint centre, int radius_x,
                         int radius_y, GColor colour);

// A filled triangle, for the pennant on a flagstick. Filled by fanning strokes
// from `apex` across the opposite edge — the same three lines of arithmetic were
// being written into every Variant that put a flag on the display.
void studio_fill_triangle(GContext *ctx, GPoint apex, GPoint from, GPoint to,
                          GColor colour);

// The conventional battery cell — outline, terminal nub, proportional fill —
// drawn to fill `frame`, nub included. Several Variants exploring the battery
// complication want exactly this shape and differ only in what sits beside it.
//
// The fill is the whole point of the icon, so it is never rounded away to
// nothing: any charge above zero draws at least one column of pixels, which is
// what makes 4% read as nearly empty rather than as empty.
void studio_battery_cell(GContext *ctx, GRect frame, int percent, GColor colour);
