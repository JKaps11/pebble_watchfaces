// approach — a hundred yards out, the green filling the frame off to one side.
//
// Axes: time-display=digital, composition=asymmetric, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The other way to add background: move the viewer instead of the weather. From
// here the hole is one large shape rather than a receding corridor, and putting
// the green off-centre is what buys the asymmetry — mass to the right up top,
// mass to the left down below where the numerals sit. It also gets the date and
// the battery out of the foreground band and onto the sky, which is the test:
// whether a complication can live on the picture without the picture losing.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define TURF GColorDarkGreen
#define SURFACE GColorIslamicGreen
#define SAND GColorRajah
#define HORIZON 66
#define GROUND 148

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorCobaltBlue);
  graphics_fill_rect(ctx, GRect(0, 0, 200, 40), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorBlueMoon);
  graphics_fill_rect(ctx, GRect(0, 40, 200, 16), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_rect(ctx, GRect(0, 56, 200, 12), 0, GCornerNone);

  for (int x = -8; x < 210; x += 15) {
    studio_fill_ellipse(ctx, GPoint(x, HORIZON), 10, 8 + ((x * 3) % 10),
                        GColorBlack);
  }

  graphics_context_set_fill_color(ctx, TURF);
  graphics_fill_rect(ctx, GRect(0, HORIZON, 200, GROUND - HORIZON), 0,
                     GCornerNone);

  // The green, large and set to the right, with the pin over the treeline.
  studio_fill_ellipse(ctx, GPoint(130, 102), 62, 30, SURFACE);
  studio_fill_ellipse(ctx, GPoint(52, 124), 32, 13, SAND);
  studio_fill_ellipse(ctx, GPoint(178, 76), 22, 8, SAND);

  graphics_context_set_fill_color(ctx, GColorBlack);
  studio_fill_ellipse(ctx, GPoint(130, 100), 5, 3, GColorBlack);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(130, 100), GPoint(130, 40));
  studio_fill_triangle(ctx, GPoint(162, 50), GPoint(131, 40), GPoint(131, 60),
                       GColorWhite);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, GROUND, 200, 228 - GROUND), 0, GCornerNone);
}

static void prv_draw_battery(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent,
                      GColorWhite);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_date_layer = studio_text_layer(
      window_layer, GRect(10, 10, 116, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentLeft);

  s_battery_layer = layer_create(GRect(152, 14, 36, 14));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(10, 162, 182, 58),
                                   studio_font(RESOURCE_ID_FONT_ARCHIVO_44),
                                   GColorWhite, GTextAlignmentLeft);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
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

STUDIO_VARIANT("approach", prv_load, prv_unload, prv_tick)
