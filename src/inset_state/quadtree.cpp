#include "cartogram_info.h"
#include "constants.h"
#include <iostream>

// Cairo for drawing and creating .ps files
#include <cairo/cairo-ps.h>
#include <cairo/cairo.h>

// CGAL's Quadtree data structure
#include <CGAL/Orthtree.h>
#include <CGAL/Orthtree_traits_d.h>
#include <CGAL/Quadtree.h>
#include <CGAL/Simple_cartesian.h>

// Delaunay triangulation
#include <CGAL/Delaunay_triangulation_2.h>

// Barycentric Coordinates
#include <CGAL/Barycentric_coordinates_2/Triangle_coordinates_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Triangle_2.h>
#include <CGAL/intersections.h>

// Creating alias: type definitions to make code more readable.
typedef CGAL::Simple_cartesian<double>
  Kernel;  // operating on 2d cartesian surface
typedef Kernel::Point_2 Point_2;  // data structure for 2d point - use .x() and
                                  // .y() method to access stored data
typedef std::vector<Point_2>
  Point_vector;  // a vector for Point_2 data structure
typedef CGAL::Orthtree_traits_2<Kernel>
  Traits;  // Traits define additional properties for quadtree.
typedef CGAL::Orthtree<Traits, Point_vector> Orthtree;  // Orthtree alias

// Delaunay Triangulation using Cartesian Kernel
typedef CGAL::Delaunay_triangulation_2<Kernel> Delaunay;

// Cpp Chrono for timing
typedef std::chrono::steady_clock::time_point time_point;
typedef std::chrono::steady_clock clock_time;
typedef std::chrono::milliseconds ms;

template <typename T> std::chrono::milliseconds inMilliseconds(T duration)
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}

struct color {
  double r, g, b;
};

// Helper functions

// Adds header information to the ps files.
void write_ps_header(std::string, cairo_surface_t *);

// Given a Orthtree object, creates a .ps file for visualization.
void draw_QuadTree(
  Orthtree &,
  std::string,
  unsigned int,
  unsigned int,
  std::unordered_set<Point_2> = std::unordered_set<Point_2>());

// Creates point vector from the given map from the command line
Point_vector get_cartogram_points(CartogramInfo *);

// Quadtree operations
void quadtree_ds(CartogramInfo *);

// Delaunay operations
void delaunay_ds(CartogramInfo *);

void test(CartogramInfo *);

void corners_from_polygon(
  const Polygon &,
  std::unordered_set<Point_2> &,
  const Delaunay &);

void triangles_to_cairo(cairo_t *, Delaunay &, color = color{0, 0, 0});

void corners_to_cairo(
  cairo_t *,
  const std::unordered_set<Point_2> &,
  color = color{1, 0, 0});

void nodes_to_cairo(cairo_t *, const Orthtree &, color = color{0, 0, 0});

void polygon_points_to_cairo(
  cairo_t *,
  const Orthtree &,
  color = color{0, 0, 1});

void quadtree_tutorial(CartogramInfo *cartogram_info)
{

  test(cartogram_info);
  // Comment out one of the following lines when working with the another to
  // have clearer terminal output quadtree_ds(cartogram_info);
  // delaunay_ds(cartogram_info);
}

