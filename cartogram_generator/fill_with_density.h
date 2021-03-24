#ifndef FILL_WITH_DENSITY_H_
#define FILL_WITH_DENSITY_H_

#include <string.h>

// Struct to store intersection data
struct intersection {
  double x;  // x-coordinate of intersection
  double target_density;  // GeoDiv's target_density
  std::string geo_div_id;
  bool direction;  // Does intersection enter or exit?

  // Overload "<" operator for this data type. Idea from
  // https://stackoverflow.com/questions/4892680/sorting-a-vector-of-structs
  bool operator < (const intersection &rhs) const
  {
    return (x < rhs.x || (x == rhs.x && direction < rhs.direction));
  }
};

void fill_with_density(MapState*);
bool line_y_intersects(XYPoint,
                       XYPoint,
                       double,
                       intersection *,
                       double,
                       double);
XYPoint point_on_surface(Polygon_with_holes);

#endif
