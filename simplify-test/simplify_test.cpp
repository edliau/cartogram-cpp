#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyline_simplification_2/simplify.h>

#include <iostream>
#include <string>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Point_2<K> Point;
typedef std::vector<Point> Polyline;
namespace PS = CGAL::Polyline_simplification_2;
typedef PS::Vertex_base_2<K> Vb;
typedef CGAL::Constrained_triangulation_face_base_2<K> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb> TDS;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS,
                                                   CGAL::Exact_predicates_tag>
    CDT;
typedef CGAL::Constrained_triangulation_plus_2<CDT> CT;
typedef PS::Stop_below_count_ratio_threshold Stop;
typedef PS::Squared_distance_cost Cost;

void print_coords(Polyline polyline) {
  std::cout << std::endl;
  std::cout << "Simplified coordinates:" << std::endl << std::endl;

  // Print out the simplified coordinates
  unsigned int i = 0;
  for (Point coord : polyline) {
    std::cout << "[ ";
    std::cout << coord.x();
    std::cout << ", ";
    std::cout << coord.y();
    std::cout << " ]";
    if (i != polyline.size() - 1) std::cout << ",";
    i++;
  }
  std::cout << std::endl << std::endl;
}

void simplify_test() {
  // Hard code a minimum working example where running PS::simplify results in
  // self-intersections. There are no self-intersections when {27, 9} is
  // omitted.
  std::vector<std::vector<int>> coords = {
      {64, 20}, {33, 27}, {27, 9}, {33, 18}, {44, 18}, {44, 8},
      {24, 0},  {0, 13},  {9, 49}, {84, 41}, {83, 29}, {64, 20},
  };
  // Simplification outputs:
  // [ 64, 20 ],[ 27, 9 ],[ 44, 18 ],[ 24, 0 ],
  // [ 9, 49 ],[ 83, 29 ],[ 64, 20 ],[ 64, 20 ]

  // Create polyline for simplifying later
  Polyline polyline;

  // Insert coordinates into polyline
  for (std::vector<int> coord : coords) {
    Point pt(coord[0], coord[1]);
    polyline.push_back(pt);
  }

  // Insert polyline into ct and run simplify()
  CT ct;
  ct.insert_constraint(polyline.begin(), polyline.end());
  PS::simplify(ct, Cost(), Stop(0.2));
  Polyline polyline_simplified;

  // Transfer simplified coordinates from ct to polyline for easy handling
  auto cit = ct.constraints_begin();
  for (auto v : ct.vertices_in_constraint(*cit)) {
    polyline_simplified.push_back(v->point());
  }

  print_coords(polyline_simplified);
}

int main() { simplify_test(); }