void corners_from_polygon(
  const Polygon &ring,
  std::unordered_set<Point_2> &corners,
  const Delaunay &dt)
{
  for (unsigned int i = 0; i < ring.size() - 1; ++i) {

    Point_2 p1(ring[i][0], ring[i][1]);
    Point_2 p2(ring[i + 1][0], ring[i + 1][1]);

    if (p1 == p2)
      continue;

    // If they are inside the same triangle then continue
    Delaunay::Face_handle f1 = dt.locate(p1);
    Delaunay::Face_handle f2 = dt.locate(p2);

    if (f1 == f2)
      continue;

    bool f1_first, found_first_face = false, found_second_face = false;

    Kernel::Segment_2 segment(p1, p2);

    Delaunay::Line_face_circulator lfc = dt.line_walk(p1, p2);

    Delaunay::Line_face_circulator lfc_begin =
      lfc;  // we store the begining iterator

    if (lfc_begin != 0)
      do {
        Delaunay::Face_handle fh = lfc;

        if (found_first_face) {
          if (f1_first) {
            if (fh == f2)
              found_second_face = true;
          } else {
            if (fh == f1)
              found_second_face = true;
          }
        } else {
          if (fh == f1) {
            f1_first = true;
            found_first_face = true;
          } else if (fh == f2) {
            f1_first = false;
            found_first_face = true;
          } else {
            ++lfc;
            continue;
          }
        }

        Kernel::Triangle_2 triangle(
          fh->vertex(0)->point(),
          fh->vertex(1)->point(),
          fh->vertex(2)->point());

        // create three segments from the triangle
        Kernel::Segment_2 s1(triangle.vertex(0), triangle.vertex(1));
        Kernel::Segment_2 s2(triangle.vertex(1), triangle.vertex(2));
        Kernel::Segment_2 s3(triangle.vertex(2), triangle.vertex(0));

        // check intersection with segment
        if (CGAL::do_intersect(segment, s1)) {
          Kernel::Point_2 p2;
          CGAL::Object p = CGAL::intersection(segment, s1);
          if (CGAL::assign(p2, p)) {
            corners.insert(p2);
          }
        }
        if (CGAL::do_intersect(segment, s2)) {
          // insert intersected point
          Kernel::Point_2 p2;
          CGAL::Object p = CGAL::intersection(segment, s2);
          if (CGAL::assign(p2, p)) {
            corners.insert(p2);
          }
        }
        if (CGAL::do_intersect(segment, s3)) {
          // insert intersected point
          Kernel::Point_2 p2;
          CGAL::Object p = CGAL::intersection(segment, s3);
          if (CGAL::assign(p2, p)) {
            corners.insert(p2);
          }
        }

        // move the iterator to the next one
        ++lfc;
      } while (lfc != lfc_begin && !found_second_face);
  }
}

void test(CartogramInfo *cartogram_info)
{
  // Create a quadtree from the given points
  Point_vector points = get_cartogram_points(cartogram_info);

  points.push_back(Point_2(0, 0));
  points.push_back(Point_2(0, 512));
  points.push_back(Point_2(512, 0));
  points.push_back(Point_2(512, 512));

  Orthtree quadtree(points, Orthtree::PointMap(), 1);

  // add four bounding box points
  quadtree.refine(10, 20);
  quadtree.grade();

  // Getting total number of cell(leaf nodes) and unique corner points
  std::unordered_set<Point_2> corners;  // set will remove the duplicates
  // corners set can be used as a input to dalaunay triangulation
  int n_nodes = 0;
  for (Orthtree::Node &node :
       quadtree.traverse<CGAL::Orthtrees::Preorder_traversal>()) {
    if (node.is_leaf()) {
      n_nodes++;
      auto bbox = quadtree.bbox(node);  // Get bbox of the node

      // Insert the four vertex of the bbox into the corners set
      corners.insert(Point_2(bbox.xmin(), bbox.ymin()));
      corners.insert(Point_2(bbox.xmax(), bbox.ymax()));
      corners.insert(Point_2(bbox.xmin(), bbox.ymax()));
      corners.insert(Point_2(bbox.xmax(), bbox.ymin()));
    }
  }

  Delaunay dt;
  dt.insert(corners.begin(), corners.end());

  time_point start = clock_time::now();

  // Getting densification points
  for (auto &[inset_pos, inset_state] :
       *cartogram_info->ref_to_inset_states()) {
    for (auto gd : inset_state.geo_divs()) {
      for (auto pwh : gd.polygons_with_holes()) {
        corners_from_polygon(pwh.outer_boundary(), corners, dt);
        for (auto hci = pwh.holes_begin(); hci != pwh.holes_end(); ++hci) {
          corners_from_polygon(*hci, corners, dt);
        }
      }
    }
  }

  time_point end = clock_time::now();
  ms duration = inMilliseconds(end - start);
  std::cout << "Time taken for densification Point " << points.size()
            << " points: " << duration.count() << " ms" << std::endl;

  Delaunay dt2;
  dt2.insert(corners.begin(), corners.end());

  // print number of triangles
  std::cout << "Number of triangles: " << dt2.number_of_faces() << std::endl;

  cairo_surface_t *surface = cairo_ps_surface_create(
    (cartogram_info->map_name() + "_delaunay.ps").c_str(),
    512,
    512);
  cairo_t *cr = cairo_create(surface);
  // write header
  write_ps_header("dddelaunay.ps", surface);

  // Draw the triangles
  triangles_to_cairo(cr, dt);

  nodes_to_cairo(cr, quadtree);

  polygon_points_to_cairo(cr, quadtree);

  corners_to_cairo(cr, corners);

  cairo_stroke(cr);
  cairo_show_page(cr);
  cairo_surface_destroy(surface);
  cairo_destroy(cr);
}

