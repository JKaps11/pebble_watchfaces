#pragma once

#include <pebble.h>

// Reusable custom Layer that renders a large time above a small date, both
// centred, in two colours on whatever the watchface has put behind it.
//
// The third arrangement of the same two strings in shared/components, and
// deliberately its own component for the reason date_over_time_layer gives:
// which of the two is on top, how they are aligned and whether they share a
// colour are not settings of one design, they are different designs. A caller
// should be able to tell from the name what will be drawn.
//
// The date takes a second colour because the watchfaces using this put it over
// a picture, where a date in the same white as the time competes with it — the
// hierarchy has to come from somewhere and at this size it cannot all come from
// the type size.
//
// The date is set in a system font because Pebble's bundled faces are the only
// ones carrying a full alphabet cheaply; the time takes whatever font the caller
// loaded.
//
// Internal state lives in the layer's own data block via layer_get_data().

Layer *time_over_date_layer_create(GRect frame, GFont time_font,
                                   GColor time_colour, GColor date_colour);
void time_over_date_layer_destroy(Layer *layer);

// Reformats and redraws the layer for the given time.
void time_over_date_layer_update(Layer *layer, struct tm *tick_time);
