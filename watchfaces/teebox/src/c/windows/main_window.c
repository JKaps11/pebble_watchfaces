#include <pebble.h>
#include "main_window.h"
#include "battery_cell_layer.h"
#include "date_over_time_layer.h"
#include "golf_course_layer.h"

// The hole from the tee, with the tee deck underfoot and the time on the black
// below it.
//
// The straight edge across the middle is the one thing this face does that a
// scene watchface usually cannot get away with, and it works here for a reason
// that belongs to the picture rather than to the layout: the black does not
// begin at an arbitrary row, it begins where the tee deck stops. A horizontal
// rule under a photograph reads as a caption bar; the front lip of a tee reads
// as somewhere to stand.
//
// Which means GROUND is not a free parameter — it is the front of the deck, and
// moving it moves the wearer.
#define GROUND 152

#define GLANCE_FRAME GRect(10, 152, 180, 68)
#define BATTERY_FRAME GRect(152, 156, 36, 14)

static Window *s_window;
static Layer *s_course_layer;
static Layer *s_foreground_layer;
static Layer *s_glance_layer;
static Layer *s_battery_layer;
static GFont s_time_font;

static void prv_draw_foreground(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, GROUND, bounds.size.w,
                                bounds.size.h - GROUND),
                     0, GCornerNone);
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  date_over_time_layer_update(s_glance_layer, tick_time);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, GColorBlack);

  s_course_layer = golf_course_layer_create(bounds, GOLF_VANTAGE_TEE, GROUND);
  layer_add_child(window_layer, s_course_layer);

  s_foreground_layer = layer_create(bounds);
  layer_set_update_proc(s_foreground_layer, prv_draw_foreground);
  layer_add_child(window_layer, s_foreground_layer);

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
  layer_destroy(s_foreground_layer);
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
