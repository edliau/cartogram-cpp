#include "geo_div.h"
#include "xy_point.h"
#include "constants.h"

GeoDiv::GeoDiv(const std::string i) : id_(i)
{
  return;
}

const std::set<std::string> GeoDiv::adjacent_geodivs() const
{
  return adjacent_geodivs_;
}

void GeoDiv::adjacent_to(const std::string id)
{
  adjacent_geodivs_.insert(id);
}

double GeoDiv::area() const
{
  double a = 0.0;
  for (const auto &pwh : polygons_with_holes()) {
    const auto ext_ring = pwh.outer_boundary();
    a += ext_ring.area();
    for (auto h = pwh.holes_begin(); h != pwh.holes_end(); ++h) {
      a += (*h).area();
    }
  }
  return a;
}

const std::string GeoDiv::id() const
{
  return id_;
}

const Polygon_with_holes GeoDiv::largest_polygon_with_holes() const
{
  double max_area = -dbl_inf;
  Polygon_with_holes largest_pwh;
  for (const auto &pwh : polygons_with_holes_) {
    double area = 0.0;
    const auto ext_ring = pwh.outer_boundary();
    area += ext_ring.area();
    for (auto h = pwh.holes_begin(); h != pwh.holes_end(); ++h) {
      area += (*h).area();
    }
    if (area > max_area) {
      max_area = area;
      largest_pwh = Polygon_with_holes(pwh);
    }
  }
  return largest_pwh;
}

unsigned int GeoDiv::n_points() const
{
  unsigned int n_points = 0;
  for (const auto &pwh : polygons_with_holes_) {
    const auto outer = pwh.outer_boundary();
    n_points += outer.size();
    for (auto h = pwh.holes_begin(); h != pwh.holes_end(); ++h) {
      n_points += (*h).size();
    }
  }
  return n_points;
}

unsigned int GeoDiv::n_polygons_with_holes() const
{
  return polygons_with_holes_.size();
}

unsigned int GeoDiv::n_rings() const
{
  unsigned int n_rings = 0;
  for (const auto &pwh : polygons_with_holes_) {
    n_rings += pwh.number_of_holes() + 1;  // Add 1 for external ring
  }
  return n_rings;
}

// TODO: THIS IS NOT THE USUAL DEFINITION OF point_on_surface(). INSTEAD OF
//       THE LARGEST POLYGON WITH HOLES, THE LARGEST LINE SEGMENT IN ANY
//       POLYGON WITH HOLES IN THE MULTIPOLYGON IS CHOSEN.
Point GeoDiv::point_on_surface_of_geodiv() const
{
  return point_on_surface_of_polygon_with_holes(largest_polygon_with_holes());
}

// Function that takes a Polygon_with_holes and returns the midpoint of the
// longest line segment that is inside the polygon and is halfway through the
// northern and southern tip of the polygon. Reference:
// https://gis.stackexchange.com/questions/76498/how-is-st-pointonsurface-
// calculated
Point GeoDiv::point_on_surface_of_polygon_with_holes(
  const Polygon_with_holes pwh) const
{
  const auto bb = pwh.bbox();
  const double line_y = (bb.ymin() + bb.ymax()) / 2;
  const double epsilon = 1e-6 / default_max_n_graticule_rows_or_cols;

  // Vector to store intersections
  std::vector<intersection> intersections;
  add_intersections(intersections,
                    pwh.outer_boundary(),
                    line_y,
                    0,
                    epsilon,
                    id_,
                    'x');

  // Store hole intersections
  for (auto hci = pwh.holes_begin(); hci != pwh.holes_end(); ++hci) {
    add_intersections(intersections,
                    *hci,
                    line_y,
                    0,
                    epsilon,
                    id_,
                    'x');
  }
  std::sort(intersections.begin(), intersections.end());

  // Assign directions (i.e., whether the line is entering or leaving the
  // polygon with holes)
  for (unsigned int i = 0; i < intersections.size(); ++i) {
    intersections[i].ray_enters = (i%2 == 0);
  }

  // TODO: USING target_density WHEN WE REALLY MEAN LINE LENGTH FEELS LIKE A
  // BAD HACK. SHOULD WE RENAME THE DATA MEMBER target_density TO
  // value_in_geo_div?
  // Assign length of line segments using the target_density property of
  // intersections for line segment lengths
  for (unsigned int i = 0; i < intersections.size(); i += 2) {
    intersections[i].target_density =
      intersections[i + 1].x() - intersections[i].x();
  }

  // Find midpoint in maximum segment length
  double max_length = 0.0;
  XYPoint midpoint;
  midpoint.y = line_y;

  // Iterate over lengths
  for (unsigned int i = 0; i < intersections.size(); i += 2) { \
    if (intersections[i].target_density > max_length) {
      const double left = intersections[i].x();
      const double right = intersections[i + 1].x();
      max_length = intersections[i].target_density;
      midpoint.x = (right + left) / 2;
    }
  }
  return Point(midpoint.x, midpoint.y);
}

const std::vector<Polygon_with_holes> GeoDiv::polygons_with_holes() const
{
  return polygons_with_holes_;
}

void GeoDiv::push_back(const Polygon_with_holes pwh)
{
  polygons_with_holes_.push_back(pwh);
  return;
}

std::vector<Polygon_with_holes> *GeoDiv::ref_to_polygons_with_holes()
{
  return &polygons_with_holes_;
}

Bbox GeoDiv::bbox() const
{
  // Find joint bounding for all polygons with holes in this geo_div
  double geo_div_xmin = dbl_inf;
  double geo_div_xmax = -dbl_inf;
  double geo_div_ymin = dbl_inf;
  double geo_div_ymax = -dbl_inf;
  for (const auto &pwh : polygons_with_holes_) {
    const auto bb = pwh.bbox();
    geo_div_xmin = std::min(bb.xmin(), geo_div_xmin);
    geo_div_ymin = std::min(bb.ymin(), geo_div_ymin);
    geo_div_xmax = std::max(bb.xmax(), geo_div_xmax);
    geo_div_ymax = std::max(bb.ymax(), geo_div_ymax);
  }
  Bbox geo_div_bbox(geo_div_xmin, geo_div_ymin, geo_div_xmax, geo_div_ymax);
  return geo_div_bbox;
}
