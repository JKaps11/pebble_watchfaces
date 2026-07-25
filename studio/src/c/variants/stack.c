// stack — everything on the centre line, the foreground given more room.
//
// Axes: time-display=digital, composition=centred, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// band's problem might not be the straight edge at all — it might be that the
// type is flush left under a picture whose subject sits in the middle, so the
// eye crosses the display diagonally on every glance. This tests that: same
// ruled split, but the battery, the date and the time run down the centre line
// the pin already stands on. It costs the scene fourteen more rows, which is the
// price of finding out whether alignment was the complaint.

#include <pebble.h>
#include "../golf_course.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define ACCENT GColorInchworm
#define GROUND 132

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  golf_course_draw(ctx, GROUND);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, GROUND, 200, 228 - GROUND), 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, ACCENT);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(20, GROUND + 3), GPoint(180, GROUND + 3));
  graphics_draw_line(ctx, GPoint(46, GROUND + 6), GPoint(154, GROUND + 6));
}

static void prv_draw_battery(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent, ACCENT);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_battery_layer = layer_create(GRect(82, 142, 36, 14));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);

  s_date_layer = studio_text_layer(
      window_layer, GRect(0, 152, 200, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentCenter);

  s_time_layer = studio_text_layer(window_layer, GRect(0, 166, 200, 56),
                                   studio_font(RESOURCE_ID_FONT_BARLOW_52),
                                   GColorWhite, GTextAlignmentCenter);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_battery_layer);
  layer_destroy(s_scene_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), s_date_text,
                sizeof(s_date_text));
  s_battery_percent = battery_state_service_peek().charge_percent;

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  layer_mark_dirty(s_battery_layer);
}

STUDIO_VARIANT("stack", prv_load, prv_unload, prv_tick)
