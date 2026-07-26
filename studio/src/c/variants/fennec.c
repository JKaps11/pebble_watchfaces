// fennec — the plain aerial, and the Batch's control.
//
// Axes: time-display=digital, composition=split, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The car is doing one simple thing — nose up, boosting, off the floor — and the
// type keeps its own half. Everything else in this Batch adds rotation, so this
// tile exists to answer the question underneath all of them first: at this size,
// does a Fennec read as a car in the air rather than as a brick at an angle?

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 122
#define CLIMB (-TRIG_MAX_ANGLE * 35 / 360)

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_floor_draw(ctx, HORIZON, 228);

  rocket_ball_draw(ctx, GPoint(166, 100), 13);

  // The shadow stays on the floor while the car is not on it, which is most of
  // what says the car is airborne rather than parked at a jaunty angle.
  studio_fill_ellipse(ctx, GPoint(100, 214), 32, 4, GColorBlack);
  rocket_fennec_draw(ctx, GPoint(100, 150), 100, CLIMB, ROCKET_BLUE, true);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(8, 4, 184, 54),
                                   studio_font(RESOURCE_ID_FONT_ARCHIVO_44),
                                   GColorWhite, GTextAlignmentCenter);

  s_date_layer = studio_text_layer(
      window_layer, GRect(8, 58, 184, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), ROCKET_BOOST,
      GTextAlignmentCenter);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  layer_destroy(s_scene_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), s_date_text,
                sizeof(s_date_text));
  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
}

STUDIO_VARIANT("fennec", prv_load, prv_unload, prv_tick)
