// listing — four-column disassembly: address, opcode bytes, mnemonic, operand.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// The most literal reading of the brief. Every value the face carries is an
// operand of a real Z80 instruction — 3E is LD A,n and C9 is RET — so the time
// is not displayed so much as loaded into the accumulator. The question is
// whether that is charming or whether burying the time in column four costs more
// legibility than the look is worth; this is the tile that finds out.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define MARGIN 12
#define PHOSPHOR GColorScreaminGreen
#define TOP 22
#define PITCH 21
#define LINE_HEIGHT 21
#define LINE_COUNT 6

static TextLayer *s_lines[LINE_COUNT];
static char s_time_line[24];
static char s_battery_line[24];
static char s_weekday_line[24];
static char s_date_line[24];

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  for (unsigned i = 0; i < LINE_COUNT; i++) {
    s_lines[i] = studio_text_layer(
        window_layer,
        GRect(MARGIN, TOP + (i * PITCH), 200 - (MARGIN * 2), LINE_HEIGHT),
        studio_font(RESOURCE_ID_FONT_MONO_14), PHOSPHOR, GTextAlignmentLeft);
  }

  text_layer_set_text(s_lines[0], "; DISASSEMBLY");
  text_layer_set_text(s_lines[5], "010C C9 RET");
}

static void prv_unload(Window *window) {
  for (unsigned i = 0; i < LINE_COUNT; i++) {
    text_layer_destroy(s_lines[i]);
  }
}

static void prv_tick(struct tm *now) {
  char time_text[16];
  studio_format(now, time_text, sizeof(time_text), NULL, 0);
  snprintf(s_time_line, sizeof(s_time_line), "0100 3E LD A,%s", time_text);

  snprintf(s_battery_line, sizeof(s_battery_line), "0103 06 LD B,%03d",
           battery_state_service_peek().charge_percent);

  char weekday[8];
  strftime(weekday, sizeof(weekday), "%a", now);
  studio_upper(weekday);
  snprintf(s_weekday_line, sizeof(s_weekday_line), "0105 21 LD HL,%s", weekday);

  char date[12];
  strftime(date, sizeof(date), "%b%d", now);
  studio_upper(date);
  snprintf(s_date_line, sizeof(s_date_line), "0108 11 LD DE,%s", date);

  text_layer_set_text(s_lines[1], s_time_line);
  text_layer_set_text(s_lines[2], s_battery_line);
  text_layer_set_text(s_lines[3], s_weekday_line);
  text_layer_set_text(s_lines[4], s_date_line);
}

STUDIO_VARIANT("listing", prv_load, prv_unload, prv_tick)
