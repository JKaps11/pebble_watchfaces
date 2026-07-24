// dense — everything on at once.
//
// Axes: digital · centred · 3–4 complications · mono · LECO · dark-on-light
//
// Carries every complication the emulator can inject state for, which makes it
// the Variant that shows whether injected state actually landed. Its density is
// the point: it is the far end of the complication-count axis, and a Batch needs
// one to compare the sparse end against.
//
// Its complications are date, battery and week number — all three of which
// actually move between the Canonical and Stress states. Step count and
// Bluetooth are avoided on purpose: neither can be injected on this emulator, so
// a Variant leaning on them would render identically in both states and the
// Stress render would not stress it. See studio/tools/studio/states.py.

#include <pebble.h>
#include "../variant.h"
#include "datetime_format.h"

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_complications_layer;

static char s_time_text[16];
static char s_date_text[16];
static char s_complications_text[48];

static TextLayer *prv_text_layer(Layer *parent, GRect frame, const char *font_key) {
  TextLayer *layer = text_layer_create(frame);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, GColorBlack);
  text_layer_set_font(layer, fonts_get_system_font(font_key));
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
  layer_add_child(parent, text_layer_get_layer(layer));
  return layer;
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorWhite);

  s_time_layer = prv_text_layer(window_layer, GRect(0, 60, bounds.size.w, 50),
                                FONT_KEY_LECO_42_NUMBERS);
  s_date_layer = prv_text_layer(window_layer, GRect(0, 112, bounds.size.w, 28),
                                FONT_KEY_GOTHIC_24_BOLD);
  s_complications_layer = prv_text_layer(
      window_layer, GRect(0, 150, bounds.size.w, 60), FONT_KEY_GOTHIC_18);
  text_layer_set_overflow_mode(s_complications_layer, GTextOverflowModeWordWrap);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_complications_layer);
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
  datetime_format_time(&info, clock_is_24h_style(), s_time_text, sizeof(s_time_text));
  datetime_format_date(&info, s_date_text, sizeof(s_date_text));

  char week_text[8];
  strftime(week_text, sizeof(week_text), "%V", now);

  BatteryChargeState battery = battery_state_service_peek();
  snprintf(s_complications_text, sizeof(s_complications_text),
           "%d%% battery\nweek %s",
           battery.charge_percent, week_text);

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  text_layer_set_text(s_complications_layer, s_complications_text);
}

STUDIO_VARIANT("dense", prv_load, prv_unload, prv_tick)
