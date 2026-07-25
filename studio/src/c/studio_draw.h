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
