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

  // Print out the simplified coordinates
  for (Point coord : polyline) {
    std::cout << "[ ";
    std::cout << std::fixed << coord.x();
    std::cout << ", ";
    std::cout << std::fixed << coord.y();
    std::cout << " ]";
    std::cout << ",";
  }

  // Repeat the first coordinate as per the GeoJSON specifications
  std::cout << "[ ";
  std::cout << std::fixed << polyline[0].x();
  std::cout << ", ";
  std::cout << std::fixed << polyline[0].y();
  std::cout << " ]";
  std::cout << std::endl;
}

void simplify_test() {
  // Hard code a minimum working example where running PS::simplify results in
  // self-intersections
  std::vector<std::vector<double>> coords = {
      {122594.842416608473286, 2947155.341584724839777},
      {122291.654872600804083, 2947226.153077695984393},
      // commenting out the line below results in no intersections
      {122231.389713566517457, 2947042.483519087545574},
      {122288.905913269147277, 2947133.505765041336417},
      {122399.960992575390264, 2947130.211158112622797},
      {122397.212925524567254, 2947037.563923422247171},
      {122200.508536974317394, 2946951.094404897652566},
      {121955.427253245317843, 2947081.585233204998076},
      {122049.323379306704737, 2947449.078677589073777},
      {122795.194644965638872, 2947365.134323424659669},
      {122791.540339241153561, 2947241.816514134872705},
      {122594.842416608473286, 2947155.341584724839777}};

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

  print_coords(polyline_simplified);
}
