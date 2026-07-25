// fairway — the hole from the tee, mown in stripes, time low on the left.
//
// Axes: time-display=digital, composition=asymmetric, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// The literal reading of the brief: the face is a golf hole seen from above,
// with black rough either side of a dogleg that narrows toward a green at the
// top. Its question is whether a representational background can carry a theme
// without the time having to fight it — the mowing stripes are the whole
// gamble, since they put tonal variation directly behind the numerals.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define TURF GColorDarkGreen
#define STRIPE GColorMayGreen
#define GREEN_SURFACE GColorIslamicGreen
#define TOP_Y 10
#define BOTTOM_Y 220

static Layer *s_hole_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

// The fairway centre line and half-width at a given row. Narrow and swung left
// at the top where the green sits, wide and central at the tee.
static int prv_centre_at(int y) {
  int32_t along = TRIG_MAX_ANGLE * (y - TOP_Y) / ((BOTTOM_Y - TOP_Y) * 2);
  return 62 + (46 * sin_lookup(along) / TRIG_MAX_RATIO);
}

static int prv_half_width_at(int y) {
  return 16 + (58 * (y - TOP_Y) / (BOTTOM_Y - TOP_Y));
}

static void prv_draw_hole(Layer *layer, GContext *ctx) {
  for (int y = TOP_Y; y < BOTTOM_Y; y++) {
    int centre = prv_centre_at(y);
    int half = prv_half_width_at(y);
    // A cylinder mower leaves alternating bands across the hole.
    graphics_context_set_fill_color(ctx, ((y / 15) % 2) ? STRIPE : TURF);
    graphics_fill_rect(ctx, GRect(centre - half, y, half * 2, 1), 0,
                       GCornerNone);
  }

  // The green at the far end, with the pin standing in it.
  GPoint green = GPoint(prv_centre_at(TOP_Y + 14), TOP_Y + 22);
  graphics_context_set_fill_color(ctx, GREEN_SURFACE);
  graphics_fill_circle(ctx, green, 20);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, green, 3);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, green, GPoint(green.x, green.y - 26));
  // The pennant, filled by fanning strokes from its point to its hoist edge.
  for (int i = 0; i <= 12; i++) {
    graphics_draw_line(ctx, GPoint(green.x + 16, green.y - 20),
                       GPoint(green.x + 1, green.y - 26 + i));
  }
}

static void prv_draw_battery(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent,
                      GColorWhite);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_hole_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_hole_layer, prv_draw_hole);
  layer_add_child(window_layer, s_hole_layer);

  s_battery_layer = layer_create(GRect(150, 14, 38, 15));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);

  s_date_layer = studio_text_layer(
      window_layer, GRect(12, 128, 150, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentLeft);

  s_time_layer = studio_text_layer(
      window_layer, GRect(10, 148, 150, 62),
      studio_font(RESOURCE_ID_FONT_BARLOW_52), GColorWhite,
      GTextAlignmentLeft);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_battery_layer);
  layer_destroy(s_hole_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), s_date_text,
                sizeof(s_date_text));
  s_battery_percent = battery_state_service_peek().charge_percent;

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  layer_mark_dirty(s_battery_layer);
}

STUDIO_VARIANT("fairway", prv_load, prv_unload, prv_tick)
