#include <pebble.h>
#include "variant.h"

static Window *s_window;
static const Variant *s_variant;

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (s_variant->tick) {
    s_variant->tick(tick_time);
  }
}

static void prv_window_load(Window *window) {
  s_variant->load(window);

  // Draw once immediately: a render is screenshotted well before the next tick.
  if (s_variant->tick) {
    time_t now = time(NULL);
    s_variant->tick(localtime(&now));
  }

  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}

static void prv_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  s_variant->unload(window);
}

int main(void) {
  s_variant = variant_selected();

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);

  app_event_loop();

  window_destroy(s_window);
}
