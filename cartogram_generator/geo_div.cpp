#include "geo_div.h"
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
  for (auto pwh : polygons_with_holes()) {
    Polygon ext_ring = pwh.outer_boundary();
    a += ext_ring.area();
    for (auto hci = pwh.holes_begin(); hci != pwh.holes_end(); ++hci) {
      Polygon hole = *hci;
      a += hole.area();
    }
  }
  return a;
}

const std::string GeoDiv::id() const
{
  return id_;
}

unsigned int GeoDiv::n_points() const
{
  unsigned int n_points = 0;
  for (Polygon_with_holes pgn_wh : polygons_with_holes_) {
    Polygon outer = pgn_wh.outer_boundary();
    n_points += outer.size();

    std::vector<Polygon> holes_v(pgn_wh.holes_begin(), pgn_wh.holes_end());
    for (Polygon hole : holes_v) {
      n_points += hole.size();
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

const std::vector<Polygon_with_holes> GeoDiv::polygons_with_holes() const
{
  return polygons_with_holes_;
}

void GeoDiv::push_back(const Polygon_with_holes pgn_wh)
{
  polygons_with_holes_.push_back(pgn_wh);
  return;
}

std::vector<Polygon_with_holes> *GeoDiv::ref_to_polygons_with_holes()
{
  return &polygons_with_holes_;
}

const Bbox GeoDiv::bbox() const
{
  double gd_xmin = dbl_inf;
  double gd_xmax = -dbl_inf;
  double gd_ymin = dbl_inf;
  double gd_ymax = -dbl_inf;
  for (const auto &pgnwh : polygons_with_holes_) {
    const Bbox pgnwh_bbox = pgnwh.bbox();
    gd_xmin = std::min(pgnwh_bbox.xmin(), gd_xmin);
    gd_ymin = std::min(pgnwh_bbox.ymin(), gd_ymin);
    gd_xmax = std::max(pgnwh_bbox.xmax(), gd_xmax);
    gd_ymax = std::max(pgnwh_bbox.ymax(), gd_ymax);
  }
  Bbox gd_bb(gd_xmin, gd_ymin, gd_xmax, gd_ymax);
  return gd_bb;
}
