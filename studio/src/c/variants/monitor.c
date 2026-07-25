// monitor — a machine-monitor memory dump, set as a centred block.
//
// Axes: time-display=digital, composition=centred, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// Centring monospaced text normally destroys it, because the columns stop
// lining up and the thing it was imitating falls apart. So the block is centred
// and the text inside it is not: the frame sits at a computed left edge wide
// enough for the longest line, which keeps the address column straight. Whether
// a listing can be centred at all without looking like a poster is the question.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define PHOSPHOR GColorScreaminGreen
// Sized for the longest line, "0104 TUE JUN16", at eight pixels a character,
// then centred on the 200 px display as a block rather than line by line.
#define BLOCK_X 38
#define BLOCK_W 124
#define TOP 76
#define PITCH 22
#define LINE_HEIGHT 22
#define LINE_COUNT 4

static TextLayer *s_lines[LINE_COUNT];
static char s_time_line[24];
static char s_date_line[24];
static char s_battery_line[24];

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  for (unsigned i = 0; i < LINE_COUNT; i++) {
    s_lines[i] = studio_text_layer(
        window_layer, GRect(BLOCK_X, TOP + (i * PITCH), BLOCK_W, LINE_HEIGHT),
        studio_font(RESOURCE_ID_FONT_MONO_14), PHOSPHOR, GTextAlignmentLeft);
  }

  text_layer_set_text(s_lines[0], ".D 0100 0110");
}

static void prv_unload(Window *window) {
  for (unsigned i = 0; i < LINE_COUNT; i++) {
    text_layer_destroy(s_lines[i]);
  }
}

static void prv_tick(struct tm *now) {
  char time_text[16];
  studio_format(now, time_text, sizeof(time_text), NULL, 0);
  snprintf(s_time_line, sizeof(s_time_line), "0100 %s", time_text);

  char date[16];
  strftime(date, sizeof(date), "%a %b%d", now);
  studio_upper(date);
  snprintf(s_date_line, sizeof(s_date_line), "0104 %s", date);

  snprintf(s_battery_line, sizeof(s_battery_line), "0108 BAT %03d",
           battery_state_service_peek().charge_percent);

  text_layer_set_text(s_lines[1], s_time_line);
  text_layer_set_text(s_lines[2], s_date_line);
  text_layer_set_text(s_lines[3], s_battery_line);
}

STUDIO_VARIANT("monitor", prv_load, prv_unload, prv_tick)