void delaunay_ds(CartogramInfo *cartogram_info)
{

  std::vector<Point_2> points;

  for (std::size_t i = 1; i <= 10; ++i) {
    points.push_back(Point_2(i, 2 * i * i - 3 * i + 3));
  }

  // Uncomment the following line to create delaunay triangulation from map
  // vertices. points = get_cartogram_points(cartogram_info);

  // Adding points: Complexity: O(1) per insertion
  Delaunay dt;
  dt.insert(points.begin(), points.end());

  std::cout << "Delaunay triangulation created." << std::endl;
  std::cout << "Number of Verticies: " << dt.number_of_vertices() << std::endl;
  std::cout << "Number of triangles: " << dt.number_of_faces() << std::endl;

  // Print the points/vertices
  for (Delaunay::Finite_vertices_iterator vit = dt.finite_vertices_begin();
       vit != dt.finite_vertices_end();
       ++vit) {
    std::cout << "Point: " << vit->point() << std::endl;
  }

  // Print the triangles (three vertex coordinate)
  for (Delaunay::Finite_faces_iterator fit = dt.finite_faces_begin();
       fit != dt.finite_faces_end();
       ++fit) {
    std::cout << "Triangle: " << fit->vertex(0)->point() << " \t\t"
              << fit->vertex(1)->point() << "\t\t " << fit->vertex(2)->point()
              << std::endl;
  }

  // Print the segments (all two points of triangles)
  for (Delaunay::Finite_edges_iterator eit = dt.finite_edges_begin();
       eit != dt.finite_edges_end();
       ++eit) {
    std::cout << "Segment: " << eit->first->vertex(eit->second)->point()
              << " \t\t" << std::endl;
  }

  // Locate a point: Worst case O(n): average O(sqrt(n))
  Delaunay::Face_handle face = dt.locate(Point_2(1, 1));
  std::cout << "Locating: (1,1)" << std::endl;
  std::cout << "(1,1) belongs to triangle: " << face->vertex(0)->point()
            << " \t\t" << face->vertex(1)->point() << "\t\t "
            << face->vertex(2)->point() << std::endl;

  //************************************* Line Walk
  //*************************************

  // Get triangles that intersect the segment of two points
  Delaunay::Line_face_circulator lfc =
    dt.line_walk(points[0], points[5]);  // arbitary points

  Delaunay::Line_face_circulator lfc_begin =
    lfc;  // we store the begining iterator

  do {
    Delaunay::Face_handle fh = lfc;  // get the face

    // Now we can apply normal face methods to fh like vertex
    std::cout << "Segment intersects triangle: " << fh->vertex(0)->point()
              << " \t\t" << fh->vertex(1)->point() << "\t\t "
              << fh->vertex(2)->point() << std::endl;

    // move the iterator to the next one
    ++lfc;
  } while (lfc !=
           lfc_begin);  // we stop when we get back to the first iterator;
  // since this is a circulator there is no end or beginning, we keep track of
  // our begining iterator, and when we get back to it we know we have
  // traversed all the intersected face handles.

  //************************************* Locate Time Testing
  //*************************************
  // Measure time required for locating points
  time_point start = clock_time::now();

  for (auto pt : points) {
    dt.locate(pt);
  }

  time_point end = clock_time::now();
  ms duration = inMilliseconds(end - start);
  std::cout << "Time taken to locate all " << points.size()
            << " points: " << duration.count() << " ms" << std::endl;

  // ******************************* Barycentric Coordinate
  // ******************************************** We first take a face handle/
  // or any three triangle points would also be enough
  Delaunay::Face_handle face_b = dt.locate(Point_2(1, 1));

  // ************************************** CGAL 5.3
  // ****************************************************
  //    // We will store the barycentric coordinate here
  //     std::vector<double> barycentric_coordinates;

  //     barycentric_coordinates.reserve(3);

  //     // We now use the CGAL function to compute the barycentric coordinate
  //     CGAL::Barycentric_coordinates::Triangle_coordinates_2<Kernel>
  //     bary(face_b->vertex(0)->point(), face_b->vertex(1)->point(),
  //     face_b->vertex(2)->point());

  //     // Now give our point inside that triangle to get that points
  //     barycentric coordinate
  //     // the calculated barycentric coordinate will be stored inside the
  //     vector barycentric_coordinate bary(Point_2(1, 1),
  //     std::back_inserter(barycentric_coordinates));

  //     // Now we retrive the barycentric coordinates just by iterating over
  //     the vector std::cout << "Barycentric coordinate: "; for(auto bc:
  //     barycentric_coordinates) {
  //         std::cout << bc << "\t\t";
  //     } // if you get different result each time you rerun, do not worry.
  //     Barrycenter
  //     // function is alright. The issue is with the face_b vertex values.
  //     The vertex values
  //     // are different each time we rerun the program.

  // ************************************** CGAL 5.4
  // ****************************************************

  // It is possible to use a vector and use the backinserter like CGAL 5.3
  // example but that might be redundant considering the direction of the
  // project.
  std::tuple<Kernel::FT, Kernel::FT, Kernel::FT> bary_coor;

  bary_coor =
    CGAL::Barycentric_coordinates::triangle_coordinates_in_tuple_2<Point_2>(
      face_b->vertex(0)->point(),
      face_b->vertex(1)->point(),
      face_b->vertex(2)->point(),
      Point_2(1, 1));  // three triangle vertices, and then query point

  std::cout << "Barycentric coordinate: " << std::get<0>(bary_coor) << "\t\t"
            << std::get<1>(bary_coor) << "\t\t" << std::get<2>(bary_coor)
            << std::endl;  // using get to access tuple elements
  // if you get different result each time you rerun, do not worry. Barrycenter
  // function is alright. The issue is with the face_b vertex values. The
  // vertex values are different each time we rerun the program.

  // *********************** Part below is to draw delaunay trianglation using
  // Cairo ****************************** NOTE: Make sure to uncomment line 65
  // to have meaningful postscript file output Draw the dt on cairo surface
  cairo_surface_t *surface = cairo_ps_surface_create("delaunay.ps", 512, 512);
  cairo_t *cr = cairo_create(surface);

  // write header
  write_ps_header("delaunay.ps", surface);

  // Draw the triangles
  for (Delaunay::Finite_faces_iterator fit = dt.finite_faces_begin();
       fit != dt.finite_faces_end();
       ++fit) {
    Point_2 p1 = fit->vertex(0)->point();
    Point_2 p2 = fit->vertex(1)->point();
    Point_2 p3 = fit->vertex(2)->point();

    // set width of line
    cairo_set_line_width(cr, 0.05);

    cairo_move_to(cr, p1.x(), p1.y());
    cairo_line_to(cr, p2.x(), p2.y());
    cairo_line_to(cr, p3.x(), p3.y());
    cairo_line_to(cr, p1.x(), p1.y());
    cairo_stroke(cr);
  }

  cairo_stroke(cr);
  cairo_show_page(cr);
  cairo_surface_destroy(surface);
  cairo_destroy(cr);
}

