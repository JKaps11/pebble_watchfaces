// caddy — the yardage sign, small, standing in the hole rather than under it.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// plaque's plate was liked and its flat green background was not, so this puts
// the plate into the scene the scene Variants earned: the course runs behind it
// and out to every edge, and the sign sits in one corner of the hole the way a
// real one does. The plate keeps giving the numerals a hard black ground, which
// is the whole reason the signage family measures well. What it costs is the
// picture — the sign is opaque, and it is standing on the fairway.

#include <pebble.h>
#include "../golf_course.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define ACCENT GColorInchworm
#define PLATE_X 8
#define PLATE_Y 132
#define PLATE_W 152
#define PLATE_H 88

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  golf_course_draw(ctx, 228);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(PLATE_X, PLATE_Y, PLATE_W, PLATE_H), 6,
                     GCornersAll);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_round_rect(ctx, GRect(PLATE_X, PLATE_Y, PLATE_W, PLATE_H), 6);

  graphics_context_set_stroke_color(ctx, ACCENT);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(PLATE_X + 10, 186),
                     GPoint(PLATE_X + PLATE_W - 10, 186));
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

  s_time_layer = studio_text_layer(window_layer, GRect(PLATE_X + 6, 130, 140, 56),
                                   studio_font(RESOURCE_ID_FONT_BARLOW_52),
                                   GColorWhite, GTextAlignmentCenter);

  s_date_layer = studio_text_layer(
      window_layer, GRect(PLATE_X + 10, 190, 84, 20),
      fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GColorWhite,
      GTextAlignmentLeft);

  s_battery_layer = layer_create(GRect(PLATE_X + 106, 194, 36, 14));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
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

STUDIO_VARIANT("caddy", prv_load, prv_unload, prv_tick)
