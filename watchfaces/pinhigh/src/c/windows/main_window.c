#include <pebble.h>
#include "main_window.h"
#include "battery_cell_layer.h"
#include "date_over_time_layer.h"
#include "golf_course_layer.h"

// The green, seen from a few paces off it, with the near fringe of the green
// rolling across the bottom of the display as a black ground for the time.
//
// The fringe is the point of the design and the reason the numerals are legible:
// a golf scene is mid-green almost everywhere, white type on it measures around
// 3:1, and cutting a black foreground out of the picture takes that to 21:1
// without a panel appearing anywhere. Rolling its edge rather than ruling it is
// what keeps it reading as ground rather than as a caption bar.
#define GROUND 156
#define FRINGE_CREST 140
#define FRINGE_SWELL 10
#define FRINGE_HIGHLIGHT 2

#define GLANCE_FRAME GRect(10, 152, 180, 68)
#define BATTERY_FRAME GRect(152, 156, 36, 14)

static Window *s_window;
static Layer *s_course_layer;
static Layer *s_fringe_layer;
static Layer *s_glance_layer;
static Layer *s_battery_layer;
static GFont s_time_font;

// A single low wave, its crown pushed off centre so it does not sit exactly
// where the cup does.
static int prv_fringe_at(int x) {
  int32_t along = (TRIG_MAX_ANGLE * 2 * x / 400) + (TRIG_MAX_ANGLE * 3 / 8);
  return FRINGE_CREST - (FRINGE_SWELL * sin_lookup(along) / TRIG_MAX_RATIO);
}

static void prv_draw_fringe(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  for (int x = 0; x < bounds.size.w; x++) {
    int crest = prv_fringe_at(x);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(x, crest, 1, bounds.size.h - crest), 0,
                       GCornerNone);
    graphics_context_set_fill_color(ctx, GColorInchworm);
    graphics_fill_rect(ctx, GRect(x, crest - FRINGE_HIGHLIGHT, 1,
                                  FRINGE_HIGHLIGHT),
                       0, GCornerNone);
  }
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  date_over_time_layer_update(s_glance_layer, tick_time);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, GColorBlack);

  s_course_layer = golf_course_layer_create(bounds, GOLF_VANTAGE_GREEN, GROUND);
  layer_add_child(window_layer, s_course_layer);

  s_fringe_layer = layer_create(bounds);
  layer_set_update_proc(s_fringe_layer, prv_draw_fringe);
  layer_add_child(window_layer, s_fringe_layer);

  s_time_font =
      fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BARLOW_52));
  s_glance_layer =
      date_over_time_layer_create(GLANCE_FRAME, s_time_font, GColorWhite);
  layer_add_child(window_layer, s_glance_layer);

  s_battery_layer = battery_cell_layer_create(BATTERY_FRAME, GColorWhite);
  layer_add_child(window_layer, s_battery_layer);

  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  date_over_time_layer_update(s_glance_layer, tick_time);

  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}

static void prv_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  battery_cell_layer_destroy(s_battery_layer);
  date_over_time_layer_destroy(s_glance_layer);
  fonts_unload_custom_font(s_time_font);
  layer_destroy(s_fringe_layer);
  golf_course_layer_destroy(s_course_layer);
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
