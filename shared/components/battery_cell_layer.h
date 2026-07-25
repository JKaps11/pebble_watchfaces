#pragma once

#include <pebble.h>

// Reusable custom Layer that renders the battery complication as the
// conventional cell: outline, terminal nub, proportional fill.
//
// The layer owns its subscription to the battery service, so a caller creates
// it and never thinks about charge again. Internal state lives in the layer's
// own data block via layer_get_data().

// The cell is drawn to fill `frame`, nub included, in `colour`.
Layer *battery_cell_layer_create(GRect frame, GColor colour);
void battery_cell_layer_destroy(Layer *layer);
