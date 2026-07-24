#pragma once

#include <stdbool.h>
#include <stddef.h>

// Plain broken-out time, deliberately not struct tm: shared/c must stay free of
// any platform's time.h (Pebble's own build blocks the real <time.h> and supplies
// its own struct tm from pebble.h, so depending on either one here would tie this
// module to a specific platform). Callers convert their own time representation
// into this before calling the functions below.
typedef struct {
  int hour;          // 0-23
  int minute;        // 0-59
  int weekday;        // 0 = Sunday .. 6 = Saturday
  int month;          // 0 = January .. 11 = December
  int day_of_month;   // 1-31
} DateTimeInfo;

// Formats a time as "H:MM AM/PM" (12h) or "HH:MM" (24h) into buffer.
void datetime_format_time(const DateTimeInfo *time_info, bool use_24h, char *buffer, size_t buffer_size);

// Formats a date as "Www Mon D" (e.g. "Wed Jul 24") into buffer.
void datetime_format_date(const DateTimeInfo *time_info, char *buffer, size_t buffer_size);
