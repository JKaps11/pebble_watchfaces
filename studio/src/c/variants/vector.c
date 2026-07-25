// vector — a label flush to the top-left, its body hanging low and right.
//
// Axes: time-display=digital, composition=asymmetric, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// Assembly source is already asymmetric and nobody thinks of it that way: labels
// sit in column one and everything else is pushed out to a tab stop, which is a
// deliberate imbalance the form has used for fifty years. This exaggerates that
// until it becomes the composition — a light upper-left corner against a heavy
// lower-right block — to see whether the layout can carry the idea by itself.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define PHOSPHOR GColorScreaminGreen
#define LABEL_X 12
#define BODY_X 66
#define BODY_W 122

static TextLayer *s_label_layer;
static TextLayer *s_time_layer;
static TextLayer *s_weekday_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;

static char s_time_text[24];
static char s_weekday_text[24];
static char s_date_text[24];
static char s_battery_text[24];

static TextLayer *prv_body(Layer *parent, int y) {
  return studio_text_layer(parent, GRect(BODY_X, y, BODY_W, 26),
                           studio_font(RESOURCE_ID_FONT_MONO_18), PHOSPHOR,
                           GTextAlignmentLeft);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_label_layer = studio_text_layer(
      window_layer, GRect(LABEL_X, 20, 176, 34),
      studio_font(RESOURCE_ID_FONT_MONO_24), PHOSPHOR, GTextAlignmentLeft);
  text_layer_set_text(s_label_layer, "RESET:");

  s_time_layer = studio_text_layer(
      window_layer, GRect(BODY_X, 92, BODY_W, 34),
      studio_font(RESOURCE_ID_FONT_MONO_24), PHOSPHOR, GTextAlignmentLeft);

  s_weekday_layer = prv_body(window_layer, 132);
  s_date_layer = prv_body(window_layer, 156);
  s_battery_layer = prv_body(window_layer, 180);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_weekday_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_label_layer);
}

static void prv_tick(struct tm *now) {
  char time_text[16];
  studio_format(now, time_text, sizeof(time_text), NULL, 0);
  snprintf(s_time_text, sizeof(s_time_text), "JP %s", time_text);

  char weekday[8];
  strftime(weekday, sizeof(weekday), "%a", now);
  studio_upper(weekday);
  snprintf(s_weekday_text, sizeof(s_weekday_text), "DB %s", weekday);

  char date[12];
  strftime(date, sizeof(date), "%b %d", now);
  studio_upper(date);
  snprintf(s_date_text, sizeof(s_date_text), "DB %s", date);

  snprintf(s_battery_text, sizeof(s_battery_text), "DB %03d",
           battery_state_service_peek().charge_percent);

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_weekday_layer, s_weekday_text);
  text_layer_set_text(s_date_layer, s_date_text);
  text_layer_set_text(s_battery_layer, s_battery_text);
}

STUDIO_VARIANT("vector", prv_load, prv_unload, prv_tick)
