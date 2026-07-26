// backboard — the moment after the goal, with the time inside the net.
//
// Axes: time-display=digital, composition=centred, complications=1-2,
//       hue=multi, type=LECO, polarity=light-on-dark
//
// The brief asked about scoring, and a goal is a moment rather than a state, so
// the question is whether a still frame of one is worth wearing. The net is the
// darkest thing the theme owns, which makes it the one place a Rocket League
// scene can hand the numerals real contrast — the opposite trade from every
// other Variant here, where the picture is bright and the type has to fight it.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../variant.h"

#define HORIZON 152
#define GOAL GRect(14, 54, 172, 98)

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_floor_draw(ctx, HORIZON, 220);
  rocket_goal_draw(ctx, GOAL, ROCKET_ORANGE);

  // The ball at rest against the back of the net, low and off to one side, with
  // short spokes of pushed mesh around it. Concentric rings were the obvious way
  // to draw that and read as a dartboard, so this is spokes instead.
  graphics_context_set_stroke_color(ctx, GColorLightGray);
  graphics_context_set_stroke_width(ctx, 1);
  for (int i = 0; i < 8; i++) {
    int32_t angle = TRIG_MAX_ANGLE * i / 8;
    graphics_draw_line(ctx, studio_point_at(GPoint(54, 126), angle, 17),
                       studio_point_at(GPoint(54, 126), angle, 25));
  }
  rocket_ball_draw(ctx, GPoint(54, 126), 15);

  // The scorer, driving out of frame, small enough to stay a detail.
  rocket_octane_draw(ctx, GRect(120, 178, 68, 33), ROCKET_BLUE, true);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_time_layer = studio_text_layer(
      window_layer, GRect(14, 66, 172, 52),
      fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS), GColorWhite,
      GTextAlignmentCenter);

  s_date_layer = studio_text_layer(
      window_layer, GRect(10, 186, 104, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentLeft);
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

STUDIO_VARIANT("backboard", prv_load, prv_unload, prv_tick)
