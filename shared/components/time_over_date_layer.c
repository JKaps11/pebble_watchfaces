#include "time_over_date_layer.h"
#include "datetime_format.h"

#include <string.h>

// The time takes the top of the frame and the date the strip below it. Both are
// measured from the top rather than split proportionally, because the time's
// font is the caller's and its cap height is not something this component can
// know — anchoring the date to a fraction of the frame would move it every time
// a caller changed face.
#define TIME_HEIGHT 54
#define DATE_HEIGHT 24

// Where the meridiem sits: a hair off the numerals and low enough to share
// their baseline rather than their cap line.
#define MERIDIEM_GAP 4
#define MERIDIEM_TOP 26

typedef struct {
  GFont time_font;
  GColor time_colour;
  GColor date_colour;
  // The time arrives from datetime_format as one string and is drawn as two.
  // In 24-hour mode `meridiem` is empty and the numerals centre exactly; in
  // 12-hour mode the pair is centred as a unit.
  char numerals[16];
  char meridiem[8];
  char date_text[16];
} TimeOverDateLayerData;

static void prv_update_proc(Layer *layer, GContext *ctx) {
  TimeOverDateLayerData *data = (TimeOverDateLayerData *)layer_get_data(layer);
  GRect bounds = layer_get_bounds(layer);
  GFont meridiem_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GRect measure = GRect(0, 0, bounds.size.w, TIME_HEIGHT);
  GSize numerals;
  GSize meridiem;
  int gap;
  int total;
  int left;

  numerals = graphics_text_layout_get_content_size(
      data->numerals, data->time_font, measure, GTextOverflowModeWordWrap,
      GTextAlignmentLeft);
  meridiem = graphics_text_layout_get_content_size(
      data->meridiem, meridiem_font, measure, GTextOverflowModeWordWrap,
      GTextAlignmentLeft);

  gap = (data->meridiem[0] == '\0') ? 0 : MERIDIEM_GAP;
  total = numerals.w + gap + meridiem.w;
  left = (bounds.size.w - total) / 2;

  // The numerals are placed rather than centred by the text engine, because the
  // meridiem has to be measured into the same sum. Centring the two separately
  // would leave them overlapping, and centring the whole string in one draw is
  // what produced an ellipsis: "9:26 AM" set in a 44 px display face is wider
  // than the display, so the engine truncated it to three dots.
  graphics_context_set_text_color(ctx, data->time_colour);
  graphics_draw_text(ctx, data->numerals, data->time_font,
                     GRect(left, 0, numerals.w + 1, TIME_HEIGHT),
                     GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

  if (gap) {
    // Set small and sitting on the numerals' baseline. A meridiem at display
    // size is a third of the width of the time for a twelfth of its
    // information.
    graphics_draw_text(ctx, data->meridiem, meridiem_font,
                       GRect(left + numerals.w + gap, MERIDIEM_TOP,
                             meridiem.w + 1, DATE_HEIGHT),
                       GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  }

  graphics_context_set_text_color(ctx, data->date_colour);
  graphics_draw_text(ctx, data->date_text, meridiem_font,
                     GRect(0, TIME_HEIGHT, bounds.size.w, DATE_HEIGHT),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter,
                     NULL);
}

Layer *time_over_date_layer_create(GRect frame, GFont time_font,
                                   GColor time_colour, GColor date_colour) {
  Layer *layer = layer_create_with_data(frame, sizeof(TimeOverDateLayerData));
  TimeOverDateLayerData *data = (TimeOverDateLayerData *)layer_get_data(layer);

  data->time_font = time_font;
  data->time_colour = time_colour;
  data->date_colour = date_colour;
  data->numerals[0] = '\0';
  data->meridiem[0] = '\0';
  data->date_text[0] = '\0';
  layer_set_update_proc(layer, prv_update_proc);
  return layer;
}

void time_over_date_layer_destroy(Layer *layer) {
  layer_destroy(layer);
}

void time_over_date_layer_update(Layer *layer, struct tm *tick_time) {
  TimeOverDateLayerData *data = (TimeOverDateLayerData *)layer_get_data(layer);
  DateTimeInfo time_info = {
    .hour = tick_time->tm_hour,
    .minute = tick_time->tm_min,
    .weekday = tick_time->tm_wday,
    .month = tick_time->tm_mon,
    .day_of_month = tick_time->tm_mday,
  };

  char formatted[16];
  char *space;

  datetime_format_time(&time_info, clock_is_24h_style(), formatted,
                       sizeof(formatted));

  // datetime_format hands back "HH:MM" or "H:MM AM". Splitting on the space
  // here rather than asking the formatter for the pieces keeps that module's
  // contract — it formats a time for reading, and how a watchface chooses to
  // set the result is not its business.
  space = strchr(formatted, ' ');
  if (space) {
    *space = '\0';
    strncpy(data->meridiem, space + 1, sizeof(data->meridiem) - 1);
    data->meridiem[sizeof(data->meridiem) - 1] = '\0';
  } else {
    data->meridiem[0] = '\0';
  }
  strncpy(data->numerals, formatted, sizeof(data->numerals) - 1);
  data->numerals[sizeof(data->numerals) - 1] = '\0';

  datetime_format_date(&time_info, data->date_text, sizeof(data->date_text));
  layer_mark_dirty(layer);
}
