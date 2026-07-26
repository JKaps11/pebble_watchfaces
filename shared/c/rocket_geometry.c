#include "rocket_geometry.h"

RocketPose rocket_pose_level(int x, int y, int scale) {
  RocketPose pose = {
    .origin_x = x,
    .origin_y = y,
    .scale = scale,
    .vscale = 100,
    .cos = ROCKET_TRIG_MAX,
    .sin = 0,
  };
  return pose;
}

RocketPoint rocket_project(const RocketPose *pose, int cx, int cy, int u,
                           int v) {
  int du = (u - cx) * pose->scale / ROCKET_GRID;
  int dv = (v - cy) * pose->scale * pose->vscale / (ROCKET_GRID * 100);
  RocketPoint at = {
    .x = pose->origin_x + (((du * pose->cos) - (dv * pose->sin))
                           / ROCKET_TRIG_MAX),
    .y = pose->origin_y + (((du * pose->sin) + (dv * pose->cos))
                           / ROCKET_TRIG_MAX),
  };
  return at;
}

bool rocket_polygon_span(const RocketPoint *points, int count, int row,
                         int *left, int *right) {
  int found_left = 0;
  int found_right = 0;
  bool any = false;

  for (int i = 0; i < count; i++) {
    RocketPoint a = points[i];
    RocketPoint b = points[(i + 1) % count];
    int top = (a.y < b.y) ? a.y : b.y;
    int bottom = (a.y < b.y) ? b.y : a.y;
    int x;

    // A horizontal edge contributes nothing: its own endpoints are already
    // contributed by the two edges either side of it, and dividing by its zero
    // height would be the only other option.
    if (a.y == b.y) {
      continue;
    }
    if (row < top || row > bottom) {
      continue;
    }

    x = a.x + (((b.x - a.x) * (row - a.y)) / (b.y - a.y));
    if (!any || x < found_left) {
      found_left = x;
    }
    if (!any || x > found_right) {
      found_right = x;
    }
    any = true;
  }

  if (!any) {
    return false;
  }
  *left = found_left;
  *right = found_right;
  return true;
}
