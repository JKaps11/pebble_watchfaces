// octane — the car as the subject, the time as a headline above it.
//
// Axes: time-display=digital, composition=split, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The most literal reading of the brief: put the Octane on the display at a size
// where the spoiler and the wedge nose are unmistakable, and give the type its
// own half so nothing is read across the arena. If a Rocket League watchface can
// just be a picture of the car, this is what that costs.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 92
#define CAR GRect(56, 132, 128, 61)

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_floor_draw(ctx, HORIZON, 228);

  // The ball above the horizon, small enough to read as distance rather than as
  // a second subject.
  rocket_ball_draw(ctx, GPoint(154, 66), 11);

  // A contact shadow, so the car is on the floor and not pasted over it.
  studio_fill_ellipse(ctx, GPoint(118, 194), 58, 5, GColorBlack);
  rocket_octane_draw(ctx, CAR, ROCKET_BLUE, true);
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

STUDIO_VARIANT("octane", prv_load, prv_unload, prv_tick)
