// registers — a CPU register dump in two columns. The split composition.
//
// Axes: time-display=digital, composition=split, complications=3-4,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// The one arrangement in the Batch that is not a listing flowing down the left
// margin, and the split is inherited rather than imposed: a register dump has
// always been printed in banks side by side. It also tests whether the split
// survives without a rule between the halves, which the brief rules out — the
// gutter has to do the whole job on its own.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define PHOSPHOR GColorScreaminGreen
#define LEFT_X 12
#define RIGHT_X 104
#define HALF_W 88
#define HEADER_Y 26
#define TOP 52
#define PITCH 32
#define ROW_HEIGHT 26
#define ROW_COUNT 3

static TextLayer *s_left_header;
static TextLayer *s_right_header;
static TextLayer *s_left[ROW_COUNT];
static TextLayer *s_right[ROW_COUNT];

static char s_accumulator[16];
static char s_battery[16];
static char s_weekday[16];
static char s_date[16];
static char s_week[16];

static TextLayer *prv_header(Layer *parent, int x) {
  return studio_text_layer(parent, GRect(x, HEADER_Y, HALF_W, 20),
                           studio_font(RESOURCE_ID_FONT_MONO_14), PHOSPHOR,
                           GTextAlignmentLeft);
}

static TextLayer *prv_row(Layer *parent, int x, int row) {
  return studio_text_layer(
      parent, GRect(x, TOP + (row * PITCH), HALF_W, ROW_HEIGHT),
      studio_font(RESOURCE_ID_FONT_MONO_18), PHOSPHOR, GTextAlignmentLeft);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_left_header = prv_header(window_layer, LEFT_X);
  s_right_header = prv_header(window_layer, RIGHT_X);
  text_layer_set_text(s_left_header, "; REGS");
  text_layer_set_text(s_right_header, "; MEM");

  for (unsigned i = 0; i < ROW_COUNT; i++) {
    s_left[i] = prv_row(window_layer, LEFT_X, i);
    s_right[i] = prv_row(window_layer, RIGHT_X, i);
  }

  text_layer_set_text(s_left[2], "SP 0100");
}

static void prv_unload(Window *window) {
  for (unsigned i = 0; i < ROW_COUNT; i++) {
    text_layer_destroy(s_right[i]);
    text_layer_destroy(s_left[i]);
  }
  text_layer_destroy(s_right_header);
  text_layer_destroy(s_left_header);
}

static void prv_tick(struct tm *now) {
  char time_text[16];
  studio_format(now, time_text, sizeof(time_text), NULL, 0);
  snprintf(s_accumulator, sizeof(s_accumulator), "AF %s", time_text);
  snprintf(s_battery, sizeof(s_battery), "BC %03d",
           battery_state_service_peek().charge_percent);

  strftime(s_weekday, sizeof(s_weekday), "%a", now);
  studio_upper(s_weekday);
  strftime(s_date, sizeof(s_date), "%b %d", now);
  studio_upper(s_date);

  char week[8];
  strftime(week, sizeof(week), "%V", now);
  snprintf(s_week, sizeof(s_week), "WK %s", week);

  text_layer_set_text(s_left[0], s_accumulator);
  text_layer_set_text(s_left[1], s_battery);
  text_layer_set_text(s_right[0], s_weekday);
  text_layer_set_text(s_right[1], s_date);
  text_layer_set_text(s_right[2], s_week);
}

STUDIO_VARIANT("registers", prv_load, prv_unload, prv_tick)
