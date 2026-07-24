#include "unity.h"
#include "datetime_format.h"

#include <string.h>

static DateTimeInfo make_datetime(int hour, int minute, int weekday, int month, int day_of_month) {
  DateTimeInfo time_info = {0};
  time_info.hour = hour;
  time_info.minute = minute;
  time_info.weekday = weekday;
  time_info.month = month;
  time_info.day_of_month = day_of_month;
  return time_info;
}

void setUp(void) {}
void tearDown(void) {}

static void test_time_24h_morning(void) {
  DateTimeInfo time_info = make_datetime(9, 5, 0, 0, 1);
  char buffer[16];
  datetime_format_time(&time_info, true, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("09:05", buffer);
}

static void test_time_24h_afternoon(void) {
  DateTimeInfo time_info = make_datetime(14, 32, 0, 0, 1);
  char buffer[16];
  datetime_format_time(&time_info, true, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("14:32", buffer);
}

static void test_time_12h_midnight(void) {
  DateTimeInfo time_info = make_datetime(0, 0, 0, 0, 1);
  char buffer[16];
  datetime_format_time(&time_info, false, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("12:00 AM", buffer);
}

static void test_time_12h_noon(void) {
  DateTimeInfo time_info = make_datetime(12, 0, 0, 0, 1);
  char buffer[16];
  datetime_format_time(&time_info, false, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("12:00 PM", buffer);
}

static void test_time_12h_afternoon(void) {
  DateTimeInfo time_info = make_datetime(14, 32, 0, 0, 1);
  char buffer[16];
  datetime_format_time(&time_info, false, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("2:32 PM", buffer);
}

static void test_time_12h_morning_single_digit_hour(void) {
  DateTimeInfo time_info = make_datetime(9, 5, 0, 0, 1);
  char buffer[16];
  datetime_format_time(&time_info, false, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("9:05 AM", buffer);
}

static void test_date_midweek(void) {
  DateTimeInfo time_info = make_datetime(0, 0, 3, 6, 24); // Wed, Jul 24
  char buffer[16];
  datetime_format_date(&time_info, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("Wed Jul 24", buffer);
}

static void test_date_new_year(void) {
  DateTimeInfo time_info = make_datetime(0, 0, 0, 0, 1); // Sun, Jan 1
  char buffer[16];
  datetime_format_date(&time_info, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("Sun Jan 1", buffer);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_time_24h_morning);
  RUN_TEST(test_time_24h_afternoon);
  RUN_TEST(test_time_12h_midnight);
  RUN_TEST(test_time_12h_noon);
  RUN_TEST(test_time_12h_afternoon);
  RUN_TEST(test_time_12h_morning_single_digit_hour);
  RUN_TEST(test_date_midweek);
  RUN_TEST(test_date_new_year);
  return UNITY_END();
}
