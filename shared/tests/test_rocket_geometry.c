#include "rocket_geometry.h"
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

// A level pose at unit scale leaves the grid alone apart from the shift, which
// is the base case every other test is a departure from.
static void test_level_pose_only_translates(void) {
  RocketPose pose = rocket_pose_level(100, 50, ROCKET_GRID);
  RocketPoint at = rocket_project(&pose, 50, 24, 50, 24);

  TEST_ASSERT_EQUAL_INT(100, at.x);
  TEST_ASSERT_EQUAL_INT(50, at.y);
}

static void test_level_pose_offsets_from_grid_centre(void) {
  RocketPose pose = rocket_pose_level(100, 50, ROCKET_GRID);
  RocketPoint nose = rocket_project(&pose, 50, 24, 100, 24);

  // Half a car to the right of centre, at unit scale.
  TEST_ASSERT_EQUAL_INT(150, nose.x);
  TEST_ASSERT_EQUAL_INT(50, nose.y);
}

static void test_scale_shrinks_about_the_centre(void) {
  RocketPose pose = rocket_pose_level(100, 50, ROCKET_GRID / 2);
  RocketPoint nose = rocket_project(&pose, 50, 24, 100, 24);

  TEST_ASSERT_EQUAL_INT(125, nose.x);
  TEST_ASSERT_EQUAL_INT(50, nose.y);
}

// A quarter turn clockwise takes the nose from ahead to below, which is the
// convention the whole module depends on: positive rotation puts the nose down.
static void test_quarter_turn_puts_the_nose_below(void) {
  RocketPose pose = rocket_pose_level(100, 50, ROCKET_GRID);
  RocketPoint nose;

  pose.cos = 0;
  pose.sin = ROCKET_TRIG_MAX;
  nose = rocket_project(&pose, 50, 24, 100, 24);

  TEST_ASSERT_EQUAL_INT(100, nose.x);
  TEST_ASSERT_EQUAL_INT(100, nose.y);
}

// Foreshortening is vertical only, and it happens in the car's frame — so on a
// level car it leaves x alone and halves the vertical offset.
static void test_vscale_compresses_the_vertical_only(void) {
  RocketPose pose = rocket_pose_level(100, 50, ROCKET_GRID);
  RocketPoint roof;

  pose.vscale = 50;
  roof = rocket_project(&pose, 50, 24, 100, 4);

  TEST_ASSERT_EQUAL_INT(150, roof.x);
  TEST_ASSERT_EQUAL_INT(40, roof.y);
}

// The squash is applied before the rotation. On a car turned a quarter turn,
// that means it shows up along the display's x rather than its y — which is the
// bug this test exists to pin down, because doing it the other way round looks
// almost right until the car banks.
static void test_vscale_follows_the_car_not_the_display(void) {
  RocketPose pose = rocket_pose_level(100, 50, ROCKET_GRID);
  RocketPoint roof;

  pose.vscale = 50;
  pose.cos = 0;
  pose.sin = ROCKET_TRIG_MAX;
  roof = rocket_project(&pose, 50, 24, 50, 4);

  TEST_ASSERT_EQUAL_INT(110, roof.x);
  TEST_ASSERT_EQUAL_INT(50, roof.y);
}

static void test_span_across_a_square(void) {
  RocketPoint square[] = { { 10, 10 }, { 30, 10 }, { 30, 30 }, { 10, 30 } };
  int left = 0;
  int right = 0;

  TEST_ASSERT_TRUE(rocket_polygon_span(square, 4, 20, &left, &right));
  TEST_ASSERT_EQUAL_INT(10, left);
  TEST_ASSERT_EQUAL_INT(30, right);
}

// Both bounding rows are inside the shape. A fill that dropped them would leave
// a car with a transparent roofline.
static void test_span_includes_the_bounding_rows(void) {
  RocketPoint square[] = { { 10, 10 }, { 30, 10 }, { 30, 30 }, { 10, 30 } };
  int left = 0;
  int right = 0;

  TEST_ASSERT_TRUE(rocket_polygon_span(square, 4, 10, &left, &right));
  TEST_ASSERT_TRUE(rocket_polygon_span(square, 4, 30, &left, &right));
}

static void test_span_narrows_down_a_triangle(void) {
  RocketPoint triangle[] = { { 20, 0 }, { 40, 40 }, { 0, 40 } };
  int wide_left = 0;
  int wide_right = 0;
  int narrow_left = 0;
  int narrow_right = 0;

  TEST_ASSERT_TRUE(rocket_polygon_span(triangle, 3, 10, &narrow_left,
                                       &narrow_right));
  TEST_ASSERT_TRUE(rocket_polygon_span(triangle, 3, 30, &wide_left,
                                       &wide_right));
  TEST_ASSERT_TRUE((narrow_right - narrow_left) < (wide_right - wide_left));
}

static void test_span_rejects_a_row_outside_the_polygon(void) {
  RocketPoint square[] = { { 10, 10 }, { 30, 10 }, { 30, 30 }, { 10, 30 } };
  int left = -1;
  int right = -1;

  TEST_ASSERT_FALSE(rocket_polygon_span(square, 4, 40, &left, &right));
  TEST_ASSERT_EQUAL_INT(-1, left);
  TEST_ASSERT_EQUAL_INT(-1, right);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_level_pose_only_translates);
  RUN_TEST(test_level_pose_offsets_from_grid_centre);
  RUN_TEST(test_scale_shrinks_about_the_centre);
  RUN_TEST(test_quarter_turn_puts_the_nose_below);
  RUN_TEST(test_vscale_compresses_the_vertical_only);
  RUN_TEST(test_vscale_follows_the_car_not_the_display);
  RUN_TEST(test_span_across_a_square);
  RUN_TEST(test_span_includes_the_bounding_rows);
  RUN_TEST(test_span_narrows_down_a_triangle);
  RUN_TEST(test_span_rejects_a_row_outside_the_polygon);
  return UNITY_END();
}
