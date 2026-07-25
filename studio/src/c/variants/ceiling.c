// ceiling — the aerial from the other end, upside down under the roof.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// Every other tile here has the car climbing away from a floor. This one has run
// out of arena: the Fennec is inverted against the ceiling with the ball falling
// away beneath it, which is the same trick as fennec with the whole frame turned
// over. It is also the only layout in the Batch that leaves the type a corner
// rather than a band, so it doubles as the test of whether these scenes need the
// full width or only most of it.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define SOFFIT 56
#define INVERTED ((TRIG_MAX_ANGLE / 2) + (TRIG_MAX_ANGLE * 7 / 360))

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_ceiling_draw(ctx, SOFFIT);

  rocket_fennec_draw(ctx, GPoint(108, 78), 100, INVERTED, ROCKET_BLUE, true);
  rocket_ball_draw(ctx, GPoint(36, 128), 14);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(10, 148, 182, 56),
                                   studio_font(RESOURCE_ID_FONT_SHARETECH_46),
                                   GColorWhite, GTextAlignmentLeft);

  s_date_layer = studio_text_layer(
      window_layer, GRect(12, 198, 180, 24),
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

STUDIO_VARIANT("ceiling", prv_load, prv_unload, prv_tick)
