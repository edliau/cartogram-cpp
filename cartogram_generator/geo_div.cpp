#include "geo_div.h"
#include <cmath>

GeoDiv::GeoDiv(const std::string i) : id_(i)
{
  return;
}

const std::string GeoDiv::id() const
{
  return id_;
}

int GeoDiv::n_polygons_with_holes() const
{
  return polygons_with_holes_.size();
}

const std::vector<Polygon_with_holes> GeoDiv::polygons_with_holes() const
{
  return polygons_with_holes_;
}

std::vector<Polygon_with_holes> *GeoDiv::ref_to_polygons_with_holes()
{
  return &polygons_with_holes_;
}

void GeoDiv::push_back(const Polygon_with_holes pgn_wh)
{
  polygons_with_holes_.push_back(pgn_wh);
  return;
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

double GeoDiv::perimeter() const
{
  double p = 0.0;
  for (auto pwh : polygons_with_holes()){
    Polygon ext_ring = pwh.outer_boundary();
    for (unsigned int i = 0; i < ext_ring.size() - 1; i++){
      double distance = std::sqrt(
        (ext_ring[i + 1][0] - ext_ring[i][0]) * (ext_ring[i + 1][0] - ext_ring[i][0]) +
        (ext_ring[i + 1][1] - ext_ring[i][1]) * (ext_ring[i + 1][1] - ext_ring[i][1])
      );
      p += ((!std::isnan(distance)) ? distance : 0.0);
    }
    for (auto hci = pwh.holes_begin(); hci != pwh.holes_end(); hci++){
      Polygon hole = *hci;
      for (unsigned int i = 0; i < hole.size() - 1; i++){
        double distance = std::sqrt(
          (hole[i + 1][0] - hole[i][0]) * (hole[i + 1][0] - hole[i][0]) +
          (hole[i + 1][1] - hole[i][1]) * (hole[i + 1][1] - hole[i][1])
        );
        p += ((!std::isnan(distance)) ? distance : 0.0);
      }
    }
  }
  return p;
}

void GeoDiv::adjacent_to(const std::string id)
{
  adjacent_geodivs_.insert(id);
}

const std::set<std::string> GeoDiv::adjacent_geodivs() const
{
  return adjacent_geodivs_;
}
