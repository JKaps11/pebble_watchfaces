// tunnel — the perspective pushed until the arena is a funnel.
//
// Axes: time-display=digital, composition=centred, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// If the new vantage is worth having, the thing it buys is convergence — so this
// tile spends everything on it: a high horizon, walls that fall away steeply and
// a floor of lines all pointing at the same spot. Stripping to one hue is part
// of the same argument, because a funnel drawn in two colours is read as two
// shapes. The time sits on the vanishing point, which is the only place a
// centred layout can put it here.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 120
#define BANK (TRIG_MAX_ANGLE * 4 / 360)

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_downfield_draw(ctx, HORIZON, GColorOxfordBlue, GColorLiberty,
                        GColorDukeBlue);

  // The goal in white rather than a team colour. Black, white and grey do not
  // count against the one hue, which is what lets the frame stay bright without
  // introducing a second colour to the funnel.
  rocket_goal_draw(ctx, GRect(70, 82, 60, 38), GColorWhite);
  rocket_ball_draw(ctx, GPoint(100, 102), 8);

  rocket_fennec_rear(ctx, GPoint(100, 176), 104, BANK, 0, ROCKET_BLUE, ROCKET_BLUE,
                     true, false);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(8, 24, 184, 54),
                                   studio_font(RESOURCE_ID_FONT_SHARETECH_46),
                                   GColorWhite, GTextAlignmentCenter);

  s_date_layer = studio_text_layer(
      window_layer, GRect(8, 2, 184, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
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

STUDIO_VARIANT("tunnel", prv_load, prv_unload, prv_tick)