void quadtree_ds(CartogramInfo *cartogram_info)
{
  Point_vector points_2d;

  // Add x = y line points from (1, 1) to (152, 152) to the points_2d vector
  for (std::size_t i = 1; i <= 152; ++i) {
    points_2d.emplace_back(Point_2(i, i));
  }

  // Create a quadtree
  Orthtree quadtree(points_2d, Orthtree::PointMap(), 1);

  // Reshapes the quadtree based on (depth, max points in a node before split)
  quadtree.refine(10, 3);  // default: (10, 20)

  // refines the orthtree such that the difference of depth
  // between two immediate neighbor leaves is never more than 1.
  quadtree.grade();

  // Uncommnet the following line to print the whole quadtree on the terminal
  // std::cout << quadtree << std::endl;

  // Draw the qudtree
  draw_QuadTree(
    quadtree,
    "toymodel.ps",
    150,
    150);  // 150, 150 are the (x, y) dimension of the canvas

  // ************************ Methods of quadtree
  // *******************************

  // Get root node
  std::cout << "Root: " << quadtree.root() << std::endl;

  // Getting total number of cell(leaf nodes) and unique corner points
  std::unordered_set<Point_2> corners;  // set will remove the duplicates
  // corners set can be used as a input to dalaunay triangulation
  int n_nodes = 0;
  for (Orthtree::Node &node :
       quadtree.traverse<CGAL::Orthtrees::Preorder_traversal>()) {
    if (node.is_leaf()) {
      n_nodes++;
      auto bbox = quadtree.bbox(node);  // Get bbox of the node

      // Insert the four vertex of the bbox into the corners set
      corners.insert(Point_2(bbox.xmin(), bbox.ymin()));
      corners.insert(Point_2(bbox.xmax(), bbox.ymax()));
      corners.insert(Point_2(bbox.xmin(), bbox.ymax()));
      corners.insert(Point_2(bbox.xmax(), bbox.ymin()));
    }
  }

  std::cout << "Number of nodes: " << n_nodes << std::endl;
  std::cout << "Number of Corners: " << corners.size() << std::endl;

  //   |  +-------------------+-------------------+
  // |  | Coord:  Ymax Xmin | Coord:  Ymax Xmax |
  // |  | Bitmap:    1    0 | Bitmap:    1    1 |
  // |  |                   |                   |
  // |  | -> index = 2      | -> index = 3      |
  // |  |                   |                   |
  // |  |                   |                   |
  // |  |                   |                   |
  // |  |                   |                   |
  // |  +-------------------+-------------------+
  // |  | Coord:  Ymin Xmin | Coord:  Ymin Xmax |
  // |  | Bitmap:    0    0 | Bitmap:    0    1 |
  // |  |                   |                   |
  // |  | -> index = 0      | -> index = 1      |
  // |  |                   |                   |
  // |  |                   |                   |
  // |  |                   |                   |
  // |  |                   |                   |
  // |  +-------------------+-------------------+

  // We can index and get four of its children
  std::cout << "First Child Root: " << quadtree.root()[0] << std::endl;
  std::cout << "Second Child Root: " << quadtree.root()[1] << std::endl;
  std::cout << "Third Child Root: " << quadtree.root()[2] << std::endl;
  std::cout << "Fourth Child Root: " << quadtree.root()[3] << std::endl;

  // Four child of first child of root
  std::cout << "First Child Root First Child: " << quadtree.root()[0][0]
            << std::endl;
  std::cout << "Second Child Root First Child: " << quadtree.root()[0][1]
            << std::endl;
  std::cout << "Third Child Root First Child: " << quadtree.root()[0][2]
            << std::endl;
  std::cout << "Fourth Child Root First Child: " << quadtree.root()[0][3]
            << std::endl;

  // Get parent of a node
  std::cout << "Parent of First Child Root First Child: "
            << quadtree.root()[0][0].parent() << std::endl;

  // Get bbox using .bbox(Node Obj) method
  std::cout << "Bbox of the tree" << std::endl;
  std::cout << quadtree.bbox(quadtree.root()[0][0]) << std::endl;

  std::cout << "Maximum depth of the tree: " << quadtree.depth() << std::endl;

  // Locate node based on points
  Orthtree::Node node =
    quadtree.locate(Point_2(30, 30));  // returns the node object
  std::cout << "Node located at (30, 30): " << node << std::endl;
  std::cout << quadtree.bbox(node) << std::endl;

  // Adjacent nodes
  // +---------------+---------------+
  // |               |               |
  // |               |               |
  // |               |               |
  // |       A       |               |
  // |               |               |
  // |               |               |
  // |               |               |
  // +-------+-------+---+---+-------+
  // |       |       |   |   |       |
  // |   A   |  (S)  +---A---+       |
  // |       |       |   |   |       |
  // +---+---+-------+---+---+-------+
  // |   |   |       |       |       |
  // +---+---+   A   |       |       |
  // |   |   |       |       |       |
  // +---+---+-------+-------+-------+
  std::cout << "Adjacent node: " << std::endl;
  // 00 - negative x & negative y
  // 01 - negative x & positive y
  std::cout << node.adjacent_node(01) << std::endl;
  std::cout << node.adjacent_node(10) << std::endl;
  std::cout << node.adjacent_node(11) << std::endl;
  std::cout << node.adjacent_node(00) << std::endl;

  // ************************ Methods of node *******************************

  std::cout << "Node Methods: " << std::endl;
  std::cout << "Node: " << node << std::endl;
  std::cout << "Is Null: " << node.is_null() << std::endl;
  std::cout << "Is Leaf: " << node.is_leaf() << std::endl;
  std::cout << "Is Root: " << node.is_root() << std::endl;
  std::cout << "Depth: " << node.depth() << std::endl;
  std::cout << "Empty: " << node.empty() << std::endl;
  std::cout << "Size: " << node.size()
            << std::endl;  // number of points in the node
  std::cout << "Local Coordinates: " << node.local_coordinates()
            << std::endl;  // relative to parent

  // parent's global coorindates * 2 + node's local coordinate
  std::cout << "Global Coordinates: " << node.global_coordinates()[0] << " "
            << node.global_coordinates()[1] << std::endl;

  // Create a quadtree for the input given map
  Point_vector points = get_cartogram_points(cartogram_info);
  Orthtree quadtree_input_map(points, Orthtree::PointMap(), 1);
  quadtree_input_map.refine(10, 20);
  quadtree_input_map.grade();

  draw_QuadTree(
    quadtree_input_map,
    cartogram_info->map_name() + "_quadtree.ps",
    512,
    512);
}

