// apex — the scene pushed to the bottom so the time can sit in the middle.
//
// Axes: time-display=digital, composition=centred, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The centred position on the composition axis, which this scene makes awkward:
// the middle of the display is where the goal and the horizon want to be, and
// the numerals want it too. Dropping the horizon to 186 hands the top two thirds
// back to black and lets the type sit on nothing — at the cost of an arena
// squeezed into a strip. The trade is the finding.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 186
#define BANK (TRIG_MAX_ANGLE * 6 / 360)
#define PITCH 0

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_downfield_draw(ctx, HORIZON, ROCKET_FLOOR, GColorLiberty,
                        ROCKET_BLUE_DEEP);

  rocket_goal_draw(ctx, GRect(66, 150, 68, 36), ROCKET_ORANGE);
  rocket_ball_draw(ctx, GPoint(100, 164), 8);

  rocket_fennec_rear(ctx, GPoint(100, 192), 86, BANK, PITCH, ROCKET_BLUE,
                     ROCKET_ORANGE, true, false);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_date_layer = studio_text_layer(
      window_layer, GRect(8, 54, 184, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), ROCKET_BOOST,
      GTextAlignmentCenter);

  s_time_layer = studio_text_layer(window_layer, GRect(8, 78, 184, 56),
                                   studio_font(RESOURCE_ID_FONT_BARLOW_52),
                                   GColorWhite, GTextAlignmentCenter);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_scene_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), s_date_text,
                sizeof(s_date_text));
  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
}

STUDIO_VARIANT("apex", prv_load, prv_unload, prv_tick)
