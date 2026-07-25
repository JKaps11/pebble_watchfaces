// phosphor — green CRT, pixel type, a divider rule. The mainframe pole.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// The other pole of the brief. Where ghostty holds its character in the face and
// the margin, this puts it all in the phosphor: one saturated green on black, a
// bitmap face, a caret and a divider. The question it answers is whether a
// green-screen watchface reads as period-correct or as costume.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define MARGIN 14
#define PHOSPHOR GColorScreaminGreen
#define RULE_Y 106

static Layer *s_rule_layer;
static TextLayer *s_caret_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static char s_battery_text[16];

static void prv_draw_rule(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, PHOSPHOR);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, 2), 0, GCornerNone);
}

static TextLayer *prv_label(Layer *parent, int y, const char *font_key) {
  return studio_text_layer(parent, GRect(MARGIN, y, 172, 24),
                           fonts_get_system_font(font_key), PHOSPHOR,
                           GTextAlignmentLeft);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_caret_layer = prv_label(window_layer, 16, FONT_KEY_GOTHIC_18_BOLD);
  text_layer_set_text(s_caret_layer, ">");

  // The custom faces are subset to digits, so every label stays on a system font.
  s_time_layer = studio_text_layer(
      window_layer, GRect(MARGIN, 42, 186 - MARGIN, 52),
      studio_font(RESOURCE_ID_FONT_SILKSCREEN_38), PHOSPHOR,
      GTextAlignmentLeft);

  s_rule_layer = layer_create(GRect(MARGIN, RULE_Y, 200 - (MARGIN * 2), 2));
  layer_set_update_proc(s_rule_layer, prv_draw_rule);
  layer_add_child(window_layer, s_rule_layer);

  s_date_layer = prv_label(window_layer, 116, FONT_KEY_GOTHIC_18);
  s_battery_layer = prv_label(window_layer, 138, FONT_KEY_GOTHIC_18);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_rule_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_caret_layer);
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

STUDIO_VARIANT("phosphor", prv_load, prv_unload, prv_tick)
