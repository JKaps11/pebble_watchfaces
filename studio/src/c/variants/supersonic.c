// supersonic — speed rather than scenery, with the mass thrown off both axes.
//
// Axes: time-display=digital, composition=asymmetric, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The Batch's argument that the theme need not be a picture of anything: a car
// going flat out at a ball is a diagonal, and a diagonal is a composition. The
// type is a pixel face because the whole tile is trying to be arcade rather than
// photographic, and it sits top-left against black — the one corner the scene
// deliberately leaves empty.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 132

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_floor_draw(ctx, HORIZON, 220);

  rocket_ball_draw(ctx, GPoint(160, 52), 17);

  // The line the car is on, drawn as it would be seen rather than as an arc:
  // a run of boost trail thinning toward the ball.
  for (int i = 0; i < 7; i++) {
    int x = 96 + (i * 11);
    int y = 132 - (i * 11);
    int size = 7 - i;
    studio_fill_ellipse(ctx, GPoint(x, y), size, size,
                        (i % 2) ? ROCKET_BOOST : ROCKET_ORANGE);
  }

  // Speed lines off the back, which is the whole reason the left of the floor is
  // left empty.
  graphics_context_set_stroke_color(ctx, GColorPictonBlue);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(10, 160), GPoint(44, 160));
  graphics_draw_line(ctx, GPoint(16, 176), GPoint(40, 176));
  graphics_draw_line(ctx, GPoint(10, 196), GPoint(50, 196));

  studio_fill_ellipse(ctx, GPoint(114, 208), 52, 4, GColorBlack);
  rocket_octane_draw(ctx, GRect(52, 152, 112, 55), ROCKET_BLUE, true);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(10, 24, 134, 48),
                                   studio_font(RESOURCE_ID_FONT_SILKSCREEN_38),
                                   GColorWhite, GTextAlignmentLeft);

  s_date_layer = studio_text_layer(
      window_layer, GRect(12, 76, 134, 24),
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

STUDIO_VARIANT("supersonic", prv_load, prv_unload, prv_tick)
