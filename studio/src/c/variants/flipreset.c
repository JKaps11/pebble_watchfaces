// flipreset — a full dial, with the car for the minute and the ball for the hour.
//
// Axes: time-display=analogue, composition=centred, complications=1-2,
//       hue=multi, type=Gothic, polarity=light-on-dark
//
// The far end of what rotation buys: no numerals at all, the Fennec orbiting
// nose-out on the minute and the ball riding a stem on the hour. The Studio has
// failed an analogue Variant before by letting two hands collapse onto the same
// bearing, so the two here are put at deliberately different radii as well as
// being a car and a ball — at 23:58 they are within two degrees of each other
// and that is the state this tile has to survive.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../variant.h"

#define DIAL_X 100
#define DIAL_Y 104
#define MARK_R 86
#define MINUTE_R 58
#define HOUR_R 32
#define CAR 54

static Layer *s_dial_layer;
static TextLayer *s_date_layer;

static char s_date_text[16];
static int s_hour;
static int s_minute;

static void prv_draw_dial(Layer *layer, GContext *ctx) {
  GPoint centre = GPoint(DIAL_X, DIAL_Y);
  int32_t minute = studio_minute_angle(s_minute);
  int32_t hour = studio_hour_angle(s_hour, s_minute);
  GPoint ball = studio_point_at(centre, hour, HOUR_R);

  for (int i = 0; i < 12; i++) {
    GPoint out = studio_point_at(centre, TRIG_MAX_ANGLE * i / 12, MARK_R);
    rocket_boost_pad(ctx, out, (i % 3 == 0) ? 8 : 5);
  }

  // The hour first and underneath, on a stem, so that when the two bearings
  // coincide the car passes over the ball rather than the ball over the car.
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx, centre, ball);
  rocket_ball_draw(ctx, ball, 14);

  rocket_fennec_draw(ctx, studio_point_at(centre, minute, MINUTE_R), CAR,
                     minute - (TRIG_MAX_ANGLE / 4), ROCKET_BLUE, true);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_dial_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_dial_layer, prv_draw_dial);
  layer_add_child(window_layer, s_dial_layer);

  s_date_layer = studio_text_layer(
      window_layer, GRect(8, 202, 184, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), ROCKET_BOOST,
      GTextAlignmentCenter);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  layer_destroy(s_dial_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, NULL, 0, s_date_text, sizeof(s_date_text));
  s_hour = now->tm_hour;
  s_minute = now->tm_min;

  text_layer_set_text(s_date_layer, s_date_text);
  layer_mark_dirty(s_dial_layer);
}

STUDIO_VARIANT("flipreset", prv_load, prv_unload, prv_tick)
