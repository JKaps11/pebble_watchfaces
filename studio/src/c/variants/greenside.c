// greenside — a few paces off the green, the pin close enough to putt at.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// The other end of the hole. Instead of the pin as a speck on the horizon it is
// the largest thing on the display, with the cup, a ball and the greenside
// bunker at a size you can actually read. overlay proved a heavy black keyline
// will hold white type straight on the turf with no panel at all, so the picture
// keeps every row: this is the most golf and the least watch of the Batch, and
// the point of it is to find out whether that trade is worth making.

#include <pebble.h>
#include "../golf_course.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define KEYLINE 2

static Layer *s_scene_layer;
static Layer *s_type_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  golf_green_draw(ctx, 228);
}

// White type with a black keyline, drawn by stamping the string around itself.
// Expensive, and the only way to put text on a picture without a panel under it.
static void prv_knockout(GContext *ctx, const char *text, GFont font, GRect box,
                         GTextAlignment alignment) {
  graphics_context_set_text_color(ctx, GColorBlack);
  for (int dx = -KEYLINE; dx <= KEYLINE; dx++) {
    for (int dy = -KEYLINE; dy <= KEYLINE; dy++) {
      if (dx == 0 && dy == 0) {
        continue;
      }
      graphics_draw_text(ctx, text, font,
                         GRect(box.origin.x + dx, box.origin.y + dy,
                               box.size.w, box.size.h),
                         GTextOverflowModeTrailingEllipsis, alignment, NULL);
    }
  }
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, text, font, box, GTextOverflowModeTrailingEllipsis,
                     alignment, NULL);
}

static void prv_draw_type(Layer *layer, GContext *ctx) {
  prv_knockout(ctx, s_date_text,
               fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
               GRect(12, 142, 120, 22), GTextAlignmentLeft);
  prv_knockout(ctx, s_time_text, studio_font(RESOURCE_ID_FONT_BARLOW_52),
               GRect(10, 164, 180, 56), GTextAlignmentLeft);

  GRect cell = GRect(150, 146, 36, 14);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(cell.origin.x - KEYLINE,
                                cell.origin.y - KEYLINE,
                                cell.size.w + (KEYLINE * 2),
                                cell.size.h + (KEYLINE * 2)),
                     2, GCornersAll);
  studio_battery_cell(ctx, cell, s_battery_percent, GColorWhite);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(bounds);
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_type_layer = layer_create(bounds);
  layer_set_update_proc(s_type_layer, prv_draw_type);
  layer_add_child(window_layer, s_type_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_type_layer);
  layer_destroy(s_scene_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), s_date_text,
                sizeof(s_date_text));
  s_battery_percent = battery_state_service_peek().charge_percent;
  layer_mark_dirty(s_type_layer);
}

STUDIO_VARIANT("greenside", prv_load, prv_unload, prv_tick)
