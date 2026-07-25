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
#include "../studio_draw.h"
#include "../variant.h"

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

static TextLayer *prv_left(Layer *parent, GRect frame, const char *font_key,
                           GColor colour) {
  return studio_text_layer(parent, frame, fonts_get_system_font(font_key),
                           colour, GTextAlignmentLeft);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorWhite);

  s_rule_layer = layer_create(GRect(14, 24, bounds.size.w, 6));
  layer_set_update_proc(s_rule_layer, prv_draw_rule);
  layer_add_child(window_layer, s_rule_layer);

  s_date_layer = prv_left(window_layer,
                                GRect(14, 132, bounds.size.w - 20, 24),
                                FONT_KEY_GOTHIC_18_BOLD, GColorBlack);
  s_time_layer = prv_left(window_layer,
                                GRect(10, 156, bounds.size.w - 16, 52),
                                FONT_KEY_LECO_42_NUMBERS, GColorBlack);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_rule_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text),
                s_date_text, sizeof(s_date_text));
  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
}

STUDIO_VARIANT("corner", prv_load, prv_unload, prv_tick)
