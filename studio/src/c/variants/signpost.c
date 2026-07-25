// signpost — the yardage plate shrunk to a sign and moved off the fairway.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// caddy had the right object and the wrong size: a plate wide enough to be a
// panel, planted in the middle of the hole. This cuts it down until it reads as
// something staked into the ground, and moves it left onto the rough where a
// real one stands. The dusk palette is the second half of the experiment — with
// the sky drained to greens the sign is the only bright thing left, which is
// either the most legible Variant in the Batch or proof that the sky was doing
// all the work.

#include <pebble.h>
#include "../golf_course.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define ACCENT GColorInchworm
#define PLATE_X 6
#define PLATE_Y 142
#define PLATE_W 128
#define PLATE_H 78

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  golf_dusk_draw(ctx, 228);

  // The post, drawn before the plate so the plate sits on top of it.
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(PLATE_X + 30, PLATE_Y + PLATE_H - 4, 5, 12), 0,
                     GCornerNone);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(PLATE_X, PLATE_Y, PLATE_W, PLATE_H), 5,
                     GCornersAll);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_round_rect(ctx, GRect(PLATE_X, PLATE_Y, PLATE_W, PLATE_H), 5);

  graphics_context_set_stroke_color(ctx, ACCENT);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(PLATE_X + 10, 196),
                     GPoint(PLATE_X + PLATE_W - 10, 196));
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

  s_time_layer = studio_text_layer(window_layer, GRect(PLATE_X + 2, 140, 124, 56),
                                   studio_font(RESOURCE_ID_FONT_BARLOW_52),
                                   GColorWhite, GTextAlignmentCenter);

  s_date_layer = studio_text_layer(
      window_layer, GRect(PLATE_X + 8, 198, 74, 18),
      fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GColorWhite,
      GTextAlignmentLeft);

  s_battery_layer = layer_create(GRect(PLATE_X + 86, 202, 34, 13));
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

STUDIO_VARIANT("signpost", prv_load, prv_unload, prv_tick)
