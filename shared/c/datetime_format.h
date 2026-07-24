#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

// Formats a time as "H:MM AM/PM" (12h) or "HH:MM" (24h) into buffer.
void datetime_format_time(const struct tm *time_info, bool use_24h, char *buffer, size_t buffer_size);

// Formats a date as "Www Mon D" (e.g. "Wed Jul 24") into buffer.
void datetime_format_date(const struct tm *time_info, char *buffer, size_t buffer_size);
