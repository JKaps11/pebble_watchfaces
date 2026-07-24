#include "datetime_format.h"

#include <stdio.h>

static const char *const WEEKDAY_NAMES[7] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

static const char *const MONTH_NAMES[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

void datetime_format_time(const DateTimeInfo *time_info, bool use_24h, char *buffer, size_t buffer_size) {
  if (use_24h) {
    snprintf(buffer, buffer_size, "%02d:%02d", time_info->hour, time_info->minute);
    return;
  }

  int hour12 = time_info->hour % 12;
  if (hour12 == 0) {
    hour12 = 12;
  }
  const char *meridiem = (time_info->hour < 12) ? "AM" : "PM";
  snprintf(buffer, buffer_size, "%d:%02d %s", hour12, time_info->minute, meridiem);
}

void datetime_format_date(const DateTimeInfo *time_info, char *buffer, size_t buffer_size) {
  // Caller must supply a valid DateTimeInfo (weekday in [0,6], month in [0,11]);
  // the modulo below is a defensive clamp, not a documented tolerance for bad input.
  const char *weekday = WEEKDAY_NAMES[time_info->weekday % 7];
  const char *month = MONTH_NAMES[time_info->month % 12];
  snprintf(buffer, buffer_size, "%s %s %d", weekday, month, time_info->day_of_month);
}
