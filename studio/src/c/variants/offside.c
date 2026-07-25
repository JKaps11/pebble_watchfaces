// offside — the shot taken from off the centre line, type down in one corner.
//
// Axes: time-display=digital, composition=asymmetric, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The asymmetric position, and the one the head-on vantage resists hardest: this
// scene's whole structure is a convergence on the middle, so throwing mass off
// axis means fighting the background rather than using it. The car goes left of
// the vanishing point and the ball high left of that, leaving the type the
// bottom-right and a diagonal of empty floor between them.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 130
#define FLOOR_END 152
#define BANK (TRIG_MAX_ANGLE * 12 / 360)
#define PITCH 0

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_downfield_draw(ctx, HORIZON, ROCKET_FLOOR, GColorLiberty,
                        ROCKET_BLUE_DEEP);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, FLOOR_END, 200, 228 - FLOOR_END), 0,
                     GCornerNone);

  rocket_goal_draw(ctx, GRect(60, 84, 80, 46), ROCKET_ORANGE);
  rocket_ball_draw(ctx, GPoint(42, 62), 12);

  rocket_fennec_rear(ctx, GPoint(72, 128), 88, BANK, PITCH, ROCKET_BLUE,
                     ROCKET_ORANGE, true, false);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(8, 156, 184, 54),
                                   studio_font(RESOURCE_ID_FONT_SHARETECH_46),
                                   GColorWhite, GTextAlignmentRight);

  s_date_layer = studio_text_layer(
      window_layer, GRect(8, 202, 184, 22),
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

STUDIO_VARIANT("offside", prv_load, prv_unload, prv_tick)
