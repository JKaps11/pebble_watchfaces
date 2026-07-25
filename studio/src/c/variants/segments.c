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
#include "../variant.h"
#include "datetime_format.h"

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static char s_time_text[16];
static char s_date_text[16];

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);

  s_time_layer = text_layer_create(GRect(0, 76, bounds.size.w, 56));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, studio_font(RESOURCE_ID_FONT_WALLPOET_44));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // The custom faces are subset to digits, so the date stays on a system font.
  s_date_layer = text_layer_create(GRect(0, 140, bounds.size.w, 26));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
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

STUDIO_VARIANT("segments", prv_load, prv_unload, prv_tick)
