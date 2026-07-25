// statusline — one inverse-video field, the way a 3270 screen marks a value.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=one-hue, type=LECO, polarity=light-on-dark
//
// Tests whether the terminal read can come from layout alone. The numerals are
// LECO — geometric, nothing to do with a terminal — so everything saying
// "machine" here is the inverse-video field and the left margin. If this lands,
// the face matters less than the Batch's two custom-type poles assume.

#include <pebble.h>
#include "../studio_draw.h"
#include "../variant.h"

#define MARGIN 14
#define FIELD GColorGreen
#define FIELD_PAD 8
#define FIELD_HEIGHT 26

static Layer *s_field_layer;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static char s_battery_text[16];

// The field is sized to its text so it reads as a highlighted value rather than
// a full-width bar — a bar spanning the display would make this a split
// composition, which is not the axis position this Variant is declaring.
static void prv_draw_field(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GSize size = graphics_text_layout_get_content_size(
      s_date_text, font, bounds, GTextOverflowModeTrailingEllipsis,
      GTextAlignmentLeft);

  graphics_context_set_fill_color(ctx, FIELD);
  graphics_fill_rect(ctx, GRect(0, 0, size.w + (FIELD_PAD * 2), FIELD_HEIGHT),
                     0, GCornerNone);

  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, s_date_text, font,
                     GRect(FIELD_PAD, 1, size.w, FIELD_HEIGHT),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft,
                     NULL);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_field_layer = layer_create(GRect(MARGIN, 20, 200 - (MARGIN * 2),
                                     FIELD_HEIGHT));
  layer_set_update_proc(s_field_layer, prv_draw_field);
  layer_add_child(window_layer, s_field_layer);

  s_time_layer = studio_text_layer(
      window_layer, GRect(MARGIN - 4, 54, 200 - (MARGIN * 2) + 4, 52),
      fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS), GColorWhite,
      GTextAlignmentLeft);

  s_battery_layer = studio_text_layer(
      window_layer, GRect(MARGIN, 114, 200 - (MARGIN * 2), 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18), FIELD, GTextAlignmentLeft);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_time_layer);
  layer_destroy(s_field_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text),
                s_date_text, sizeof(s_date_text));

  BatteryChargeState battery = battery_state_service_peek();
  snprintf(s_battery_text, sizeof(s_battery_text), "BAT %d%%",
           battery.charge_percent);

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_battery_layer, s_battery_text);
  layer_mark_dirty(s_field_layer);
}

STUDIO_VARIANT("statusline", prv_load, prv_unload, prv_tick)
