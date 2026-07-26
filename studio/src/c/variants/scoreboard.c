// scoreboard — the hour and the minute as the two teams' score.
//
// Axes: time-display=digital, composition=split, complications=3-4,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The game's own scoreboard is two numbers in two team plates with the ball
// between them, and a watch already has two numbers that want separating. So the
// time is not shown on a scoreboard here — it *is* the scoreboard, which is the
// one idea in this Batch that could not be transplanted to another theme. The
// bottom half is the match HUD, carrying everything else.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define PLATE_Y 14
#define PLATE_H 66
#define HUD_TOP 96
#define ROW_H 22

static Layer *s_plate_layer;
static Layer *s_boost_layer;
static Layer *s_car_layer;
static TextLayer *s_hour_layer;
static TextLayer *s_minute_layer;
static TextLayer *s_date_layer;
static TextLayer *s_week_layer;
static TextLayer *s_yearday_layer;
static TextLayer *s_boost_layer_text;

static char s_hour_text[8];
static char s_minute_text[8];
static char s_date_text[16];
static char s_week_text[16];
static char s_yearday_text[16];
static char s_boost_text[16];
static int s_battery_percent;

static void prv_draw_plate(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, ROCKET_BLUE_DEEP);
  graphics_fill_rect(ctx, GRect(8, PLATE_Y, 78, PLATE_H), 4, GCornersAll);
  graphics_context_set_fill_color(ctx, ROCKET_BLUE);
  graphics_fill_rect(ctx, GRect(8, PLATE_Y, 78, 5), 0, GCornerNone);

  graphics_context_set_fill_color(ctx, ROCKET_ORANGE_DEEP);
  graphics_fill_rect(ctx, GRect(114, PLATE_Y, 78, PLATE_H), 4, GCornersAll);
  graphics_context_set_fill_color(ctx, ROCKET_ORANGE);
  graphics_fill_rect(ctx, GRect(114, PLATE_Y, 78, 5), 0, GCornerNone);

  rocket_ball_draw(ctx, GPoint(100, PLATE_Y + (PLATE_H / 2)), 13);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(8, HUD_TOP - 8, 184, 2), 0, GCornerNone);
}

// The boost meter as the game draws it: a segmented bar rather than a
// continuous one, because a segment count is readable at a glance and a bar
// length is not.
static void prv_draw_boost(Layer *layer, GContext *ctx) {
  const int segments = 12;
  int lit = (segments * s_battery_percent + 99) / 100;

  for (int i = 0; i < segments; i++) {
    graphics_context_set_fill_color(ctx, (i < lit) ? ROCKET_BOOST
                                                   : GColorOxfordBlue);
    graphics_fill_rect(ctx, GRect(i * 9, 0, 7, 12), 0, GCornerNone);
  }
}

static void prv_draw_car(Layer *layer, GContext *ctx) {
  rocket_floor_draw(ctx, 10, 44);
  rocket_octane_draw(ctx, GRect(116, 8, 68, 32), ROCKET_BLUE, true);
}

static TextLayer *prv_row(Layer *parent, int row, int x, int width,
                          GTextAlignment alignment) {
  return studio_text_layer(parent, GRect(x, HUD_TOP + (row * ROW_H), width,
                                         ROW_H),
                           studio_font(RESOURCE_ID_FONT_MONO_18), GColorWhite,
                           alignment);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_plate_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_plate_layer, prv_draw_plate);
  layer_add_child(window_layer, s_plate_layer);

  s_hour_layer = studio_text_layer(window_layer, GRect(8, PLATE_Y + 8, 78, 54),
                                   studio_font(RESOURCE_ID_FONT_SHARETECH_46),
                                   GColorWhite, GTextAlignmentCenter);
  s_minute_layer = studio_text_layer(window_layer,
                                     GRect(114, PLATE_Y + 8, 78, 54),
                                     studio_font(RESOURCE_ID_FONT_SHARETECH_46),
                                     GColorWhite, GTextAlignmentCenter);

  s_date_layer = prv_row(window_layer, 0, 8, 184, GTextAlignmentLeft);
  s_week_layer = prv_row(window_layer, 1, 8, 90, GTextAlignmentLeft);
  s_yearday_layer = prv_row(window_layer, 1, 102, 90, GTextAlignmentRight);

  s_boost_layer = layer_create(GRect(8, HUD_TOP + (2 * ROW_H) + 4, 108, 12));
  layer_set_update_proc(s_boost_layer, prv_draw_boost);
  layer_add_child(window_layer, s_boost_layer);
  s_boost_layer_text = prv_row(window_layer, 2, 122, 70, GTextAlignmentRight);

  s_car_layer = layer_create(GRect(0, 176, 200, 44));
  layer_set_update_proc(s_car_layer, prv_draw_car);
  layer_add_child(window_layer, s_car_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_car_layer);
  text_layer_destroy(s_boost_layer_text);
  layer_destroy(s_boost_layer);
  text_layer_destroy(s_yearday_layer);
  text_layer_destroy(s_week_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_minute_layer);
  text_layer_destroy(s_hour_layer);
  layer_destroy(s_plate_layer);
}

static void prv_tick(struct tm *now) {
  char week[8];

  studio_format(now, NULL, 0, s_date_text, sizeof(s_date_text));
  studio_upper(s_date_text);

  // The plates carry the hour and the minute apart, so the time is formatted in
  // two pieces here rather than split back out of one string.
  if (clock_is_24h_style()) {
    snprintf(s_hour_text, sizeof(s_hour_text), "%02d", now->tm_hour);
  } else {
    int hour = now->tm_hour % 12;
    snprintf(s_hour_text, sizeof(s_hour_text), "%02d", hour ? hour : 12);
  }
  snprintf(s_minute_text, sizeof(s_minute_text), "%02d", now->tm_min);

  s_battery_percent = battery_state_service_peek().charge_percent;
  snprintf(s_boost_text, sizeof(s_boost_text), "%d%%", s_battery_percent);
  strftime(week, sizeof(week), "%V", now);
  snprintf(s_week_text, sizeof(s_week_text), "WK %s", week);
  snprintf(s_yearday_text, sizeof(s_yearday_text), "DAY %d", now->tm_yday + 1);

  text_layer_set_text(s_hour_layer, s_hour_text);
  text_layer_set_text(s_minute_layer, s_minute_text);
  text_layer_set_text(s_date_layer, s_date_text);
  text_layer_set_text(s_week_layer, s_week_text);
  text_layer_set_text(s_yearday_layer, s_yearday_text);
  text_layer_set_text(s_boost_layer_text, s_boost_text);
  layer_mark_dirty(s_boost_layer);
}

STUDIO_VARIANT("scoreboard", prv_load, prv_unload, prv_tick)
