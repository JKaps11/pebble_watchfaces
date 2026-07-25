// nightgame — the arena with the lights off and only the goal lit.
//
// Axes: time-display=digital, composition=centred, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// The opposite answer to "improve the background" from stands: not more in it,
// less. Floor and walls go to black, the floor lines survive as the faintest
// thing on the display, and the only lit objects are the goal, the ball and the
// car's own boost. It should be the most legible tile in the Batch by a distance
// — the question is whether what is left still reads as an arena or just as a
// car on a black field.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 140
#define BANK (-TRIG_MAX_ANGLE * 4 / 360)

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_downfield_draw(ctx, HORIZON, GColorBlack, GColorOxfordBlue,
                        GColorBlack);

  // The goal is drawn wide rather than small, because the numerals go inside its
  // mouth and Wallpoet is a broad segment face — sized for depth instead, the
  // outer digits end up sitting on the frame.
  rocket_goal_draw(ctx, GRect(14, 90, 172, 50), ROCKET_BLUE);

  // The mouth blacked back out behind where the numerals go. The net is dark
  // enough to sit type on but its mesh runs straight through the glyphs, and a
  // segment face has too little stroke to survive that.
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(20, 96, 160, 42), 0, GCornerNone);

  rocket_ball_draw(ctx, GPoint(100, 72), 10);
  rocket_fennec_rear(ctx, GPoint(100, 190), 80, BANK, 0, ROCKET_BLUE, ROCKET_BLUE,
                     true, false);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_date_layer = studio_text_layer(
      window_layer, GRect(8, 34, 184, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), ROCKET_BLUE,
      GTextAlignmentCenter);

  // Over the net, which is the darkest ground the theme owns.
  s_time_layer = studio_text_layer(window_layer, GRect(8, 88, 184, 54),
                                   studio_font(RESOURCE_ID_FONT_WALLPOET_44),
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

STUDIO_VARIANT("nightgame", prv_load, prv_unload, prv_tick)
