// scorecard — the hour and the minute as the OUT and IN halves of a card.
//
// Axes: time-display=digital, composition=split, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// The notational reading of the brief rather than the pictorial one: no turf, no
// flag, just the ruled grid every golfer has held. The split is inherited rather
// than imposed — a card has always been printed as a front nine and a back nine
// — and the question is whether that borrowed structure is legible as a clock,
// or whether two boxed numbers with no colon between them stop reading as a time.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define ACCENT GColorInchworm
#define BOX_Y 38
#define BOX_H 76
#define LEFT_X 12
#define RIGHT_X 104
#define BOX_W 84

static Layer *s_rules_layer;
static TextLayer *s_out_label;
static TextLayer *s_in_label;
static TextLayer *s_hour_layer;
static TextLayer *s_minute_layer;
static TextLayer *s_date_layer;
static TextLayer *s_par_layer;
static Layer *s_battery_layer;

static char s_hour_text[8];
static char s_minute_text[8];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_rules(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_rect(ctx, GRect(LEFT_X, BOX_Y, BOX_W, BOX_H));
  graphics_draw_rect(ctx, GRect(RIGHT_X, BOX_Y, BOX_W, BOX_H));

  graphics_context_set_stroke_color(ctx, ACCENT);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_rect(ctx, GRect(LEFT_X, 126, 176, 40));
}

static void prv_draw_battery(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent, ACCENT);
}

static TextLayer *prv_digits(Layer *parent, int x) {
  return studio_text_layer(parent, GRect(x, BOX_Y + 10, BOX_W, 58),
                           studio_font(RESOURCE_ID_FONT_SHARETECH_46),
                           GColorWhite, GTextAlignmentCenter);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_rules_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_rules_layer, prv_draw_rules);
  layer_add_child(window_layer, s_rules_layer);

  s_out_label = studio_text_layer(window_layer, GRect(LEFT_X, 14, BOX_W, 20),
                                  studio_font(RESOURCE_ID_FONT_MONO_14), ACCENT,
                                  GTextAlignmentCenter);
  text_layer_set_text(s_out_label, "OUT");

  s_in_label = studio_text_layer(window_layer, GRect(RIGHT_X, 14, BOX_W, 20),
                                 studio_font(RESOURCE_ID_FONT_MONO_14), ACCENT,
                                 GTextAlignmentCenter);
  text_layer_set_text(s_in_label, "IN");

  s_hour_layer = prv_digits(window_layer, LEFT_X);
  s_minute_layer = prv_digits(window_layer, RIGHT_X);

  s_date_layer = studio_text_layer(window_layer, GRect(LEFT_X, 132, 176, 30),
                                   studio_font(RESOURCE_ID_FONT_MONO_24),
                                   GColorWhite, GTextAlignmentCenter);

  s_par_layer = studio_text_layer(window_layer, GRect(LEFT_X, 180, 90, 20),
                                  studio_font(RESOURCE_ID_FONT_MONO_14), ACCENT,
                                  GTextAlignmentLeft);
  text_layer_set_text(s_par_layer, "PAR 72");

  s_battery_layer = layer_create(GRect(150, 182, 38, 15));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_par_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_minute_layer);
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_in_label);
  text_layer_destroy(s_out_label);
  layer_destroy(s_rules_layer);
}

static void prv_tick(struct tm *now) {
  snprintf(s_hour_text, sizeof(s_hour_text), "%02d", now->tm_hour);
  snprintf(s_minute_text, sizeof(s_minute_text), "%02d", now->tm_min);

  strftime(s_date_text, sizeof(s_date_text), "%a %b %d", now);
  studio_upper(s_date_text);

  s_battery_percent = battery_state_service_peek().charge_percent;

  text_layer_set_text(s_hour_layer, s_hour_text);
  text_layer_set_text(s_minute_layer, s_minute_text);
  text_layer_set_text(s_date_layer, s_date_text);
  layer_mark_dirty(s_battery_layer);
}

STUDIO_VARIANT("scorecard", prv_load, prv_unload, prv_tick)
