// yardage — the sprinkler-head plate, with the time where the distance goes.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=one-hue, type=LECO, polarity=light-on-dark
//
// Golf by way of its signage rather than its scenery. A yardage plate is already
// a small dark panel carrying one big number on a field of grass, which is a
// watchface, so the theme costs the layout almost nothing — the turf and the pin
// stay out at the edges and the panel keeps a hard rectangle for the numerals to
// sit in. The safe entry in the Batch, and worth having as the thing the wilder
// ones have to beat.

#include <pebble.h>
#include "../studio_draw.h"
#include "../variant.h"

#define TURF GColorDarkGreen
#define STRIPE GColorMayGreen
#define ACCENT GColorInchworm
#define PLATE_X 8
#define PLATE_Y 100
#define PLATE_W 152
#define PLATE_H 114

static Layer *s_scene_layer;
static TextLayer *s_par_layer;
static TextLayer *s_label_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  for (int y = 0; y < bounds.size.h; y += 14) {
    graphics_context_set_fill_color(ctx, ((y / 14) % 2) ? STRIPE : TURF);
    graphics_fill_rect(ctx, GRect(0, y, bounds.size.w, 14), 0, GCornerNone);
  }

  // The pin, standing out on the part of the turf the plate does not cover.
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(168, 18), GPoint(168, 88));
  graphics_context_set_stroke_width(ctx, 1);
  for (int i = 0; i <= 18; i++) {
    graphics_draw_line(ctx, GPoint(190, 27), GPoint(169, 18 + i));
  }
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(168, 90), 4);

  // The plate itself.
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(PLATE_X, PLATE_Y, PLATE_W, PLATE_H), 6,
                     GCornersAll);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_round_rect(ctx, GRect(PLATE_X, PLATE_Y, PLATE_W, PLATE_H), 6);

  graphics_context_set_stroke_color(ctx, ACCENT);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(PLATE_X + 12, 178),
                     GPoint(PLATE_X + PLATE_W - 12, 178));
}

static void prv_draw_battery(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent, ACCENT);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_par_layer = studio_text_layer(
      window_layer, GRect(12, 12, 100, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GColorWhite,
      GTextAlignmentLeft);
  text_layer_set_text(s_par_layer, "16TH");

  s_label_layer = studio_text_layer(
      window_layer, GRect(PLATE_X + 12, PLATE_Y + 6, PLATE_W - 24, 18),
      fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), ACCENT,
      GTextAlignmentLeft);
  text_layer_set_text(s_label_layer, "TO THE PIN");

  s_time_layer = studio_text_layer(
      window_layer, GRect(PLATE_X + 4, PLATE_Y + 26, PLATE_W - 8, 50),
      fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS), GColorWhite,
      GTextAlignmentCenter);

  s_date_layer = studio_text_layer(
      window_layer, GRect(PLATE_X + 12, 182, 92, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentLeft);

  s_battery_layer = layer_create(GRect(PLATE_X + 106, 187, 34, 14));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_label_layer);
  text_layer_destroy(s_par_layer);
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

STUDIO_VARIANT("yardage", prv_load, prv_unload, prv_tick)
