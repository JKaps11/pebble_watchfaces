// airroll — the car's rotation is the minute hand.
//
// Axes: time-display=hybrid, composition=centred, complications=1-2,
//       hue=multi, type=Bitham, polarity=light-on-dark
//
// The one idea in this Batch that only exists because the car can now be turned:
// a Fennec air-rolling once an hour, nose always on the minute. It is a real
// hybrid rather than a decorated digital — the hour is a numeral and the minute
// is genuinely read off an angle — and it is the only tile where the spin is
// information instead of an illustration of one.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../variant.h"

#define DIAL_X 100
#define DIAL_Y 152
#define DIAL_R 46
#define CAR 68

static Layer *s_dial_layer;
static TextLayer *s_hour_layer;
static TextLayer *s_date_layer;

static char s_hour_text[8];
static char s_date_text[16];
static int s_minute;

static void prv_draw_dial(Layer *layer, GContext *ctx) {
  GPoint centre = GPoint(DIAL_X, DIAL_Y);
  int32_t minute = studio_minute_angle(s_minute);

  // Twelve marks, the quarters as boost pads and the rest as plain ticks. The
  // pads are there because an angle needs somewhere to be read against, and the
  // theme already owns a diamond that does that job.
  for (int i = 0; i < 12; i++) {
    int32_t at = TRIG_MAX_ANGLE * i / 12;
    GPoint out = studio_point_at(centre, at, DIAL_R + 12);

    if (i % 3 == 0) {
      rocket_boost_pad(ctx, out, 7);
    } else {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx, GRect(out.x - 2, out.y - 2, 4, 4), 0,
                         GCornerNone);
    }
  }

  // The car's nose sits on the minute. Its own zero is level and facing right,
  // which is three o'clock, so the dial angle is turned back a quarter to meet
  // it — get this wrong and the watch is fifteen minutes fast.
  rocket_fennec_draw(ctx, centre, CAR, minute - (TRIG_MAX_ANGLE / 4),
                     ROCKET_BLUE, true);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_hour_layer = studio_text_layer(
      window_layer, GRect(8, 8, 184, 52),
      fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS), GColorWhite,
      GTextAlignmentCenter);

  s_dial_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_dial_layer, prv_draw_dial);
  layer_add_child(window_layer, s_dial_layer);

  s_date_layer = studio_text_layer(
      window_layer, GRect(8, 60, 184, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), ROCKET_BOOST,
      GTextAlignmentCenter);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  layer_destroy(s_dial_layer);
  text_layer_destroy(s_hour_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, NULL, 0, s_date_text, sizeof(s_date_text));
  s_minute = now->tm_min;

  if (clock_is_24h_style()) {
    snprintf(s_hour_text, sizeof(s_hour_text), "%02d", now->tm_hour);
  } else {
    int hour = now->tm_hour % 12;
    snprintf(s_hour_text, sizeof(s_hour_text), "%d", hour ? hour : 12);
  }

  text_layer_set_text(s_hour_layer, s_hour_text);
  text_layer_set_text(s_date_layer, s_date_text);
  layer_mark_dirty(s_dial_layer);
}

STUDIO_VARIANT("airroll", prv_load, prv_unload, prv_tick)