// Creates point vector for the map data given in the command line
Point_vector get_cartogram_points(CartogramInfo *cart_info)
{
  Point_vector points_2d;
  for (auto &[inset_pos, inset_state] : *cart_info->ref_to_inset_states()) {
    for (auto gd : inset_state.geo_divs()) {
      for (auto pwh : gd.polygons_with_holes()) {
        Polygon ext_ring = pwh.outer_boundary();
        if (cart_info->original_ext_ring_is_clockwise()) {
          ext_ring.reverse_orientation();
        }
        // Get exterior ring coordinates
        for (unsigned int i = 0; i < ext_ring.size(); ++i) {
          double arr[2];
          arr[0] = ext_ring[i][0];
          arr[1] = ext_ring[i][1];
          points_2d.emplace_back(Point_2(arr[0], arr[1]));
        }

        // Get holes of polygon with holes
        for (auto hci = pwh.holes_begin(); hci != pwh.holes_end(); ++hci) {
          Polygon hole = *hci;

          // nlohmann::json hole_container;
          for (unsigned int i = 0; i < hole.size(); ++i) {
            double arr[2];
            arr[0] = hole[i][0];
            arr[1] = hole[i][1];
            points_2d.emplace_back(Point_2(arr[0], arr[1]));
          }
        }
      }
    }
  }
  return points_2d;
}

