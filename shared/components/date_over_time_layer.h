#pragma once

#include <pebble.h>

// Reusable custom Layer that renders a small date above a large time, both
// flush left, in one colour on whatever the watchface has put behind it.
//
// Deliberately not a variation of time_date_layer: that one centres a Gothic
// time above a Gothic date in black, which is a different arrangement rather
// than a different setting of the same one. Parameterising a component until it
// can be either would leave a caller unable to tell from the name what it draws.
//
// The date is set in a system font because Pebble's bundled faces are the only
// ones that carry a full alphabet cheaply; the time takes whatever font the
// caller loaded, which is the whole reason this exists as a separate component.
//
// Internal state lives in the layer's own data block via layer_get_data().

Layer *date_over_time_layer_create(GRect frame, GFont time_font, GColor colour);
void date_over_time_layer_destroy(Layer *layer);

// Reformats and redraws the layer for the given time.
void date_over_time_layer_update(Layer *layer, struct tm *tick_time);
