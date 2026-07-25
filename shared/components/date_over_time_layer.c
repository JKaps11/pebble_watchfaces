#include "date_over_time_layer.h"
#include "datetime_format.h"

// The date sits clear of the frame's left edge; the time is flush with it,
// because a large face's own side bearing already supplies the optical inset
// and matching them numerically makes the time look indented.
#define DATE_INSET 2
#define DATE_HEIGHT 22
#define TIME_TOP 12
#define TIME_HEIGHT 56

// Leaves room at the right of the date's line for a complication the watchface
// positions itself — on both golf faces that is the battery cell.
#define DATE_GUTTER 64

typedef struct {
  char time_text[16];
  char date_text[16];
  GFont time_font;
  GColor colour;
} DateOverTimeLayerData;

static void prv_update_proc(Layer *layer, GContext *ctx) {
  DateOverTimeLayerData *data = (DateOverTimeLayerData *)layer_get_data(layer);
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_text_color(ctx, data->colour);

  graphics_draw_text(ctx, data->date_text,
                     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                     GRect(DATE_INSET, 0, bounds.size.w - DATE_GUTTER,
                           DATE_HEIGHT),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft,
                     NULL);

  graphics_draw_text(ctx, data->time_text, data->time_font,
                     GRect(0, TIME_TOP, bounds.size.w, TIME_HEIGHT),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft,
                     NULL);
}

Layer *date_over_time_layer_create(GRect frame, GFont time_font,
                                   GColor colour) {
  Layer *layer = layer_create_with_data(frame,
                                        sizeof(DateOverTimeLayerData));
  DateOverTimeLayerData *data = (DateOverTimeLayerData *)layer_get_data(layer);
  data->time_text[0] = '\0';
  data->date_text[0] = '\0';
  data->time_font = time_font;
  data->colour = colour;
  layer_set_update_proc(layer, prv_update_proc);
  return layer;
}

void date_over_time_layer_destroy(Layer *layer) {
  layer_destroy(layer);
}

void date_over_time_layer_update(Layer *layer, struct tm *tick_time) {
  DateOverTimeLayerData *data = (DateOverTimeLayerData *)layer_get_data(layer);
  DateTimeInfo time_info = {
    .hour = tick_time->tm_hour,
    .minute = tick_time->tm_min,
    .weekday = tick_time->tm_wday,
    .month = tick_time->tm_mon,
    .day_of_month = tick_time->tm_mday,
  };
  datetime_format_time(&time_info, clock_is_24h_style(), data->time_text,
                       sizeof(data->time_text));
  datetime_format_date(&time_info, data->date_text, sizeof(data->date_text));
  layer_mark_dirty(layer);
}
