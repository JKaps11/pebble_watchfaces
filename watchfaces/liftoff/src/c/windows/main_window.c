#include <pebble.h>
#include "main_window.h"
#include "rocket_arena_layer.h"
#include "time_over_date_layer.h"
#include "battery_cell_layer.h"

// The same stadium and the same split, with the car off the deck and its nose to
// the sky.
//
// It shares everything with goalmouth except the vantage, which is the point: the
// two are the same watchface with the car doing different things, and they are
// separate rather than one configurable face because which one a person wants is
// a matter of taste and not of settings.
//
// The car is drawn in profile here while the arena still faces down the pitch,
// and that inconsistency is deliberate. Seen end-on a car looks identical whether
// its nose points at the sky or at the goal, so an aerial cannot be shown from
// behind at all — the choice is a car that is clearly climbing or a camera that
// is consistent, and this face takes the climb.
#define GLANCE_FRAME GRect(8, 4, 184, 78)
// Under the date and still clear of the far wall, which starts 46 rows above
// the horizon. The whole glance stack stays inside the black third.
#define BATTERY_FRAME GRect(82, 84, 36, 13)
// The date and the battery share a colour because they share a tier: both are
// complications, and the hierarchy has to read at a glance without relying on
// size alone.
#define COMPLICATION_COLOUR GColorYellow

static Window *s_window;
static Layer *s_arena_layer;
static Layer *s_glance_layer;
static Layer *s_battery_layer;
static GFont s_time_font;

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_over_date_layer_update(s_glance_layer, tick_time);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, GColorBlack);

  s_arena_layer = rocket_arena_layer_create(bounds, ROCKET_VANTAGE_AERIAL);
  layer_add_child(window_layer, s_arena_layer);

  s_time_font =
      fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ARCHIVO_44));
  s_glance_layer = time_over_date_layer_create(GLANCE_FRAME, s_time_font,
                                               GColorWhite, COMPLICATION_COLOUR);
  layer_add_child(window_layer, s_glance_layer);

  s_battery_layer = battery_cell_layer_create(BATTERY_FRAME,
                                              COMPLICATION_COLOUR);
  layer_add_child(window_layer, s_battery_layer);

  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  time_over_date_layer_update(s_glance_layer, tick_time);

  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}

static void prv_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  battery_cell_layer_destroy(s_battery_layer);
  time_over_date_layer_destroy(s_glance_layer);
  fonts_unload_custom_font(s_time_font);
  rocket_arena_layer_destroy(s_arena_layer);
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
