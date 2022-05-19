#include <iostream>
#include "cartogram_info.h"
#include "constants.h"

// Cairo for drawing and creating .ps files
#include <cairo/cairo-ps.h>
#include <cairo/cairo.h>

// CGAL's Quadtree data structure
#include <CGAL/Quadtree.h>
#include <CGAL/Orthtree.h>
#include <CGAL/Orthtree_traits_d.h>
#include <CGAL/Simple_cartesian.h>

// Creating alias: type definitions to make code more readable.
typedef CGAL::Simple_cartesian<double> Kernel; // operating on 2d cartesian surface
typedef Kernel::Point_2 Point_2; // data structure for 2d point - use .x() and .y() method to access
                                 // stored data
typedef std::vector<Point_2> Point_vector; // a vector for Point_2 data structure
typedef CGAL::Orthtree_traits_2<Kernel> Traits; // Traits define additional properties for quadtree.
typedef CGAL::Orthtree<Traits, Point_vector> Orthtree; // Orthtree alias

// Helper functions

// Adds header information to the ps files.
void write_ps_header(std::string, cairo_surface_t *); 

// Given a Orthtree object, creates a .ps file for visualization.
void draw_QuadTree(Orthtree &, std::string, unsigned int, unsigned int); 

// Creates point vector from the given map from the command line
Point_vector get_cartogram_points(CartogramInfo *);


void quadtree_tutorial(CartogramInfo *cartogram_info) {
    
    Point_vector points_2d;
    
    // Add x = y line points from (1, 1) to (152, 152) to the points_2d vector
    for (std::size_t i = 1; i <= 152; ++i) {
        points_2d.emplace_back(Point_2(i, i));
    }
    
    // Create a quadtree
    Orthtree quadtree(points_2d, Orthtree::PointMap(), 1);
    
    // Reshapes the quadtree based on (depth, max points in a node before split)
    quadtree.refine(10, 3); // default: (10, 20)
    
    // refines the orthtree such that the difference of depth
    // between two immediate neighbor leaves is never more than 1.
    quadtree.grade();
    
    // Uncommnet the following line to print the whole quadtree on the terminal
    // std::cout << quadtree << std::endl;
    
    // Draw the qudtree
    draw_QuadTree(quadtree, "toymodel.ps", 150, 150); // 150, 150 are the (x, y) dimension of the canvas

    // ************************ Methods of quadtree *******************************
    
    // Get root node
    std::cout << "Root: " << quadtree.root() << std::endl;
    
    
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
    Orthtree::Node node = quadtree.locate(Point_2(30, 30)); // returns the node object
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
    std::cout << "Size: " << node.size() << std::endl; // number of points in the node
    std::cout << "Local Coordinates: " << node.local_coordinates() << std::endl; // relative to parent
    
    // parent's global coorindates * 2 + node's local coordinate
    std::cout << "Global Coordinates: " << node.global_coordinates()[0] << " " 
              << node.global_coordinates()[1] << std::endl;
   
   
    // Create a quadtree for the input given map
    Point_vector input_map_points = get_cartogram_points(cartogram_info);
    Orthtree quadtree_input_map(input_map_points, Orthtree::PointMap(), 1);
    quadtree_input_map.refine(10, 20);
    quadtree_input_map.grade();
    
    draw_QuadTree(quadtree_input_map, cartogram_info->map_name() + "_quadtree.ps", 512, 512);
}

// Creates point vector for the map data given in the command line
Point_vector get_cartogram_points(CartogramInfo *cart_info) {
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
                for (auto hci = pwh.holes_begin(); hci != pwh.holes_end();
                     ++hci) {
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

void write_ps_header(std::string filename, cairo_surface_t *surface) {
    const std::string title = "%%Title: " + filename;
    cairo_ps_surface_dsc_comment(surface, title.c_str());
    cairo_ps_surface_dsc_comment(surface,
                                 "%%Creator: Michael T. Gastner et al.");
    cairo_ps_surface_dsc_comment(surface, "%%For: Humanity");
    cairo_ps_surface_dsc_comment(surface, "%%Copyright: License CC BY");
    cairo_ps_surface_dsc_comment(surface, "%%Magnification: 1.0000");
}

void draw_QuadTree(Orthtree &quadtree, std::string filename,
                   unsigned int lx_ = 512, unsigned int ly_ = 512) {
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
    cairo_stroke(cr);
    cairo_show_page(cr);
    cairo_surface_destroy(surface);
    cairo_destroy(cr);
}