#pragma once

#include <pebble.h>

// Reusable custom Layer that renders the current time and date.
// Internal state (formatted text buffers) lives in the layer's own
// data block via layer_get_data() — callers only ever touch the Layer*.

Layer *time_date_layer_create(GRect frame);
void time_date_layer_destroy(Layer *layer);

// Reformats and redraws the layer for the given time.
void time_date_layer_update(Layer *layer, struct tm *tick_time);
