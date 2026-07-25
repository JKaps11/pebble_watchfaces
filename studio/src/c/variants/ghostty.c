// ghostty — the modern terminal default: off-white on near-black, block cursor.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=mono, type=custom, polarity=light-on-dark
//
// One pole of the brief. A contemporary terminal emulator's default is almost
// entirely achromatic: the character comes from the monospaced face, the left
// margin and the block cursor rather than from colour. This Variant asks whether
// that restraint survives being shrunk to 200x228, or whether what is left just
// reads as white text on black.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define MARGIN 14
#define CURSOR_GAP 8
#define CURSOR_WIDTH 17

static Layer *s_prompt_layer;
static TextLayer *s_sigil_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;

static GFont s_time_font;
static char s_time_text[16];
static char s_date_text[16];
static char s_battery_text[16];

// The cursor sits immediately after the time, so this row is drawn rather than
// laid out — the block's x depends on how wide the time actually rendered.
static void prv_draw_prompt(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GSize size = graphics_text_layout_get_content_size(
      s_time_text, s_time_font, bounds, GTextOverflowModeTrailingEllipsis,
      GTextAlignmentLeft);

  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, s_time_text, s_time_font, bounds,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft,
                     NULL);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(size.w + CURSOR_GAP, 16, CURSOR_WIDTH, 32), 0,
                     GCornerNone);
}

static TextLayer *prv_dim(Layer *parent, int y) {
  return studio_text_layer(parent, GRect(MARGIN, y, 172, 24),
                           fonts_get_system_font(FONT_KEY_GOTHIC_18),
                           GColorLightGray, GTextAlignmentLeft);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_time_font = studio_font(RESOURCE_ID_FONT_SHARETECH_46);

  s_sigil_layer = prv_dim(window_layer, 18);
  text_layer_set_text(s_sigil_layer, "~ $");

  s_prompt_layer = layer_create(GRect(MARGIN, 42, 200 - MARGIN, 60));
  layer_set_update_proc(s_prompt_layer, prv_draw_prompt);
  layer_add_child(window_layer, s_prompt_layer);

  s_date_layer = prv_dim(window_layer, 106);
  s_battery_layer = prv_dim(window_layer, 128);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_prompt_layer);
  text_layer_destroy(s_sigil_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text),
                s_date_text, sizeof(s_date_text));

  BatteryChargeState battery = battery_state_service_peek();
  snprintf(s_battery_text, sizeof(s_battery_text), "bat %d%%",
           battery.charge_percent);

  text_layer_set_text(s_date_layer, s_date_text);
  text_layer_set_text(s_battery_layer, s_battery_text);
  layer_mark_dirty(s_prompt_layer);
}

STUDIO_VARIANT("ghostty", prv_load, prv_unload, prv_tick)
