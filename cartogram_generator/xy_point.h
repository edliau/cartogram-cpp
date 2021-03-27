#ifndef XY_POINT_H_
#define XY_POINT_H_

struct XY_Point {
  double x;
  double y;

  // Constructor, setting points to 0
  XY_Point() {
    x = 0; y = 0;
  }

  // Constructor with two values given
  XY_Point(double xg, double yg) {
    x = xg; y = yg;
  }
};

#endif
