#include "unity.h"
#include "datetime_format.h"

#include <string.h>

static struct tm make_tm(int hour, int min, int wday, int mon, int mday) {
  struct tm time_info = {0};
  time_info.tm_hour = hour;
  time_info.tm_min = min;
  time_info.tm_wday = wday;
  time_info.tm_mon = mon;
  time_info.tm_mday = mday;
  return time_info;
}

void setUp(void) {}
void tearDown(void) {}

static void test_time_24h_morning(void) {
  struct tm time_info = make_tm(9, 5, 0, 0, 1);
  char buffer[16];
  datetime_format_time(&time_info, true, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("09:05", buffer);
}

static void test_time_24h_afternoon(void) {
  struct tm time_info = make_tm(14, 32, 0, 0, 1);
  char buffer[16];
  datetime_format_time(&time_info, true, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("14:32", buffer);
}

static void test_time_12h_midnight(void) {
  struct tm time_info = make_tm(0, 0, 0, 0, 1);
  char buffer[16];
  datetime_format_time(&time_info, false, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("12:00 AM", buffer);
}

static void test_time_12h_noon(void) {
  struct tm time_info = make_tm(12, 0, 0, 0, 1);
  char buffer[16];
  datetime_format_time(&time_info, false, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("12:00 PM", buffer);
}

static void test_time_12h_afternoon(void) {
  struct tm time_info = make_tm(14, 32, 0, 0, 1);
  char buffer[16];
  datetime_format_time(&time_info, false, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("2:32 PM", buffer);
}

static void test_time_12h_morning_single_digit_hour(void) {
  struct tm time_info = make_tm(9, 5, 0, 0, 1);
  char buffer[16];
  datetime_format_time(&time_info, false, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("9:05 AM", buffer);
}

static void test_date_midweek(void) {
  struct tm time_info = make_tm(0, 0, 3, 6, 24); // Wed, Jul 24
  char buffer[16];
  datetime_format_date(&time_info, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_STRING("Wed Jul 24", buffer);
}

static void test_date_new_year(void) {
  struct tm time_info = make_tm(0, 0, 0, 0, 1); // Sun, Jan 1
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