void write_ps_header(std::string filename, cairo_surface_t *surface)
{
  const std::string title = "%%Title: " + filename;
  cairo_ps_surface_dsc_comment(surface, title.c_str());
  cairo_ps_surface_dsc_comment(
    surface,
    "%%Creator: Michael T. Gastner et al.");
  cairo_ps_surface_dsc_comment(surface, "%%For: Humanity");
  cairo_ps_surface_dsc_comment(surface, "%%Copyright: License CC BY");
  cairo_ps_surface_dsc_comment(surface, "%%Magnification: 1.0000");
}

void draw_QuadTree(
  Orthtree &quadtree,
  std::string filename,
  unsigned int lx_ = 512,
  unsigned int ly_ = 512,
  std::unordered_set<Point_2> corners)
{
  // draw quadtree
  cairo_surface_t *surface =
    cairo_ps_surface_create(filename.c_str(), lx_, ly_);
  cairo_t *cr = cairo_create(surface);
  write_ps_header(filename, surface);

  for (Orthtree::Node &node :
       quadtree.traverse<CGAL::Orthtrees::Preorder_traversal>()) {
    auto bbox = quadtree.bbox(node);
    auto xmin = bbox.xmin();
    auto ymin = bbox.ymin();
    auto xmax = bbox.xmax();
    auto ymax = bbox.ymax();

    cairo_set_line_width(cr, 0.35);

    // draw a rectangle with bbox values
    cairo_rectangle(cr, xmin, ly_ - ymin, xmax - xmin, ymin - ymax);
    cairo_stroke(cr);

    if (node.is_leaf()) {
      for (Point_2 p : node) {
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 0.5);
        cairo_move_to(cr, p.x(), ly_ - p.y());
        cairo_line_to(cr, p.x() + 0.05, ly_ - p.y() - 0.05);
        cairo_close_path(cr);
      }
    }
  }
  for (Point_2 p : corners) {
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    // red color
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_set_line_width(cr, 0.5);
    cairo_move_to(cr, p.x(), ly_ - p.y());
    cairo_line_to(cr, p.x() + 0.05, ly_ - p.y() - 0.05);
    cairo_close_path(cr);
  }
  cairo_stroke(cr);
  cairo_show_page(cr);
  cairo_surface_destroy(surface);
  cairo_destroy(cr);
}

