// stands — the same shot with a stadium built around it.
//
// Axes: time-display=digital, composition=split, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// goalmouth's arena is empty, which is a fair description of a training pack and
// not of a match. Terraces above the far wall and two floodlight rigs on the
// skyline are the cheapest things that fix that, and the horizon has to drop to
// make room for them — so this tile is also the test of how little floor the
// scene can stand before the car runs out of ground.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 182
#define BANK (-TRIG_MAX_ANGLE * 5 / 360)

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_downfield_draw(ctx, HORIZON, ROCKET_FLOOR, GColorLiberty,
                        ROCKET_BLUE_DEEP);
  rocket_stands_draw(ctx, HORIZON);

  rocket_goal_draw(ctx, GRect(64, 142, 72, 40), ROCKET_ORANGE);
  rocket_ball_draw(ctx, GPoint(100, 160), 9);

  rocket_fennec_rear(ctx, GPoint(100, 190), 82, BANK, 0, ROCKET_BLUE,
                     ROCKET_ORANGE, true, false);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(8, 2, 184, 54),
                                   studio_font(RESOURCE_ID_FONT_BARLOW_52),
                                   GColorWhite, GTextAlignmentCenter);

  s_date_layer = studio_text_layer(
      window_layer, GRect(8, 54, 184, 24),
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

STUDIO_VARIANT("stands", prv_load, prv_unload, prv_tick)
