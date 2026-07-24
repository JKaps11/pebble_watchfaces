// corner — mass pushed into the bottom-left, with one accent hue.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=one-hue, type=LECO, polarity=dark-on-light
//
// Two questions at once, which is what a Spread is for: what a design looks like
// when nothing is centred, and what a single accent hue buys. Restraint with
// colour correlated with popularity in the surveyed corpus — 43% of faces use
// exactly one hue besides black, white and grey — so one accent is the position
// worth seeing rather than none or many.

#include <pebble.h>
#include "../variant.h"
#include "datetime_format.h"

#define ACCENT GColorOrange

static Layer *s_rule_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_rule(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, ACCENT);
  graphics_fill_rect(ctx, GRect(0, 0, 56, bounds.size.h), 0, GCornerNone);
}

static TextLayer *prv_text_layer(Layer *parent, GRect frame, const char *font,
                                 GColor colour) {
  TextLayer *layer = text_layer_create(frame);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, colour);
  text_layer_set_font(layer, fonts_get_system_font(font));
  text_layer_set_text_alignment(layer, GTextAlignmentLeft);
  layer_add_child(parent, text_layer_get_layer(layer));
  return layer;
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorWhite);

  s_rule_layer = layer_create(GRect(14, 24, bounds.size.w, 6));
  layer_set_update_proc(s_rule_layer, prv_draw_rule);
  layer_add_child(window_layer, s_rule_layer);

  s_date_layer = prv_text_layer(window_layer,
                                GRect(14, 132, bounds.size.w - 20, 24),
                                FONT_KEY_GOTHIC_18_BOLD, GColorBlack);
  s_time_layer = prv_text_layer(window_layer,
                                GRect(10, 156, bounds.size.w - 16, 52),
                                FONT_KEY_LECO_42_NUMBERS, GColorBlack);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_rule_layer);
}

static void prv_tick(struct tm *now) {
  DateTimeInfo info = {
    .hour = now->tm_hour,
    .minute = now->tm_min,
    .weekday = now->tm_wday,
    .month = now->tm_mon,
    .day_of_month = now->tm_mday,
  };
  datetime_format_time(&info, clock_is_24h_style(), s_time_text,
                       sizeof(s_time_text));
  datetime_format_date(&info, s_date_text, sizeof(s_date_text));
  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
}

STUDIO_VARIANT("corner", prv_load, prv_unload, prv_tick)
