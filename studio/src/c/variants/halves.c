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
#include "../studio_draw.h"
#include "../variant.h"

#define ACCENT GColorPictonBlue
#define SPLIT_Y 116

static Layer *s_dial_layer;
static TextLayer *s_time_layer;
static char s_time_text[16];
static int s_hour;
static int s_minute;

static void prv_draw_dial(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint centre = grect_center_point(&bounds);
  int radius = 40;

  graphics_context_set_stroke_color(ctx, ACCENT);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, centre, radius);

  int32_t hour_angle = studio_hour_angle(s_hour, s_minute);
  int32_t minute_angle = studio_minute_angle(s_minute);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 5);
  graphics_draw_line(ctx, centre, studio_point_at(centre, hour_angle, radius - 20));
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, centre,
                     studio_point_at(centre, minute_angle, radius - 7));
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);

  s_dial_layer = layer_create(GRect(0, 0, bounds.size.w, SPLIT_Y));
  layer_set_update_proc(s_dial_layer, prv_draw_dial);
  layer_add_child(window_layer, s_dial_layer);

  s_time_layer = studio_text_layer(
      window_layer, GRect(0, SPLIT_Y + 22, bounds.size.w, 52),
      fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS),
      GColorWhite, GTextAlignmentCenter);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  layer_destroy(s_dial_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), NULL, 0);
  text_layer_set_text(s_time_layer, s_time_text);

  s_hour = now->tm_hour;
  s_minute = now->tm_min;
  layer_mark_dirty(s_dial_layer);
}

STUDIO_VARIANT("halves", prv_load, prv_unload, prv_tick)
