// lowline — blueprint's layout with the fills put back.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The head-to-head blueprint asked for. Same horizon, same goal, same car, same
// cleared corner, same Silkscreen — the two tiles differ only in whether the
// arena is drawn as lines or as surfaces. If blueprint holds its own beside this
// one then the walls, the crowd and the colour in the rest of the Session were
// never carrying the depth, and the convergence was.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 124
#define FLOOR_END 150

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_downfield_draw(ctx, HORIZON, ROCKET_FLOOR, GColorLiberty,
                        ROCKET_BLUE_DEEP);

  // The floor is cut off at the same row blueprint's lattice stops at, so the
  // corner the type sits in is black in both. Without this the comparison would
  // be between two backgrounds and two grounds for the numerals at once.
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, FLOOR_END, 200, 228 - FLOOR_END), 0,
                     GCornerNone);

  rocket_goal_draw(ctx, GRect(68, 88, 64, 36), ROCKET_ORANGE);
  rocket_ball_draw(ctx, GPoint(100, 106), 9);
  rocket_fennec_rear(ctx, GPoint(128, 128), 84, 0, 0, ROCKET_BLUE,
                     ROCKET_ORANGE, true, false);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(10, 156, 136, 46),
                                   studio_font(RESOURCE_ID_FONT_SILKSCREEN_38),
                                   GColorWhite, GTextAlignmentLeft);

  s_date_layer = studio_text_layer(
      window_layer, GRect(12, 200, 136, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), ROCKET_BOOST,
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

STUDIO_VARIANT("lowline", prv_load, prv_unload, prv_tick)
