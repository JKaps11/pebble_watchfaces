// kickoff — the pitch from above, with the time laid across the halfway line.
//
// Axes: time-display=digital, composition=centred, complications=0,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The other way to say Rocket League without drawing a car in profile: draw the
// pitch, which nobody who has played the game needs a caption for. Nothing but
// the time is on it, so this is also the Batch's test of whether the theme can
// carry a face with no complications at all to make it useful.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define PITCH GRect(8, 8, 184, 212)
#define BAND_Y 90
#define BAND_H 52

static Layer *s_scene_layer;
static TextLayer *s_time_layer;

static char s_time_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_pitch_draw(ctx, PITCH);

  rocket_octane_top(ctx, GPoint(54, 46), 36, ROCKET_BLUE, true);
  rocket_octane_top(ctx, GPoint(146, 182), 36, ROCKET_ORANGE, false);
  rocket_ball_draw(ctx, GPoint(100, 66), 14);

  // The halfway line, widened until it is a plate the numerals can sit on. Its
  // two edges are the team colours in the order the pitch already has them, so
  // the plate reads as part of the pitch rather than as a panel over it.
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(8, BAND_Y, 184, BAND_H), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, ROCKET_BLUE);
  graphics_fill_rect(ctx, GRect(8, BAND_Y, 184, 3), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, ROCKET_ORANGE);
  graphics_fill_rect(ctx, GRect(8, BAND_Y + BAND_H - 3, 184, 3), 0,
                     GCornerNone);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_time_layer = studio_text_layer(window_layer,
                                   GRect(8, BAND_Y - 1, 184, BAND_H),
                                   studio_font(RESOURCE_ID_FONT_BARLOW_52),
                                   GColorWhite, GTextAlignmentCenter);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  layer_destroy(s_scene_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), NULL, 0);
  text_layer_set_text(s_time_layer, s_time_text);
}

STUDIO_VARIANT("kickoff", prv_load, prv_unload, prv_tick)
