#include "cgal_typedef.h"
#include <functional>

void Polygon_with_holes::for_each_point(std::function<void(Point)>
                                        point_func) const
{

  // Exterior ring
  const Polygon ext_ring = outer_boundary();
  for (const Point p : ext_ring ) {
    point_func(p);
  }

  // Holes
  for (auto h = holes_begin(); h != holes_end(); ++h) {
    const Polygon hole = *h;
    for (const Point p : hole ) {
      point_func(p);
    }
  }
}

void Polygon_with_holes::for_each_pair(std::function<void(Point, Point)>
                                       point_pair_func) const
{

  const Polygon ext_ring = outer_boundary();
  Point prev_point = (*ext_ring.vertices_end());
  for (Point curr_point : ext_ring) {
    point_pair_func(curr_point, prev_point);
    prev_point = curr_point;
  }

  // Holes
  for (auto h = holes_begin(); h != holes_end(); ++h) {
    const Polygon hole = *h;
    Point prev_point = (*hole.vertices_end());
      for (Point curr_point : hole) {
        point_pair_func(curr_point, prev_point);
        prev_point = curr_point;
      }
  }
}
