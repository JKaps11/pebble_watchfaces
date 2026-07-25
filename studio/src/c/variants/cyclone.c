// cyclone — six Fennecs orbiting the time, all turning the same way.
//
// Axes: time-display=digital, composition=centred, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// tornado spends the spin along a line; this spends it around the numerals, so
// the type sits in the calm middle of it and the whole edge of the display does
// the moving. Each car is tangent to the ring and brightens the way round, which
// is the only cue giving the orbit a direction. The plate is the honest part of
// the design: cars pass behind it, and without it the time would be read across
// six of them.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define RING_X 100
#define RING_Y 114
#define RING_RX 62
#define RING_RY 88
#define CARS 6
#define CAR 38

static Layer *s_ring_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static GColor prv_shade(int i) {
  switch (i) {
    case 0: return GColorOxfordBlue;
    case 1: return GColorDukeBlue;
    case 2: return GColorLiberty;
    case 3: return GColorBlueMoon;
    case 4: return GColorPictonBlue;
    default: return ROCKET_BLUE;
  }
}

static void prv_draw_ring(Layer *layer, GContext *ctx) {
  // An ellipse rather than a circle. The display is taller than it is wide, so a
  // circular orbit would either clear the sides by a mile or run off the top.
  for (int i = 0; i < CARS; i++) {
    int32_t at = TRIG_MAX_ANGLE * i / CARS;
    GPoint on = GPoint(
        RING_X + (int16_t)(sin_lookup(at) * RING_RX / TRIG_MAX_RATIO),
        RING_Y - (int16_t)(cos_lookup(at) * RING_RY / TRIG_MAX_RATIO));

    // Tangent to the ring: the car's own zero faces right, which is exactly the
    // clockwise tangent at twelve o'clock, so the ring angle passes straight
    // through with no quarter turn.
    if (i == CARS - 1) {
      rocket_fennec_draw(ctx, on, CAR, at, ROCKET_BLUE, true);
    } else {
      rocket_fennec_ghost(ctx, on, CAR, at, prv_shade(i));
    }
  }

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(28, 78, 144, 74), 5, GCornersAll);
  graphics_context_set_stroke_color(ctx, ROCKET_BLUE);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_round_rect(ctx, GRect(28, 78, 144, 74), 5);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_ring_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_ring_layer, prv_draw_ring);
  layer_add_child(window_layer, s_ring_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(30, 76, 140, 52),
                                   studio_font(RESOURCE_ID_FONT_BARLOW_52),
                                   GColorWhite, GTextAlignmentCenter);

  s_date_layer = studio_text_layer(
      window_layer, GRect(30, 128, 140, 20),
      fonts_get_system_font(FONT_KEY_GOTHIC_14), ROCKET_BOOST,
      GTextAlignmentCenter);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  layer_destroy(s_ring_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), s_date_text,
                sizeof(s_date_text));
  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
}

STUDIO_VARIANT("cyclone", prv_load, prv_unload, prv_tick)
