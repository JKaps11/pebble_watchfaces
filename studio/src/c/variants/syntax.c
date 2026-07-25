// syntax — coloured keys, plain values, the way a terminal colourises output.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The boundary test for a brief that asked for essentials only. Highlighting is
// the one place a real terminal legitimately uses more than one hue, and it
// carries information — the colour is what tells you which field you are looking
// at. Either that reads as native to the idiom, or three hues on a wrist reads
// as the clutter the brief ruled out. Nothing measures that; it has to be seen.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define MARGIN 14
#define VALUE_X 58
#define KEY_DATE GColorPictonBlue
#define KEY_BATTERY GColorChromeYellow
#define VALUE_BATTERY GColorScreaminGreen

static TextLayer *s_time_layer;
static TextLayer *s_date_key_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_key_layer;
static TextLayer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static char s_battery_text[16];

static TextLayer *prv_key(Layer *parent, int y, GColor colour,
                          const char *text) {
  TextLayer *layer = studio_text_layer(
      parent, GRect(MARGIN, y, VALUE_X - MARGIN, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18), colour, GTextAlignmentLeft);
  text_layer_set_text(layer, text);
  return layer;
}

static TextLayer *prv_value(Layer *parent, int y, GColor colour) {
  return studio_text_layer(parent, GRect(VALUE_X, y, 200 - VALUE_X - MARGIN, 24),
                           fonts_get_system_font(FONT_KEY_GOTHIC_18), colour,
                           GTextAlignmentLeft);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_time_layer = studio_text_layer(
      window_layer, GRect(MARGIN, 30, 200 - MARGIN, 58),
      studio_font(RESOURCE_ID_FONT_SHARETECH_46), GColorWhite,
      GTextAlignmentLeft);

  s_date_key_layer = prv_key(window_layer, 104, KEY_DATE, "date");
  s_date_layer = prv_value(window_layer, 104, GColorWhite);

  s_battery_key_layer = prv_key(window_layer, 128, KEY_BATTERY, "bat");
  s_battery_layer = prv_value(window_layer, 128, VALUE_BATTERY);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_battery_key_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_date_key_layer);
  text_layer_destroy(s_time_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text),
                s_date_text, sizeof(s_date_text));

  BatteryChargeState battery = battery_state_service_peek();
  snprintf(s_battery_text, sizeof(s_battery_text), "%d%%",
           battery.charge_percent);

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  text_layer_set_text(s_battery_layer, s_battery_text);
}

STUDIO_VARIANT("syntax", prv_load, prv_unload, prv_tick)
