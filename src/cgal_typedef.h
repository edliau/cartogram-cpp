#ifndef CGAL_TYPEDEF_H_
#define CGAL_TYPEDEF_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Polyline_simplification_2/simplify.h>
#include <CGAL/Polygon_set_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Epick;
typedef CGAL::Polygon_2<Epick> Polygon;
typedef CGAL::Polygon_with_holes_2<Epick> Polygon_with_holes;
typedef CGAL::Aff_transformation_2<Epick> Transformation;
typedef CGAL::Point_2<Epick> Point;
typedef CGAL::Bbox_2 Bbox;
typedef CGAL::Segment_2<Epick> Segment;
typedef CGAL::Polygon_set_2<Epick> Polygon_set;

// Polyline simplification
namespace PS = CGAL::Polyline_simplification_2;
typedef PS::Vertex_base_2<Epick> Vb;
typedef CGAL::Constrained_triangulation_face_base_2<Epick> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb> TDS;
typedef CGAL::Constrained_Delaunay_triangulation_2<
    Epick,
    TDS,
    CGAL::Exact_predicates_tag
    > CDT;
typedef CGAL::Constrained_triangulation_plus_2<CDT> CT;
typedef PS::Stop_below_count_ratio_threshold Stop;
typedef PS::Squared_distance_cost Cost;

#endif
