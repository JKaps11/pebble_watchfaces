#include "unity.h"
#include "golf_perspective.h"

// The two views the shipping watchfaces actually use. pinhigh and teebox give
// the scene different amounts of room, and the point of the module is that they
// still see the same hole.
static const GolfView PINHIGH = { .horizon = 74, .ground = 156 };
static const GolfView TEEBOX = { .horizon = 74, .ground = 152 };
static const GolfView FULL = { .horizon = 74, .ground = 228 };

void setUp(void) {}
void tearDown(void) {}

static void test_horizon_row_is_zero_depth(void) {
  TEST_ASSERT_EQUAL_INT(0, golf_depth_at_row(&FULL, FULL.horizon));
}

static void test_ground_row_is_max_depth(void) {
  TEST_ASSERT_EQUAL_INT(GOLF_DEPTH_MAX, golf_depth_at_row(&FULL, FULL.ground));
}

static void test_row_above_horizon_is_negative_depth(void) {
  TEST_ASSERT_TRUE(golf_depth_at_row(&FULL, FULL.horizon - 10) < 0);
}

static void test_depth_and_row_round_trip(void) {
  for (int depth = 0; depth <= GOLF_DEPTH_MAX; depth += 16) {
    int row = golf_row_at_depth(&PINHIGH, depth);
    TEST_ASSERT_INT_WITHIN(4, depth, golf_depth_at_row(&PINHIGH, row));
  }
}

static void test_degenerate_view_does_not_divide_by_zero(void) {
  GolfView flat = { .horizon = 74, .ground = 74 };
  (void)golf_depth_at_row(&flat, 100);
  (void)golf_row_at_depth(&flat, 128);
  TEST_PASS();
}

static void test_fairway_matches_constants_at_both_ends(void) {
  TEST_ASSERT_EQUAL_INT(GOLF_FAR_CENTRE, golf_centre_at_depth(0));
  TEST_ASSERT_EQUAL_INT(GOLF_FAR_HALF, golf_half_at_depth(0));
  TEST_ASSERT_EQUAL_INT(GOLF_NEAR_CENTRE, golf_centre_at_depth(GOLF_DEPTH_MAX));
  TEST_ASSERT_EQUAL_INT(GOLF_NEAR_HALF, golf_half_at_depth(GOLF_DEPTH_MAX));
}

static void test_fairway_widens_toward_the_viewer(void) {
  int previous = golf_half_at_depth(0);
  for (int depth = 8; depth <= GOLF_DEPTH_MAX; depth += 8) {
    int half = golf_half_at_depth(depth);
    TEST_ASSERT_TRUE(half >= previous);
    previous = half;
  }
  TEST_ASSERT_TRUE(golf_half_at_depth(GOLF_DEPTH_MAX) > golf_half_at_depth(0));
}

static void test_fairway_doglegs_left_toward_the_viewer(void) {
  TEST_ASSERT_TRUE(golf_centre_at_depth(GOLF_DEPTH_MAX)
                   < golf_centre_at_depth(0));
}

// The reason this module exists: two watchfaces that hand the scene different
// grounds must still be looking at one hole, not at two that happen to be green.
static void test_two_views_see_the_same_hole(void) {
  for (int depth = 0; depth <= GOLF_DEPTH_MAX; depth += 16) {
    int pin_row = golf_row_at_depth(&PINHIGH, depth);
    int tee_row = golf_row_at_depth(&TEEBOX, depth);
    TEST_ASSERT_INT_WITHIN(2, golf_centre_at_row(&PINHIGH, pin_row),
                           golf_centre_at_row(&TEEBOX, tee_row));
    TEST_ASSERT_INT_WITHIN(4, golf_half_at_row(&PINHIGH, pin_row),
                           golf_half_at_row(&TEEBOX, tee_row));
  }
}

static void test_first_stripe_band_spans_first_depth_rows(void) {
  TEST_ASSERT_EQUAL_INT(0, golf_stripe_band(&FULL, FULL.horizon, 4, 2));
  TEST_ASSERT_EQUAL_INT(0, golf_stripe_band(&FULL, FULL.horizon + 3, 4, 2));
  TEST_ASSERT_EQUAL_INT(1, golf_stripe_band(&FULL, FULL.horizon + 4, 4, 2));
}

static void test_stripe_bands_deepen_by_growth(void) {
  // Bands of 4, then 6, then 8 rows: boundaries at offsets 4, 10 and 18.
  TEST_ASSERT_EQUAL_INT(1, golf_stripe_band(&FULL, FULL.horizon + 9, 4, 2));
  TEST_ASSERT_EQUAL_INT(2, golf_stripe_band(&FULL, FULL.horizon + 10, 4, 2));
  TEST_ASSERT_EQUAL_INT(2, golf_stripe_band(&FULL, FULL.horizon + 17, 4, 2));
  TEST_ASSERT_EQUAL_INT(3, golf_stripe_band(&FULL, FULL.horizon + 18, 4, 2));
}

static void test_stripe_band_above_horizon_is_zero(void) {
  TEST_ASSERT_EQUAL_INT(0, golf_stripe_band(&FULL, FULL.horizon - 5, 4, 2));
}

static void test_stripe_band_survives_zero_first_depth(void) {
  // A caller passing 0 would otherwise loop forever rather than fail loudly.
  TEST_ASSERT_EQUAL_INT(20, golf_stripe_band(&FULL, FULL.horizon + 20, 0, 0));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_horizon_row_is_zero_depth);
  RUN_TEST(test_ground_row_is_max_depth);
  RUN_TEST(test_row_above_horizon_is_negative_depth);
  RUN_TEST(test_depth_and_row_round_trip);
  RUN_TEST(test_degenerate_view_does_not_divide_by_zero);
  RUN_TEST(test_fairway_matches_constants_at_both_ends);
  RUN_TEST(test_fairway_widens_toward_the_viewer);
  RUN_TEST(test_fairway_doglegs_left_toward_the_viewer);
  RUN_TEST(test_two_views_see_the_same_hole);
  RUN_TEST(test_first_stripe_band_spans_first_depth_rows);
  RUN_TEST(test_stripe_bands_deepen_by_growth);
  RUN_TEST(test_stripe_band_above_horizon_is_zero);
  RUN_TEST(test_stripe_band_survives_zero_first_depth);
  return UNITY_END();
}
