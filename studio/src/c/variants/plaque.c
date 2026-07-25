// plaque — yardage as it was, with the course behind it given somewhere to go.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=one-hue, type=LECO, polarity=light-on-dark
//
// The straight refinement rather than a new idea: same corner-anchored plate,
// same LECO numerals, but the turf now recedes — the stripes deepen toward the
// viewer and the green sits small and far off in the corner the plate leaves
// empty. Nothing about the time changed, which is the point. This is here to
// show how much of the added background the original was already carrying, and
// how little it costs to add the rest.

#include <pebble.h>
#include "../studio_draw.h"
#include "../variant.h"

#define TURF GColorDarkGreen
#define STRIPE GColorMayGreen
#define SURFACE GColorIslamicGreen
#define ACCENT GColorInchworm
#define PLATE_X 8
#define PLATE_Y 100
#define PLATE_W 152
#define PLATE_H 116

static Layer *s_scene_layer;
static TextLayer *s_label_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, TURF);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Stripes deepening toward the viewer, which is the whole perspective cue.
  int y = 0;
  int depth = 4;
  for (int band = 0; y < bounds.size.h; band++) {
    if (band % 2) {
      graphics_context_set_fill_color(ctx, STRIPE);
      graphics_fill_rect(ctx, GRect(0, y, bounds.size.w, depth), 0,
                         GCornerNone);
    }
    y += depth;
    depth += 2;
  }

  // The green, far off in the corner the plate does not use.
  studio_fill_ellipse(ctx, GPoint(152, 46), 38, 15, SURFACE);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(151, 44, 3, 2), 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(152, 44), GPoint(152, 14));
  studio_fill_triangle(ctx, GPoint(180, 21), GPoint(153, 14), GPoint(153, 28),
                       GColorWhite);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(PLATE_X, PLATE_Y, PLATE_W, PLATE_H), 6,
                     GCornersAll);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_round_rect(ctx, GRect(PLATE_X, PLATE_Y, PLATE_W, PLATE_H), 6);

  graphics_context_set_stroke_color(ctx, ACCENT);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(PLATE_X + 12, 180),
                     GPoint(PLATE_X + PLATE_W - 12, 180));
}

static void prv_draw_battery(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent, ACCENT);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

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
      window_layer, GRect(PLATE_X + 12, 184, 92, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentLeft);

  s_battery_layer = layer_create(GRect(PLATE_X + 106, 189, 34, 14));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_label_layer);
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

STUDIO_VARIANT("plaque", prv_load, prv_unload, prv_tick)
