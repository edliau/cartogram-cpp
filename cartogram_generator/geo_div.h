#ifndef GEO_DIV_H_
#define GEO_DIV_H_

#include "cgal_typedef.h"
#include <string>
#include <vector>

class GeoDiv {
  private:
    std::string id_;
    std::vector<Polygon_with_holes> polygons_with_holes_;
    std::set<std::string> adjacent_geodivs_;
    GeoDiv();

  public:
    explicit GeoDiv(const std::string);
    const std::set<std::string> adjacent_geodivs() const;
    void adjacent_to(const std::string);
    double area() const;
    double area_longlat() const;
    const std::string id() const;
    unsigned int n_points() const;
    unsigned int n_polygons_with_holes() const;
    unsigned int n_rings() const;
    const std::vector<Polygon_with_holes> polygons_with_holes() const;
    void push_back(const Polygon_with_holes);
    std::vector<Polygon_with_holes> *ref_to_polygons_with_holes();
};

#endif
