// symtab — the assembler's symbol table, every field an EQU. The 5+ end.
//
// Axes: time-display=digital, composition=corner-anchored, complications=5+,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// A symbol table is the one place in a listing where density is native: it is a
// column of names against values and it is *supposed* to be long. So this is the
// fairest test of whether the assembly framing buys back the crowding that sank
// `readout` in Batch 2 — same six fields, but arranged as something that has a
// reason to be a list.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define MARGIN 12
#define PHOSPHOR GColorScreaminGreen
#define TOP 22
#define PITCH 20
#define LINE_HEIGHT 20
#define LINE_COUNT 7

static TextLayer *s_lines[LINE_COUNT];
static char s_time_line[24];
static char s_date_line[24];
static char s_battery_line[24];
static char s_week_line[24];
static char s_yearday_line[24];

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  for (unsigned i = 0; i < LINE_COUNT; i++) {
    s_lines[i] = studio_text_layer(
        window_layer,
        GRect(MARGIN, TOP + (i * PITCH), 200 - (MARGIN * 2), LINE_HEIGHT),
        studio_font(RESOURCE_ID_FONT_MONO_14), PHOSPHOR, GTextAlignmentLeft);
  }

  text_layer_set_text(s_lines[0], "; SYMBOL TABLE");
}

static void prv_unload(Window *window) {
  for (unsigned i = 0; i < LINE_COUNT; i++) {
    text_layer_destroy(s_lines[i]);
  }
}

static void prv_tick(struct tm *now) {
  char time_text[16];
  studio_format(now, time_text, sizeof(time_text), NULL, 0);
  snprintf(s_time_line, sizeof(s_time_line), "TIME EQU %s", time_text);

  char date[16];
  strftime(date, sizeof(date), "%a %b%d", now);
  studio_upper(date);
  snprintf(s_date_line, sizeof(s_date_line), "DATE EQU %s", date);

  BatteryChargeState battery = battery_state_service_peek();
  snprintf(s_battery_line, sizeof(s_battery_line), "BATT EQU %03d",
           battery.charge_percent);

  char week[8];
  strftime(week, sizeof(week), "%V", now);
  snprintf(s_week_line, sizeof(s_week_line), "WEEK EQU %s", week);
  snprintf(s_yearday_line, sizeof(s_yearday_line), "YDAY EQU %03d",
           now->tm_yday + 1);

  text_layer_set_text(s_lines[1], s_time_line);
  text_layer_set_text(s_lines[2], s_date_line);
  text_layer_set_text(s_lines[3], s_battery_line);
  text_layer_set_text(s_lines[4], s_week_line);
  text_layer_set_text(s_lines[5], s_yearday_line);
  text_layer_set_text(s_lines[6],
                      battery.is_charging ? "PWR  EQU EXT" : "PWR  EQU BAT");
}

STUDIO_VARIANT("symtab", prv_load, prv_unload, prv_tick)
