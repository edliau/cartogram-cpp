#ifndef ROUND_POINT_H_
#define ROUND_POINT_H_

#include "inset_state.h"

bool almost_equal(double, double);
bool points_almost_equal(const Point, const Point);
bool xy_points_almost_equal(const XYPoint, const XYPoint);
double rounded_to_bicimal(const double,
                          const unsigned int,
                          const unsigned int);
Point rounded_point(const Point,
                    const unsigned int,
                    const unsigned int);
XYPoint rounded_XYpoint(const XYPoint,
                        const unsigned int,
                        const unsigned int);

#endif
