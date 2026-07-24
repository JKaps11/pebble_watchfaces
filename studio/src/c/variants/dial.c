// dial — hands only, no numerals.
//
// Axes: time-display=analogue, composition=centred, complications=0,
//       hue=mono, type=Gothic, polarity=dark-on-light
//
// The far end of the time-display axis from every digital Variant, and the
// reason the Canonical state is 10:09 rather than something tidier: at 10:09 the
// hands sit open and balanced instead of overlapping into a single stroke.
//
// The type axis is declared Gothic because nothing here is set in type at all;
// an analogue face carries its character in geometry, which is the point of
// having it in a Batch.

#include <pebble.h>
#include "../variant.h"

static Layer *s_dial_layer;
static int s_hour;
static int s_minute;

static GPoint prv_point_at(GPoint centre, int32_t angle, int length) {
  return GPoint(
      centre.x + (int16_t)(sin_lookup(angle) * length / TRIG_MAX_RATIO),
      centre.y - (int16_t)(cos_lookup(angle) * length / TRIG_MAX_RATIO));
}

static void prv_update(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint centre = grect_center_point(&bounds);
  int shorter = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
  int radius = shorter / 2 - 12;

  graphics_context_set_stroke_color(ctx, GColorBlack);

  graphics_context_set_stroke_width(ctx, 2);
  for (int hour = 0; hour < 12; hour++) {
    int32_t angle = TRIG_MAX_ANGLE * hour / 12;
    int inset = (hour % 3 == 0) ? 12 : 6;
    graphics_draw_line(ctx, prv_point_at(centre, angle, radius - inset),
                       prv_point_at(centre, angle, radius));
  }

  int32_t hour_angle =
      TRIG_MAX_ANGLE * ((s_hour % 12) * 60 + s_minute) / (12 * 60);
  int32_t minute_angle = TRIG_MAX_ANGLE * s_minute / 60;

  graphics_context_set_stroke_width(ctx, 6);
  graphics_draw_line(ctx, centre, prv_point_at(centre, hour_angle, radius - 42));
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, centre,
                     prv_point_at(centre, minute_angle, radius - 16));

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, centre, 4);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorWhite);
  s_dial_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_dial_layer, prv_update);
  layer_add_child(window_layer, s_dial_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_dial_layer);
}

static void prv_tick(struct tm *now) {
  s_hour = now->tm_hour;
  s_minute = now->tm_min;
  layer_mark_dirty(s_dial_layer);
}

STUDIO_VARIANT("dial", prv_load, prv_unload, prv_tick)
