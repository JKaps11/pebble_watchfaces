// flagstick — the minute read off the height of a flag on its pole.
//
// Axes: time-display=hybrid, composition=corner-anchored, complications=1-2,
//       hue=one-hue, type=Bitham, polarity=light-on-dark
//
// The hybrid position, and the one thing in the Batch that asks the theme to do
// real work rather than decorate: the pennant climbs the stick as the hour
// fills, so the pole is a minute hand that happens to look like golf. The hour
// is digital and large beside it. Reading a minute off a pole is imprecise by
// construction — this Variant exists to find out whether that is charming or
// simply wrong, which is not a question a measurement can answer.

#include <pebble.h>
#include "../studio_draw.h"
#include "../variant.h"

#define ACCENT GColorInchworm
#define TURF GColorDarkGreen
#define POLE_X 32
#define POLE_TOP 16
#define POLE_BASE 200

static Layer *s_stick_layer;
static TextLayer *s_hour_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_hour_text[8];
static char s_date_text[16];
static int s_battery_percent;
static int s_minute;

static void prv_draw_stick(Layer *layer, GContext *ctx) {
  // The green the pin is planted in.
  graphics_context_set_fill_color(ctx, TURF);
  graphics_fill_circle(ctx, GPoint(POLE_X, POLE_BASE + 8), 26);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(POLE_X, POLE_BASE), 5);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, GPoint(POLE_X, POLE_TOP), GPoint(POLE_X, POLE_BASE));

  // Quarter-hour ticks, so the pennant's height has something to be read against.
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, ACCENT);
  for (int quarter = 0; quarter <= 4; quarter++) {
    int y = POLE_BASE - ((POLE_BASE - POLE_TOP) * quarter / 4);
    graphics_draw_line(ctx, GPoint(POLE_X + 5, y), GPoint(POLE_X + 13, y));
  }

  int hoist_y = POLE_BASE - ((POLE_BASE - POLE_TOP) * s_minute / 59);
  if (hoist_y > POLE_BASE - 22) {
    hoist_y = POLE_BASE - 22;
  }

  graphics_context_set_stroke_color(ctx, ACCENT);
  graphics_context_set_stroke_width(ctx, 1);
  for (int i = 0; i <= 22; i++) {
    graphics_draw_line(ctx, GPoint(POLE_X + 44, hoist_y + 11),
                       GPoint(POLE_X + 2, hoist_y + i));
  }
}

static void prv_draw_battery(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent, ACCENT);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_stick_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_stick_layer, prv_draw_stick);
  layer_add_child(window_layer, s_stick_layer);

  s_hour_layer = studio_text_layer(
      window_layer, GRect(84, 96, 106, 52),
      fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS), GColorWhite,
      GTextAlignmentRight);

  s_date_layer = studio_text_layer(
      window_layer, GRect(84, 152, 106, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentRight);

  s_battery_layer = layer_create(GRect(152, 182, 38, 15));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_hour_layer);
  layer_destroy(s_stick_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, NULL, 0, s_date_text, sizeof(s_date_text));
  snprintf(s_hour_text, sizeof(s_hour_text), "%02d", now->tm_hour);
  s_battery_percent = battery_state_service_peek().charge_percent;
  s_minute = now->tm_min;

  text_layer_set_text(s_hour_layer, s_hour_text);
  text_layer_set_text(s_date_layer, s_date_text);
  layer_mark_dirty(s_battery_layer);
  layer_mark_dirty(s_stick_layer);
}

STUDIO_VARIANT("flagstick", prv_load, prv_unload, prv_tick)
