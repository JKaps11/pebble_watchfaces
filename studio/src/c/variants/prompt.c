// prompt — three shell lines, each with its own sigil. System type throughout.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=one-hue, type=Bitham, polarity=light-on-dark
//
// The control on the type axis. ghostty and phosphor both buy their terminal
// read with a custom face; this one buys it purely with structure — a green
// sigil starting every line, output aligned behind it — and spends nothing on
// resources. If it holds its own on the sheet, the custom faces are decoration.

#include <pebble.h>
#include "../studio_draw.h"
#include "../variant.h"

#define MARGIN 14
#define SIGIL GColorGreen
#define OUTPUT_X 40

static TextLayer *s_time_sigil_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_sigil_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_sigil_layer;
static TextLayer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static char s_battery_text[16];

static TextLayer *prv_sigil(Layer *parent, int y, const char *font_key) {
  TextLayer *layer = studio_text_layer(parent, GRect(MARGIN, y, 24, 32),
                                       fonts_get_system_font(font_key), SIGIL,
                                       GTextAlignmentLeft);
  text_layer_set_text(layer, "$");
  return layer;
}

static TextLayer *prv_output(Layer *parent, int y, int height,
                             const char *font_key) {
  return studio_text_layer(parent, GRect(OUTPUT_X, y, 200 - OUTPUT_X - MARGIN,
                                         height),
                           fonts_get_system_font(font_key), GColorWhite,
                           GTextAlignmentLeft);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_time_sigil_layer = prv_sigil(window_layer, 32, FONT_KEY_GOTHIC_24_BOLD);
  // BITHAM_42_MEDIUM_NUMBERS carries digits and punctuation only, which is all
  // the time needs; the lines below it are set in a full system font.
  s_time_layer = prv_output(window_layer, 20, 52,
                            FONT_KEY_BITHAM_42_MEDIUM_NUMBERS);

  s_date_sigil_layer = prv_sigil(window_layer, 100, FONT_KEY_GOTHIC_18_BOLD);
  s_date_layer = prv_output(window_layer, 100, 24, FONT_KEY_GOTHIC_18);

  s_battery_sigil_layer = prv_sigil(window_layer, 124, FONT_KEY_GOTHIC_18_BOLD);
  s_battery_layer = prv_output(window_layer, 124, 24, FONT_KEY_GOTHIC_18);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_battery_sigil_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_date_sigil_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_time_sigil_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text),
                s_date_text, sizeof(s_date_text));

  BatteryChargeState battery = battery_state_service_peek();
  snprintf(s_battery_text, sizeof(s_battery_text), "bat %d%%",
           battery.charge_percent);

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  text_layer_set_text(s_battery_layer, s_battery_text);
}

STUDIO_VARIANT("prompt", prv_load, prv_unload, prv_tick)
