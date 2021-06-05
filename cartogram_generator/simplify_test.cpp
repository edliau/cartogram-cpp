#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyline_simplification_2/simplify.h>

#include <iostream>
#include <string>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Point_2<K> Point;
typedef std::vector<Point> Polyline;
namespace PS = CGAL::Polyline_simplification_2;
typedef CGAL::Constrained_triangulation_plus_2<CDT> CT;
typedef PS::Stop_below_count_ratio_threshold Stop;
typedef PS::Squared_distance_cost Cost;

void print_coords(Polyline polyline) {
  std::cout << std::endl;
  std::cout << "Simplified coordinates:" << std::endl << std::endl;

  // Print out the simplified coordinates
  for (Point coord : polyline) {
    std::cout << "[ ";
    std::cout << std::fixed << coord.x() / 10000;
    std::cout << ", ";
    std::cout << std::fixed << coord.y() / 10000;
    std::cout << " ]";
    std::cout << ",";
  }

  // Repeat the first coordinate as per the GeoJSON specifications
  std::cout << "[ ";
  std::cout << std::fixed << polyline[0].x() / 10000;
  std::cout << ", ";
  std::cout << std::fixed << polyline[0].y() / 10000;
  std::cout << " ]";
  std::cout << std::endl << std::endl;
}

void simplify_test() {
  // Hard code a minimum working example where running PS::simplify results in
  // self-intersections
  std::vector<std::vector<double>> coords = {
      {122594.84, 2947155.34},
      {122291.65, 2947226.15},
      // commenting out the line below results in no intersections
      {122231.38, 2947042.48},
      {122288.90, 2947133.50},
      {122399.96, 2947130.21},
      {122397.21, 2947037.56},
      {122200.50, 2946951.09},
      {121955.42, 2947081.58},
      {122049.32, 2947449.07},
      {122795.19, 2947365.13},
      {122791.54, 2947241.81},
      {122594.84, 2947155.34}};

  // Create polyline for simplifying later
  Polyline polyline;

  // Insert coordinates into polyline
  for (std::vector<double> coord : coords) {
    Point pt(coord[0], coord[1]);
    polyline.push_back(pt);
  }

  // Insert polyline into ct and run simplify()
  CT ct;
  ct.insert_constraint(polyline.begin(), polyline.end());
  PS::simplify(ct, Cost(), Stop(0.2));
  Polyline polyline_simplified;

  // Insert simplified coordinates into polyline for easy handling
  auto cit = ct.constraints_begin();
  for (auto vit = ct.points_in_constraint_begin(*cit);
       vit != ct.points_in_constraint_end(*cit); vit++) {
    polyline_simplified.push_back(*vit);
  }

  print_coords(polyline);
}
