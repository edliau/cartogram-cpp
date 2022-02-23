#include "cgal_typedef.h"
#include <functional>

void Polygon_with_holes::apply(std::function<Point(Point, Point)> func)
{
  return almost_equal(x(), rhs.x()) && almost_equal(y(), rhs.y());
}

class Polygon_with_holes: public CGAL_Polygon_with_holes
