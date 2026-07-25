// segments — LCD-style numerals, the character carried entirely by the face.
//
// Axes: time-display=digital, composition=centred, complications=1-2,
//       hue=mono, type=custom, polarity=light-on-dark
//
// The custom position on the type axis. It exists to answer whether a Variant's
// character can come from the typeface alone: everything else here is the same
// centred, mono, light-on-dark arrangement as nocturne, so any difference on the
// Contact sheet between the two is the face and nothing else.

#include <pebble.h>
#include "../studio_font.h"
#include "../studio_draw.h"
#include "../variant.h"

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static char s_time_text[16];
static char s_date_text[16];

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);

  s_time_layer = studio_text_layer(
      window_layer, GRect(0, 76, bounds.size.w, 56),
      studio_font(RESOURCE_ID_FONT_WALLPOET_44),
      GColorWhite, GTextAlignmentCenter);

  // The custom faces are subset to digits, so the date stays on a system font.
  s_date_layer = studio_text_layer(
      window_layer, GRect(0, 140, bounds.size.w, 26),
      fonts_get_system_font(FONT_KEY_GOTHIC_18),
      GColorWhite, GTextAlignmentCenter);
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

STUDIO_VARIANT("segments", prv_load, prv_unload, prv_tick)
