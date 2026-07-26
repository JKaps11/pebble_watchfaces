// liftoff — nose to the sky, climbing at the goal.
//
// Axes: time-display=digital, composition=split, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// This is goalmouth's scene with the car swapped from the rear view to the
// profile one, turned steeply nose-up. That swap is the finding. A rear view
// carries no pitch at all: pointing the nose at the sky changes nothing about a
// shape seen end-on, and foreshortening it to compensate reads as a car skimming
// the floor rather than leaving it. The profile car turned sixty degrees says it
// in one shape, at the price of no longer facing down the pitch.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 152
#define CLIMB (-TRIG_MAX_ANGLE * 62 / 360)

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_downfield_draw(ctx, HORIZON, ROCKET_FLOOR, GColorLiberty,
                        ROCKET_BLUE_DEEP);

  rocket_goal_draw(ctx, GRect(58, 104, 84, 48), ROCKET_ORANGE);
  rocket_ball_draw(ctx, GPoint(100, 126), 11);

  // The shadow stays on the floor, well below and behind the car, and it is the
  // only thing in the frame saying the two are no longer in the same place.
  studio_fill_ellipse(ctx, GPoint(104, 214), 28, 4, GColorBlack);
  rocket_fennec_draw(ctx, GPoint(106, 158), 94, CLIMB, ROCKET_BLUE, true);
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

STUDIO_VARIANT("liftoff", prv_load, prv_unload, prv_tick)
