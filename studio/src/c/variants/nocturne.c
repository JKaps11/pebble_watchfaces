// nocturne — the same idea as plain, inverted.
//
// Axes: time-display=digital, composition=centred, complications=1-2,
//       hue=mono, type=Bitham, polarity=light-on-dark
//
// Exists to put ink polarity on a Contact sheet as its own question. On a
// reflective display polarity is a legibility decision rather than a mood one:
// light marks on a dark ground have less to reflect, so this is the Variant
// whose Stress contrast measurement is worth reading closely.

#include <pebble.h>
#include "../studio_draw.h"
#include "../variant.h"

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static char s_time_text[16];
static char s_date_text[16];

static TextLayer *prv_centred(Layer *parent, GRect frame, const char *font_key) {
  return studio_text_layer(parent, frame, fonts_get_system_font(font_key),
                           GColorWhite, GTextAlignmentCenter);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);

  s_time_layer = prv_centred(window_layer, GRect(0, 78, bounds.size.w, 52),
                                FONT_KEY_BITHAM_42_MEDIUM_NUMBERS);
  s_date_layer = prv_centred(window_layer, GRect(0, 134, bounds.size.w, 26),
                                FONT_KEY_GOTHIC_18);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text),
                s_date_text, sizeof(s_date_text));
  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
}

STUDIO_VARIANT("nocturne", prv_load, prv_unload, prv_tick)
