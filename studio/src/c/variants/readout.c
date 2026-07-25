// readout — the full status dump. The far end of the sweep, and the boundary.
//
// Axes: time-display=digital, composition=corner-anchored, complications=5+,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// This is the tile the brief ruled out, built anyway because a Sweep that stops
// short of its own axis proves nothing. A mainframe console really did dump
// every field it had, so this is the most literally faithful Variant in the
// Batch and, in all likelihood, the least wearable — which is the finding. Its
// fields are all clock- or battery-derived, so every one of them moves under the
// Stress state rather than sitting there looking like data.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define MARGIN 14
#define PHOSPHOR GColorScreaminGreen
#define RULE_Y 100
#define ROW_HEIGHT 18
#define FIRST_ROW 108

static Layer *s_rule_layer;
static Layer *s_icon_layer;
static TextLayer *s_caret_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_week_layer;
static TextLayer *s_yearday_layer;
static TextLayer *s_power_layer;

static char s_time_text[16];
static char s_date_text[16];
static char s_battery_text[16];
static char s_week_text[16];
static char s_yearday_text[16];
static int s_battery_percent;

static void prv_draw_rule(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, PHOSPHOR);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, 2), 0, GCornerNone);
}

static void prv_draw_icon(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent, PHOSPHOR);
}

static TextLayer *prv_row(Layer *parent, int row, int x) {
  return studio_text_layer(
      parent, GRect(x, FIRST_ROW + (row * ROW_HEIGHT), 200 - x - MARGIN,
                    ROW_HEIGHT),
      fonts_get_system_font(FONT_KEY_GOTHIC_14), PHOSPHOR, GTextAlignmentLeft);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_caret_layer = studio_text_layer(
      window_layer, GRect(MARGIN, 14, 172, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), PHOSPHOR,
      GTextAlignmentLeft);
  text_layer_set_text(s_caret_layer, ">");

  s_time_layer = studio_text_layer(
      window_layer, GRect(MARGIN, 38, 186 - MARGIN, 52),
      studio_font(RESOURCE_ID_FONT_SILKSCREEN_38), PHOSPHOR,
      GTextAlignmentLeft);

  s_rule_layer = layer_create(GRect(MARGIN, RULE_Y, 200 - (MARGIN * 2), 2));
  layer_set_update_proc(s_rule_layer, prv_draw_rule);
  layer_add_child(window_layer, s_rule_layer);

  s_date_layer = prv_row(window_layer, 0, MARGIN);

  s_icon_layer = layer_create(
      GRect(MARGIN, FIRST_ROW + ROW_HEIGHT + 3, 22, 11));
  layer_set_update_proc(s_icon_layer, prv_draw_icon);
  layer_add_child(window_layer, s_icon_layer);

  s_battery_layer = prv_row(window_layer, 1, MARGIN + 30);
  s_week_layer = prv_row(window_layer, 2, MARGIN);
  s_yearday_layer = prv_row(window_layer, 3, MARGIN);
  s_power_layer = prv_row(window_layer, 4, MARGIN);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_power_layer);
  text_layer_destroy(s_yearday_layer);
  text_layer_destroy(s_week_layer);
  text_layer_destroy(s_battery_layer);
  layer_destroy(s_icon_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_rule_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_caret_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text),
                s_date_text, sizeof(s_date_text));

  BatteryChargeState battery = battery_state_service_peek();
  s_battery_percent = battery.charge_percent;
  snprintf(s_battery_text, sizeof(s_battery_text), "%d%%", s_battery_percent);

  char week[8];
  strftime(week, sizeof(week), "%V", now);
  snprintf(s_week_text, sizeof(s_week_text), "WEEK %s", week);
  snprintf(s_yearday_text, sizeof(s_yearday_text), "DAY %d", now->tm_yday + 1);

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  text_layer_set_text(s_battery_layer, s_battery_text);
  text_layer_set_text(s_week_layer, s_week_text);
  text_layer_set_text(s_yearday_layer, s_yearday_text);
  text_layer_set_text(s_power_layer, battery.is_charging ? "PWR EXT" : "PWR BAT");
}

STUDIO_VARIANT("readout", prv_load, prv_unload, prv_tick)
