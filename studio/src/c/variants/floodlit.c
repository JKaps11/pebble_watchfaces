// floodlit — the goal already going off, washing the arena from the horizon.
//
// Axes: time-display=digital, composition=asymmetric, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The one tile where the background is not a place but an event: rays coming out
// of the goal mouth, which is what a Rocket League net does the instant it is
// scored in. It also throws the car off centre so the light and the mass pull in
// opposite directions. The risk is obvious and worth measuring rather than
// arguing about — rays behind numerals is exactly the kind of busy ground that
// makes a time unreadable.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 150
#define BANK (TRIG_MAX_ANGLE * 8 / 360)
#define BURST_X 100
#define BURST_Y 126

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_downfield_draw(ctx, HORIZON, ROCKET_FLOOR, GColorLiberty,
                        ROCKET_BLUE_DEEP);

  // Rays out of the goal mouth, alternating so the burst has texture rather
  // than being a solid fan. Drawn before the goal, so the goal reads as the
  // thing they are coming out of.
  graphics_context_set_stroke_width(ctx, 3);
  for (int i = 0; i < 16; i++) {
    int32_t at = TRIG_MAX_ANGLE * i / 16;
    graphics_context_set_stroke_color(ctx, (i % 2) ? ROCKET_BOOST
                                                   : ROCKET_ORANGE);
    graphics_draw_line(ctx, GPoint(BURST_X, BURST_Y),
                       studio_point_at(GPoint(BURST_X, BURST_Y), at,
                                       (i % 2) ? 96 : 66));
  }

  rocket_goal_draw(ctx, GRect(60, 102, 80, 48), ROCKET_ORANGE);
  rocket_ball_draw(ctx, GPoint(100, 126), 12);

  rocket_fennec_rear(ctx, GPoint(70, 186), 92, BANK, 0, ROCKET_BLUE, ROCKET_ORANGE,
                     true, false);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(8, 2, 184, 52),
                                   studio_font(RESOURCE_ID_FONT_ARCHIVO_44),
                                   GColorWhite, GTextAlignmentRight);

  s_date_layer = studio_text_layer(
      window_layer, GRect(8, 52, 184, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), ROCKET_BOOST,
      GTextAlignmentRight);
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

STUDIO_VARIANT("floodlit", prv_load, prv_unload, prv_tick)
