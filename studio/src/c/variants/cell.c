// cell — the battery as a conventional cell icon with its percentage beside it.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// The direct answer to replacing the BAT label: the icon takes the word's place
// and the number stays. Belt and braces — the shape says what the field is, the
// digits say how much — which is either reassuring or redundant, and that is the
// thing to look at against gauge, where the number is gone.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define MARGIN 14
#define PHOSPHOR GColorScreaminGreen
#define RULE_Y 106
#define ICON_Y 142

static Layer *s_rule_layer;
static Layer *s_icon_layer;
static TextLayer *s_caret_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static char s_battery_text[16];
static int s_battery_percent;

static void prv_draw_rule(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, PHOSPHOR);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, 2), 0, GCornerNone);
}

static void prv_draw_icon(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent, PHOSPHOR);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_caret_layer = studio_text_layer(
      window_layer, GRect(MARGIN, 16, 172, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), PHOSPHOR,
      GTextAlignmentLeft);
  text_layer_set_text(s_caret_layer, ">");

  s_time_layer = studio_text_layer(
      window_layer, GRect(MARGIN, 42, 186 - MARGIN, 52),
      studio_font(RESOURCE_ID_FONT_SILKSCREEN_38), PHOSPHOR,
      GTextAlignmentLeft);

  s_rule_layer = layer_create(GRect(MARGIN, RULE_Y, 200 - (MARGIN * 2), 2));
  layer_set_update_proc(s_rule_layer, prv_draw_rule);
  layer_add_child(window_layer, s_rule_layer);

  s_date_layer = studio_text_layer(
      window_layer, GRect(MARGIN, 116, 172, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18), PHOSPHOR, GTextAlignmentLeft);

  s_icon_layer = layer_create(GRect(MARGIN, ICON_Y, 26, 13));
  layer_set_update_proc(s_icon_layer, prv_draw_icon);
  layer_add_child(window_layer, s_icon_layer);

  s_battery_layer = studio_text_layer(
      window_layer, GRect(MARGIN + 34, ICON_Y - 4, 120, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18), PHOSPHOR, GTextAlignmentLeft);
}

static void prv_unload(Window *window) {
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

  s_battery_percent = battery_state_service_peek().charge_percent;
  snprintf(s_battery_text, sizeof(s_battery_text), "%d%%", s_battery_percent);

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  text_layer_set_text(s_battery_layer, s_battery_text);
  layer_mark_dirty(s_icon_layer);
}

STUDIO_VARIANT("cell", prv_load, prv_unload, prv_tick)
