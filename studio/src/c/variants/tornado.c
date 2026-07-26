// tornado — the spin itself, drawn stroboscopically up a diagonal.
//
// Axes: time-display=digital, composition=asymmetric, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// A car at a strange angle is not a spin; a car at six angles along one line is.
// The copies darken backwards down the diagonal, so the eye reads an order and
// therefore a direction of travel, and the corner the diagonal vacates is where
// the type goes. This is the tile that tests whether a still frame can hold
// motion at all — everything else in this Batch either freezes it or encodes it.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 178
#define GHOSTS 5
#define CAR 54

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

// Darkest first: the trail is read back to front, so the order these come out in
// is the order the car went through them.
static GColor prv_ghost_colour(int i) {
  switch (i) {
    case 0: return GColorOxfordBlue;
    case 1: return GColorDukeBlue;
    case 2: return GColorLiberty;
    case 3: return GColorBlueMoon;
    default: return GColorPictonBlue;
  }
}

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  rocket_floor_draw(ctx, HORIZON, 222);

  // Five ghosts up the diagonal, then the car itself at the head of it. A whole
  // turn is spread across the six, so the last one lands back near level and the
  // spin reads as one revolution rather than as a random scatter of angles.
  for (int i = 0; i < GHOSTS; i++) {
    int x = 46 + (i * 22);
    int y = 200 - (i * 21);
    int32_t angle = TRIG_MAX_ANGLE * i / 6;
    rocket_fennec_ghost(ctx, GPoint(x, y), CAR, angle, prv_ghost_colour(i));
  }

  rocket_fennec_draw(ctx, GPoint(156, 96), CAR, TRIG_MAX_ANGLE * 5 / 6,
                     ROCKET_BLUE, true);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(10, 14, 132, 46),
                                   studio_font(RESOURCE_ID_FONT_SILKSCREEN_38),
                                   GColorWhite, GTextAlignmentLeft);

  s_date_layer = studio_text_layer(
      window_layer, GRect(12, 62, 132, 24),
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

STUDIO_VARIANT("tornado", prv_load, prv_unload, prv_tick)
