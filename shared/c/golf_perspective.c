#include "golf_perspective.h"

static int prv_span(const GolfView *view) {
  int span = view->ground - view->horizon;
  return (span > 0) ? span : 1;
}

int golf_depth_at_row(const GolfView *view, int row) {
  return (row - view->horizon) * GOLF_DEPTH_MAX / prv_span(view);
}

int golf_row_at_depth(const GolfView *view, int depth) {
  return view->horizon + (depth * prv_span(view) / GOLF_DEPTH_MAX);
}

int golf_centre_at_depth(int depth) {
  return GOLF_FAR_CENTRE
         - ((GOLF_FAR_CENTRE - GOLF_NEAR_CENTRE) * depth / GOLF_DEPTH_MAX);
}

int golf_half_at_depth(int depth) {
  return GOLF_FAR_HALF
         + ((GOLF_NEAR_HALF - GOLF_FAR_HALF) * depth / GOLF_DEPTH_MAX);
}

int golf_centre_at_row(const GolfView *view, int row) {
  return golf_centre_at_depth(golf_depth_at_row(view, row));
}

int golf_half_at_row(const GolfView *view, int row) {
  return golf_half_at_depth(golf_depth_at_row(view, row));
}

int golf_stripe_band(const GolfView *view, int row, int first_depth,
                     int growth) {
  int offset = row - view->horizon;
  int band = 0;
  int top = 0;
  int depth = (first_depth > 0) ? first_depth : 1;

  if (offset < 0) {
    return 0;
  }

  // Walked rather than solved: the closed form is a quadratic in the band index
  // and needs a square root, which is more machinery than a loop over at most a
  // dozen bands deserves.
  while (offset >= top + depth) {
    top += depth;
    depth += growth;
    band++;
  }
  return band;
}
