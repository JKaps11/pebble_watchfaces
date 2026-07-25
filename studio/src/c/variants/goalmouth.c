// goalmouth — fennec's layout, turned to face down the pitch.
//
// Axes: time-display=digital, composition=split, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The smallest possible change to the tile this Session picked: same split, same
// face, same Archivo. Only the camera has moved, from beside the car to behind
// it, and the background has something to look at — walls falling away, a floor
// running back to the viewer, a goal on the horizon. If this is enough, the
// problem with fennec's background was that a side view has nothing in it to
// converge.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 152
#define BANK (-TRIG_MAX_ANGLE * 6 / 360)

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_downfield_draw(ctx, HORIZON, ROCKET_FLOOR, GColorLiberty,
                        ROCKET_BLUE_DEEP);

  // The goal is deliberately smaller than the car. That is the whole depth cue:
  // two objects of known size, the nearer one bigger.
  rocket_goal_draw(ctx, GRect(58, 104, 84, 48), ROCKET_ORANGE);
  rocket_ball_draw(ctx, GPoint(100, 128), 11);

  rocket_fennec_rear(ctx, GPoint(100, 182), 100, BANK, 0, ROCKET_BLUE,
                     ROCKET_ORANGE, true, false);
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

STUDIO_VARIANT("goalmouth", prv_load, prv_unload, prv_tick)
