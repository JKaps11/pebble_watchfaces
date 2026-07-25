// teemarker — standing on the tee, the hole plaque bolted across the foreground.
//
// Axes: time-display=digital, composition=split, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// yardage crossed with the scene work: the plaque grows to a full-width panel
// and the turf above it becomes a place you are standing rather than a texture.
// The blue tee markers are the only thing on the display that is not grass or
// signage, and they are there deliberately — a golfer reads two blue blocks as
// "the tees" instantly, which is a lot of theme for very few pixels.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define TURF GColorDarkGreen
#define STRIPE GColorMayGreen
#define ACCENT GColorInchworm
#define MARKER GColorBlueMoon
#define PLAQUE_Y 122

static Layer *s_scene_layer;
static TextLayer *s_hole_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorCobaltBlue);
  graphics_fill_rect(ctx, GRect(0, 0, 200, 26), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_rect(ctx, GRect(0, 26, 200, 12), 0, GCornerNone);

  for (int x = -8; x < 210; x += 13) {
    studio_fill_ellipse(ctx, GPoint(x, 38), 9, 6 + ((x * 3) % 9), GColorBlack);
  }

  graphics_context_set_fill_color(ctx, TURF);
  graphics_fill_rect(ctx, GRect(0, 38, 200, PLAQUE_Y - 38), 0, GCornerNone);

  int y = 38;
  int depth = 5;
  for (int band = 0; y < PLAQUE_Y; band++) {
    if (band % 2) {
      graphics_context_set_fill_color(ctx, STRIPE);
      graphics_fill_rect(ctx, GRect(0, y, 200, depth), 0, GCornerNone);
    }
    y += depth;
    depth += 3;
  }

  // The pin, small and far off, and the two tee markers close at hand.
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(112, 48), GPoint(112, 30));
  studio_fill_triangle(ctx, GPoint(126, 34), GPoint(113, 30), GPoint(113, 38),
                       GColorWhite);

  studio_fill_ellipse(ctx, GPoint(38, 104), 15, 11, MARKER);
  studio_fill_ellipse(ctx, GPoint(166, 104), 15, 11, MARKER);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(8, PLAQUE_Y, 184, 98), 6, GCornersAll);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_round_rect(ctx, GRect(8, PLAQUE_Y, 184, 98), 6);

  graphics_context_set_stroke_color(ctx, ACCENT);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(20, 190), GPoint(180, 190));
}

static void prv_draw_battery(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent, ACCENT);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_hole_layer = studio_text_layer(
      window_layer, GRect(20, PLAQUE_Y + 6, 160, 18),
      fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), ACCENT,
      GTextAlignmentCenter);
  text_layer_set_text(s_hole_layer, "HOLE 16   PAR 4");

  s_time_layer = studio_text_layer(window_layer, GRect(14, 142, 172, 52),
                                   studio_font(RESOURCE_ID_FONT_SHARETECH_46),
                                   GColorWhite, GTextAlignmentCenter);

  s_date_layer = studio_text_layer(
      window_layer, GRect(20, 192, 100, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentLeft);

  s_battery_layer = layer_create(GRect(144, 197, 36, 14));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_hole_layer);
  layer_destroy(s_scene_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), s_date_text,
                sizeof(s_date_text));
  s_battery_percent = battery_state_service_peek().charge_percent;

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  layer_mark_dirty(s_battery_layer);
}

STUDIO_VARIANT("teemarker", prv_load, prv_unload, prv_tick)
