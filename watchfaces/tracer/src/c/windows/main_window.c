#include <pebble.h>
#include "main_window.h"
#include "time_date_layer.h"

static Window *s_window;
static Layer *s_time_date_layer;

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_date_layer_update_time(s_time_date_layer, tick_time);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_time_date_layer = time_date_layer_create(bounds);
  layer_add_child(window_layer, s_time_date_layer);

  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  time_date_layer_update_time(s_time_date_layer, tick_time);

  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}

static void prv_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  time_date_layer_destroy(s_time_date_layer);
}

void main_window_push(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);
}

void main_window_destroy(void) {
  window_destroy(s_window);
}
