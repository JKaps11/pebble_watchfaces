// halves — a small dial above, the same time in figures below.
//
// Axes: time-display=hybrid, composition=split, complications=1-2,
//       hue=one-hue, type=Bitham, polarity=light-on-dark
//
// The hybrid position on the time-display axis, which was 13% of the surveyed
// corpus and is the one a designer is least likely to picture without seeing.
// Its real question is whether showing the time twice reads as considered or as
// indecisive — which no measurement answers, so it needs to be looked at.

#include <pebble.h>
#include "../variant.h"
#include "datetime_format.h"

#define ACCENT GColorPictonBlue
#define SPLIT_Y 116

static Layer *s_dial_layer;
static TextLayer *s_time_layer;
static char s_time_text[16];
static int s_hour;
static int s_minute;

static GPoint prv_point_at(GPoint centre, int32_t angle, int length) {
  return GPoint(
      centre.x + (int16_t)(sin_lookup(angle) * length / TRIG_MAX_RATIO),
      centre.y - (int16_t)(cos_lookup(angle) * length / TRIG_MAX_RATIO));
}

static void prv_draw_dial(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint centre = grect_center_point(&bounds);
  int radius = 40;

  graphics_context_set_stroke_color(ctx, ACCENT);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, centre, radius);

  int32_t hour_angle =
      TRIG_MAX_ANGLE * ((s_hour % 12) * 60 + s_minute) / (12 * 60);
  int32_t minute_angle = TRIG_MAX_ANGLE * s_minute / 60;

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 5);
  graphics_draw_line(ctx, centre, prv_point_at(centre, hour_angle, radius - 20));
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, centre,
                     prv_point_at(centre, minute_angle, radius - 7));
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);

  s_dial_layer = layer_create(GRect(0, 0, bounds.size.w, SPLIT_Y));
  layer_set_update_proc(s_dial_layer, prv_draw_dial);
  layer_add_child(window_layer, s_dial_layer);

  s_time_layer = text_layer_create(
      GRect(0, SPLIT_Y + 22, bounds.size.w, 52));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer,
                      fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  layer_destroy(s_dial_layer);
}

static void prv_tick(struct tm *now) {
  DateTimeInfo info = {
    .hour = now->tm_hour,
    .minute = now->tm_min,
    .weekday = now->tm_wday,
    .month = now->tm_mon,
    .day_of_month = now->tm_mday,
  };
  datetime_format_time(&info, clock_is_24h_style(), s_time_text,
                       sizeof(s_time_text));
  text_layer_set_text(s_time_layer, s_time_text);

  s_hour = now->tm_hour;
  s_minute = now->tm_min;
  layer_mark_dirty(s_dial_layer);
}

STUDIO_VARIANT("halves", prv_load, prv_unload, prv_tick)
