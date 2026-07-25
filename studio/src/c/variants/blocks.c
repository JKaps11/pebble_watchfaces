// blocks — the battery as a row of filled character cells, the way a TTY would.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// The only battery treatment in the Sweep that is native to the idiom rather
// than borrowed from a phone status bar. A terminal with no graphics draws a
// meter out of the characters it has, and the quantisation is honest — ten cells
// is exactly the precision anyone acts on. Whether it reads as a battery at all
// without the familiar cell outline is the question.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define MARGIN 14
#define PHOSPHOR GColorScreaminGreen
#define RULE_Y 106
#define CELL_COUNT 10
#define CELL_WIDTH 12
#define CELL_GAP 4
#define CELL_HEIGHT 18

static Layer *s_rule_layer;
static Layer *s_meter_layer;
static TextLayer *s_caret_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_rule(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, PHOSPHOR);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, 2), 0, GCornerNone);
}

// Cells round up, so any charge at all lights the first one — an empty row would
// mean a dead watch, which is not what 4% is telling you.
static void prv_draw_meter(Layer *layer, GContext *ctx) {
  int lit = (s_battery_percent * CELL_COUNT + 99) / 100;

  for (int i = 0; i < CELL_COUNT; i++) {
    GRect cell = GRect(i * (CELL_WIDTH + CELL_GAP), 0, CELL_WIDTH, CELL_HEIGHT);
    if (i < lit) {
      graphics_context_set_fill_color(ctx, PHOSPHOR);
      graphics_fill_rect(ctx, cell, 0, GCornerNone);
    } else {
      graphics_context_set_stroke_color(ctx, PHOSPHOR);
      graphics_context_set_stroke_width(ctx, 1);
      graphics_draw_rect(ctx, cell);
    }
  }
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_caret_layer = studio_text_layer(
      window_layer, GRect(MARGIN, 16, 172, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), PHOSPHOR,
      GTextAlignmentLeft);
  text_layer_set_text(s_caret_layer, ">");

  s_time_layer = studio_text_layer(
      window_layer, GRect(MARGIN, 42, 186 - MARGIN, 52),
      studio_font(RESOURCE_ID_FONT_SILKSCREEN_38), PHOSPHOR,
      GTextAlignmentLeft);

  s_rule_layer = layer_create(GRect(MARGIN, RULE_Y, 200 - (MARGIN * 2), 2));
  layer_set_update_proc(s_rule_layer, prv_draw_rule);
  layer_add_child(window_layer, s_rule_layer);

  s_date_layer = studio_text_layer(
      window_layer, GRect(MARGIN, 116, 172, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18), PHOSPHOR, GTextAlignmentLeft);

  s_meter_layer = layer_create(
      GRect(MARGIN, 148, CELL_COUNT * (CELL_WIDTH + CELL_GAP), CELL_HEIGHT));
  layer_set_update_proc(s_meter_layer, prv_draw_meter);
  layer_add_child(window_layer, s_meter_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_meter_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_rule_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_caret_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text),
                s_date_text, sizeof(s_date_text));

  s_battery_percent = battery_state_service_peek().charge_percent;

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  layer_mark_dirty(s_meter_layer);
}

STUDIO_VARIANT("blocks", prv_load, prv_unload, prv_tick)
