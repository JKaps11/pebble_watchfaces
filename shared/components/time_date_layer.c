#include "time_date_layer.h"
#include "datetime_format.h"

typedef struct {
  char time_text[16];
  char date_text[16];
} TimeDateLayerData;

static void prv_update_proc(Layer *layer, GContext *ctx) {
  TimeDateLayerData *data = (TimeDateLayerData *)layer_get_data(layer);
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_text_color(ctx, GColorBlack);

  GRect time_rect = GRect(0, 0, bounds.size.w, bounds.size.h / 2);
  graphics_draw_text(ctx, data->time_text, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
                      time_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

  GRect date_rect = GRect(0, bounds.size.h / 2, bounds.size.w, bounds.size.h / 2);
  graphics_draw_text(ctx, data->date_text, fonts_get_system_font(FONT_KEY_GOTHIC_18),
                      date_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

Layer *time_date_layer_create(GRect frame) {
  Layer *layer = layer_create_with_data(frame, sizeof(TimeDateLayerData));
  TimeDateLayerData *data = (TimeDateLayerData *)layer_get_data(layer);
  data->time_text[0] = '\0';
  data->date_text[0] = '\0';
  layer_set_update_proc(layer, prv_update_proc);
  return layer;
}

void time_date_layer_destroy(Layer *layer) {
  layer_destroy(layer);
}

void time_date_layer_update(Layer *layer, struct tm *tick_time) {
  TimeDateLayerData *data = (TimeDateLayerData *)layer_get_data(layer);
  DateTimeInfo time_info = {
    .hour = tick_time->tm_hour,
    .minute = tick_time->tm_min,
    .weekday = tick_time->tm_wday,
    .month = tick_time->tm_mon,
    .day_of_month = tick_time->tm_mday,
  };
  datetime_format_time(&time_info, clock_is_24h_style(), data->time_text, sizeof(data->time_text));
  datetime_format_date(&time_info, data->date_text, sizeof(data->date_text));
  layer_mark_dirty(layer);
}
