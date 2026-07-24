#include <pebble.h>
#include "main_window.h"

static Window *s_window;
static TextLayer *s_placeholder_layer;

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_placeholder_layer = text_layer_create(GRect(0, bounds.size.h / 2 - 10, bounds.size.w, 20));
  text_layer_set_text(s_placeholder_layer, "tracer");
  text_layer_set_text_alignment(s_placeholder_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_placeholder_layer));
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_placeholder_layer);
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