void corners_to_cairo(
  cairo_t *cr,
  const std::unordered_set<Point_2> &corners,
  color clr)
{
  cairo_set_source_rgb(cr, clr.r, clr.g, clr.b);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_width(cr, 0.5);
  for (Point_2 corner : corners) {
    cairo_move_to(cr, corner.x(), 512 - corner.y());
    cairo_line_to(cr, corner.x() + 0.05, 512 - corner.y() - 0.05);
    cairo_close_path(cr);
    cairo_stroke(cr);
  }
}

void nodes_to_cairo(cairo_t *cr, const Orthtree &quadtree, color clr)
{
  // set color
  cairo_set_source_rgb(cr, clr.r, clr.g, clr.b);
  cairo_set_line_width(cr, 0.35);
  for (Orthtree::Node &node :
       quadtree.traverse<CGAL::Orthtrees::Preorder_traversal>()) {
    CGAL::Bbox_2 bbox = quadtree.bbox(node);

    // draw a rectangle with bbox values
    cairo_rectangle(
      cr,
      bbox.xmin(),
      512 - bbox.ymin(),
      bbox.xmax() - bbox.xmin(),
      bbox.ymin() - bbox.ymax());
    cairo_stroke(cr);
  }
}

void polygon_points_to_cairo(cairo_t *cr, const Orthtree &quadtree, color clr)
{
  // set color
  cairo_set_source_rgb(cr, clr.r, clr.g, clr.b);
  for (Orthtree::Node &node :
       quadtree.traverse<CGAL::Orthtrees::Preorder_traversal>()) {

    if (node.is_leaf()) {
      for (Point_2 p : node) {
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_width(cr, 0.5);
        cairo_move_to(cr, p.x(), 512 - p.y());
        cairo_line_to(cr, p.x() + 0.05, 512 - p.y() - 0.05);
        cairo_close_path(cr);
        cairo_stroke(cr);
      }
    }
  }
}

void triangles_to_cairo(cairo_t *cr, Delaunay &dt, color clr)
{
  // Draw the triangles
  for (Delaunay::Finite_faces_iterator fit = dt.finite_faces_begin();
       fit != dt.finite_faces_end();
       ++fit) {
    Point_2 p1 = fit->vertex(0)->point();
    Point_2 p2 = fit->vertex(1)->point();
    Point_2 p3 = fit->vertex(2)->point();

    // set width of line
    cairo_set_line_width(cr, 0.05);

    // set color
    cairo_set_source_rgb(cr, clr.r, clr.g, clr.b);

    cairo_move_to(cr, p1.x(), 512 - p1.y());
    cairo_line_to(cr, p2.x(), 512 - p2.y());
    cairo_line_to(cr, p3.x(), 512 - p3.y());
    cairo_line_to(cr, p1.x(), 512 - p1.y());
    cairo_stroke(cr);
  }
}