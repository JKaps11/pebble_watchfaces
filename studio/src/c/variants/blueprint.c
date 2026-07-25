// blueprint — the same shot as a wireframe, with no fills at all.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=mono, type=custom, polarity=light-on-dark
//
// The Batch's control on the whole question. Every other tile answers "improve
// the background" by adding to it; this one asks whether the background needs
// colour or mass at all, or whether the convergence was doing the work the whole
// time. It is the only mono tile, the only one that leaves the type a corner
// instead of a band, and if it holds up then most of what the others spend is
// spent on decoration.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define HORIZON 124
#define FLOOR_END 150
#define GOAL GRect(68, 88, 64, 36)

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static char s_time_text[16];
static char s_date_text[16];

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorLightGray);
  graphics_context_set_stroke_width(ctx, 1);

  // The floor as a lattice: rays to the vanishing point, and rungs across them
  // spaced wider as they come forward, which is the only thing that makes a grid
  // read as a receding plane rather than as a net.
  // The lattice stops at FLOOR_END and the corner below it is left black. A
  // wireframe floor is the worst ground for type there is — every glyph gets a
  // line through it — so the one thing this Variant must not do is run its own
  // background underneath its own numerals.
  for (int x = -180; x <= 380; x += 32) {
    graphics_draw_line(ctx, GPoint(100, HORIZON), GPoint(x, FLOOR_END));
  }
  {
    int y = HORIZON + 4;
    int step = 5;
    while (y < FLOOR_END) {
      graphics_draw_line(ctx, GPoint(0, y), GPoint(200, y));
      y += step;
      step += 4;
    }
  }

  // The side walls as two lines each, and the far wall as one.
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(0, HORIZON - 96), GPoint(36, HORIZON - 44));
  graphics_draw_line(ctx, GPoint(200, HORIZON - 96), GPoint(164, HORIZON - 44));
  graphics_draw_line(ctx, GPoint(36, HORIZON - 44), GPoint(164, HORIZON - 44));
  graphics_draw_line(ctx, GPoint(36, HORIZON - 44), GPoint(36, HORIZON));
  graphics_draw_line(ctx, GPoint(164, HORIZON - 44), GPoint(164, HORIZON));
  graphics_draw_line(ctx, GPoint(0, HORIZON), GPoint(200, HORIZON));

  // The goal as an outline with its shoulders, drawn the way the solid one is
  // built so the two read as the same object.
  graphics_draw_rect(ctx, GOAL);
  graphics_draw_line(ctx, GPoint(GOAL.origin.x, GOAL.origin.y + 12),
                     GPoint(GOAL.origin.x + 12, GOAL.origin.y));
  graphics_draw_line(ctx,
                     GPoint(GOAL.origin.x + GOAL.size.w, GOAL.origin.y + 12),
                     GPoint(GOAL.origin.x + GOAL.size.w - 12, GOAL.origin.y));

  rocket_ball_draw(ctx, GPoint(100, 106), 9);
  rocket_fennec_rear(ctx, GPoint(128, 128), 84, 0, 0, GColorWhite, GColorWhite,
                     true, true);
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
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorLightGray,
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

STUDIO_VARIANT("blueprint", prv_load, prv_unload, prv_tick)
