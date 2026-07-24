// plain — the reference Variant.
//
// Axes: digital · centred · 0 complications · mono · Gothic · dark-on-light
//
// Deliberately unremarkable. It exists to prove the render path end to end and
// to give every later Variant something to be compared against, so it is built
// entirely out of the shared component a real watchface would use rather than
// out of Studio drawing code.

#include <pebble.h>
#include "../variant.h"
#include "time_date_layer.h"

static Layer *s_time_date_layer;

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  s_time_date_layer = time_date_layer_create(layer_get_bounds(window_layer));
  layer_add_child(window_layer, s_time_date_layer);
}

static void prv_unload(Window *window) {
  time_date_layer_destroy(s_time_date_layer);
}

static void prv_tick(struct tm *now) {
  time_date_layer_update(s_time_date_layer, now);
}

STUDIO_VARIANT("plain", prv_load, prv_unload, prv_tick)
