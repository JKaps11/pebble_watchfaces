// teletype — printed on paper: black on white, small type, a ruled line.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=mono, type=Gothic, polarity=dark-on-light
//
// The terminal before the screen was a printer, and nobody asks for this one.
// It is here for a reason specific to this hardware: Pebble's display is
// reflective rather than emissive, so the dark ground every other Variant in the
// Batch assumes is a sunlight-legibility decision as much as a mood one. Gothic
// stops at 28px, so this is also the honest ceiling of the system-type position.

#include <pebble.h>
#include "../studio_draw.h"
#include "../variant.h"

#define MARGIN 14
#define RULE_Y 96

static Layer *s_rule_layer;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static char s_battery_text[16];

static void prv_draw_rule(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, 1), 0, GCornerNone);
}

static TextLayer *prv_line(Layer *parent, int y, int height,
                           const char *font_key, GColor colour) {
  return studio_text_layer(parent, GRect(MARGIN, y, 200 - (MARGIN * 2), height),
                           fonts_get_system_font(font_key), colour,
                           GTextAlignmentLeft);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorWhite);

  s_date_layer = prv_line(window_layer, 26, 24, FONT_KEY_GOTHIC_18,
                          GColorDarkGray);
  s_time_layer = prv_line(window_layer, 50, 40, FONT_KEY_GOTHIC_28_BOLD,
                          GColorBlack);

  s_rule_layer = layer_create(GRect(MARGIN, RULE_Y, 200 - (MARGIN * 2), 1));
  layer_set_update_proc(s_rule_layer, prv_draw_rule);
  layer_add_child(window_layer, s_rule_layer);

  s_battery_layer = prv_line(window_layer, 104, 24, FONT_KEY_GOTHIC_18,
                             GColorDarkGray);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_battery_layer);
  layer_destroy(s_rule_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text),
                s_date_text, sizeof(s_date_text));

  BatteryChargeState battery = battery_state_service_peek();
  snprintf(s_battery_text, sizeof(s_battery_text), "BAT %d%%",
           battery.charge_percent);

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  text_layer_set_text(s_battery_layer, s_battery_text);
}

STUDIO_VARIANT("teletype", prv_load, prv_unload, prv_tick)
