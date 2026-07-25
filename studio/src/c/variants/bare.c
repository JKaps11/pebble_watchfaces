// bare — phosphor with nothing but the time. The zero end of the sweep.
//
// Axes: time-display=digital, composition=corner-anchored, complications=0,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// The control for a Batch about how the battery should read: it asks whether it
// should be there at all. Everything else in this Sweep is this Variant plus
// something, so whatever the added complications are worth is the difference
// between this tile and its neighbours.

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

static char s_time_text[16];

static void prv_draw_rule(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, PHOSPHOR);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, 2), 0, GCornerNone);
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
}

static void prv_unload(Window *window) {
  layer_destroy(s_rule_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_caret_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), NULL, 0);
  text_layer_set_text(s_time_layer, s_time_text);
}

STUDIO_VARIANT("bare", prv_load, prv_unload, prv_tick)
