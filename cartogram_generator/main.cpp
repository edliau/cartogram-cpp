// TO DO: positional matching of argument flags

#include "constants.h"
#include "map_state.h"
#include "blur_density.h"
#include "fill_with_density.h"
#include "flatten_density.h"
#include "read_csv.h"
#include "read_geojson.h"
#include "rescale_map.h"
#include "write_eps.h"
#include "densification_points.h"
#include "matrix.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <CGAL/Boolean_set_operations_2.h>

// Functions that are called if the corresponding command-line options are
// present
void on_geometry(const std::string geometry_file_name)
{
  std::cerr << "Using geometry from file " << geometry_file_name << std::endl;
  return;
}

void on_visual_variable_file(const std::string geometry_file_name)
{
  std::cerr << "Using visual variables from file "
            << geometry_file_name
            << std::endl;
  return;
}

int main(const int argc, const char *argv[])
{
  using namespace boost::program_options;
  std::string geo_file_name;

  // Default number of grid cells along longer Cartesian coordinate axis.
  int long_grid_side_length = default_long_grid_side_length;

  // World maps need special projections. By default, we assume that the
  // input map is not a world map.
  bool world;

  // Other boolean values that are needed to parse the command line arguments
  bool input_polygons_to_eps,
       density_to_eps;

  // Parse command-line options. See
  // https://theboostcpplibraries.com/boost.program_options
  variables_map vm;
  try {
    options_description desc{"Options"};
    desc.add_options()(
      "help,h", "Help screen"
      )(
      "geometry,g",
      value<std::string>(&geo_file_name)/*->required()*/->notifier(on_geometry),
      "GeoJSON file"
      )(
      "visual_variable_file,v",
      value<std::string>()->notifier(on_visual_variable_file),
      "CSV file with ID, area, and (optionally) colour"
      )(
      "id,i",
      value<std::string>(),
      "Column name for IDs of geographic divisions (default: 1st CSV column)"
      )(
      "area,a",
      value<std::string>(),
      "Column name for target areas (default: 2nd CSV column)"
      )(
      "color,c",
      value<std::string>(),
      "Column name for colors (default: 3rd CSV column if it exists)"
      )(
      "long_grid_side_length,l",
      value<int>(&long_grid_side_length),
      "Number of grid cells along longer Cartesian coordinate axis"
      )(
      "world,w",
      value<bool>(&world)->default_value(false)->implicit_value(false),
      "Boolean: is input a world map in longitude-latitude format?"
      )(
      "input_polygons_to_eps",
      value<bool>(&input_polygons_to_eps)
      ->default_value(false)
      ->implicit_value(true),
      "Boolean: make EPS image input_polygons.eps?"
      )(
      "density_to_eps",
      value<bool>(&density_to_eps)
      ->default_value(false)
      ->implicit_value(true),
      "Boolean: make EPS images input_*.eps?"
      );
    store(parse_command_line(argc, argv, desc), vm);
    if (vm.count("help") || vm.empty()) {
      std::cerr << desc << '\n';
      return EXIT_SUCCESS;
    } else {
      notify(vm);  // Triggers notifier functions such as on_geometry()
    }
  } catch (const error &ex) {
    std::cerr << "ERROR: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }
  // MapState map_state(vm["visual_variable_file"].as<std::string>(),
  //                    world,
  //                    density_to_eps);

  // Read visual variables (e.g. area, color) from CSV
  // read_csv(vm, &map_state);

  // Read geometry
  // try {
  //   read_geojson(geo_file_name, &map_state);
  // } catch (const std::system_error& e) {
  //   std::cerr << "ERROR: "
  //             << e.what()
  //             << " ("
  //             << e.code()
  //             << ")"
  //             << std::endl;
  //   return EXIT_FAILURE;
  // }

  CGAL::set_pretty_mode(std::cout);

  MapState map_state;
  map_state.make_grid(64, 64);

  std::vector<Polygon> holes;

  GeoDiv gd_old_grat("old_grat");

  // old graticule cell
  Quadrilateral old_graticule;
  old_graticule.push_back(Point(16, 16));
  old_graticule.push_back(Point(32, 16));
  old_graticule.push_back(Point(32, 32));
  old_graticule.push_back(Point(16, 32));
  Polygon_with_holes old_grat(old_graticule, holes.begin(), holes.end());

  // divided into triangles
  // Triangle old_tri_right;
  // old_tri_right.push_back(Point(16, 16));
  // old_tri_right.push_back(Point(32, 16));
  // old_tri_right.push_back(Point(32, 32));
  //
  // Triangle old_tri_left;
  // old_tri_left.push_back(Point(16, 16));
  // old_tri_left.push_back(Point(16, 32));
  // old_tri_left.push_back(Point(32, 32));

  gd_old_grat.push_back(old_grat);
  // map_state.push_back(gd_old_grat);

  GeoDiv gd_old_poly("old_poly");

  // create a polygon with 1 holes
  Polygon old_polygon;
  old_polygon.push_back(Point(18, 18));
  old_polygon.push_back(Point(19, 24));
  old_polygon.push_back(Point(23, 21));
  old_polygon.push_back(Point(23, 17));

  std::vector<Polygon> old_poly_holes;
  Polygon hole_1;
  hole_1.push_back(Point(20,20));
  hole_1.push_back(Point(20,21));
  hole_1.push_back(Point(21,21));
  hole_1.push_back(Point(21,20));
  old_poly_holes.push_back(hole_1);

  Polygon_with_holes old_poly(old_polygon, old_poly_holes.begin(), old_poly_holes.end());

  gd_old_poly.push_back(old_poly);
  map_state.push_back(gd_old_poly);

  std::vector<GeoDiv> new_map;

  GeoDiv gd_new_grat("new_grat");

  // new graticule cell
  Quadrilateral new_graticule;
  new_graticule.push_back(Point(16, 16));
  new_graticule.push_back(Point(32, 16));
  new_graticule.push_back(Point(21, 21));
  new_graticule.push_back(Point(16, 32));
  Polygon_with_holes new_grat(new_graticule, holes.begin(), holes.end());

  gd_new_grat.push_back(new_grat);
  new_map.push_back(gd_new_grat);

  // divided into triangles
  // Triangle new_tri_right;
  // new_tri_right.push_back(Point(16, 16));
  // new_tri_right.push_back(Point(32, 16));
  // new_tri_right.push_back(Point(21, 21));
  //
  // Triangle new_tri_left;
  // new_tri_left.push_back(Point(16, 16));
  // new_tri_left.push_back(Point(16, 32));
  // new_tri_left.push_back(Point(21, 21));

  // figure out new polygon
  // iterate through points in map_state

  // given: map_state

  for (GeoDiv gd : map_state.geo_divs()) {

    GeoDiv temp_geodiv("test");

    for (Polygon_with_holes pwh : gd.polygons_with_holes()) {

      Polygon temp_ext_boundary;

      for (Point p : pwh.outer_boundary()) {

        // given: point

        // find out graticule cell point is in
        //

        // find out transformed graticule cell representing old graticule cell
        // phong_function

        // divide both cells into triangles using midpoint

        double bl_tr_mid_x = (new_graticule[0][0] + new_graticule[2][0]) / 2;
        double bl_tr_mid_y = (new_graticule[0][1] + new_graticule[2][1]) / 2;
        Point bl_tr_mid(bl_tr_mid_x, bl_tr_mid_y);


        double br_tl_mid_x = (new_graticule[1][0] + new_graticule[3][0]) / 2;
        double br_tl_mid_y = (new_graticule[1][1] + new_graticule[3][1]) / 2;
        Point br_tl_mid(br_tl_mid_x, br_tl_mid_y);

        Triangle old_tri_right;
        Triangle old_tri_left;

        Triangle new_tri_right;
        Triangle new_tri_left;

        if (new_graticule.bounded_side(bl_tr_mid) == CGAL::ON_BOUNDED_SIDE) {

          old_tri_right.push_back(old_graticule[0]);
          old_tri_right.push_back(old_graticule[1]);
          old_tri_right.push_back(old_graticule[2]);

          new_tri_right.push_back(new_graticule[0]);
          new_tri_right.push_back(new_graticule[1]);
          new_tri_right.push_back(new_graticule[2]);


          old_tri_left.push_back(old_graticule[0]);
          old_tri_left.push_back(old_graticule[2]);
          old_tri_left.push_back(old_graticule[3]);

          new_tri_left.push_back(new_graticule[0]);
          new_tri_left.push_back(new_graticule[2]);
          new_tri_left.push_back(new_graticule[3]);

        } else if (new_graticule.bounded_side(br_tl_mid) == CGAL::ON_BOUNDED_SIDE) {

          old_tri_right.push_back(old_graticule[1]);
          old_tri_right.push_back(old_graticule[2]);
          old_tri_right.push_back(old_graticule[3]);

          new_tri_right.push_back(new_graticule[1]);
          new_tri_right.push_back(new_graticule[2]);
          new_tri_right.push_back(new_graticule[3]);

          old_tri_left.push_back(old_graticule[0]);
          old_tri_left.push_back(old_graticule[1]);
          old_tri_left.push_back(old_graticule[3]);

          new_tri_left.push_back(new_graticule[0]);
          new_tri_left.push_back(new_graticule[1]);
          new_tri_left.push_back(new_graticule[3]);

        } else {
          std::cout << "Both diaganols outside graticule cell!" << '\n';
        }

        // Suppose we find that, before the cartogram transformation, a point (x, y)
        // is in the triangle (a, b, c). We want to find its position in
        // the projected triangle (p, q, r). We locally approximate the cartogram
        // transformation by an affine transformation T such that T(a) = p,
        // T(b) = q and T(c) = r. We can think of T as a 3x3 matrix
        //  /t11 t12 t13\
        // | t21 t22 t23 |  such that
        //  \ 0   0   1 /
        //  /t11 t12 t13\   /a1 b1 c1\     /p1 q1 r1\
        // | t21 t22 t23 | | a2 b2 c2 | = | p2 q2 r2 | or TA = P. Hence T = PA^{-1}
        //  \ 0   0   1 /   \ 1  1  1/     \ 1  1  1/
        //                              /b2-c2 c1-b1 b1*c2-b2*c1\
        // We have A^{-1} = (1/det(A)) | c2-a2 a1-c1 a2*c1-a1*c2 |. By multiplying
        //                              \a2-b2 b1-a1 a1*b2-a2*b1/
        // PA^{-1} we obtain t11, t12, t13, t21, t22, t23. The preimage of (x, y) i
        // the unprojected map is then "pre" with coordinates
        // pre.x = t11*x + t12*y + t13, pre.y = t21*x + t22*y + t23.

        Triangle old_tri;
        Triangle new_tri;

        if (old_tri_left.bounded_side(p) == CGAL::ON_BOUNDED_SIDE ||
            old_tri_left.bounded_side(p) == CGAL::ON_BOUNDARY) {

              old_tri = old_tri_left;
              new_tri = new_tri_left;

            }
        else if (old_tri_right.bounded_side(p) == CGAL::ON_BOUNDED_SIDE ||
            old_tri_right.bounded_side(p) == CGAL::ON_BOUNDARY) {

              old_tri = old_tri_right;
              new_tri = new_tri_right;

            }
        else {
            std::cout << "Point outside graticule cell!" << '\n';
        }

        // Old triangle (a, b, c) as a matrix
        Matrix abc_mA = old_tri; // also matrix A

        // New triangle (p, q, r) as a matrix
        Matrix pqr_mP = new_tri; // also matrix P

        // Calculating transformation matrix
        Matrix mT = pqr_mP.multiply(abc_mA.inverse());

        // Transforming point and pushing back to temporary_ext_boundary
        Point transformed = mT.transform_point(p);
        temp_ext_boundary.push_back(transformed);
      }

      std::vector<Polygon> holes_temp;

      // Plot holes
      for (auto hci = pwh.holes_begin(); hci != pwh.holes_end(); ++hci) {
        Polygon hole = *hci;

        Polygon hole_temp;

        for (Point p : hole) {

                // given: point

                // find out graticule cell point is in
                //

                // find out transformed graticule cell representing old graticule cell
                // phong_function

                // divide both cells into triangles using midpoint

                double bl_tr_mid_x = (new_graticule[0][0] + new_graticule[2][0]) / 2;
                double bl_tr_mid_y = (new_graticule[0][1] + new_graticule[2][1]) / 2;
                Point bl_tr_mid(bl_tr_mid_x, bl_tr_mid_y);


                double br_tl_mid_x = (new_graticule[1][0] + new_graticule[3][0]) / 2;
                double br_tl_mid_y = (new_graticule[1][1] + new_graticule[3][1]) / 2;
                Point br_tl_mid(br_tl_mid_x, br_tl_mid_y);

                Triangle old_tri_right;
                Triangle old_tri_left;

                Triangle new_tri_right;
                Triangle new_tri_left;

                if (new_graticule.bounded_side(bl_tr_mid) == CGAL::ON_BOUNDED_SIDE) {

                  old_tri_right.push_back(old_graticule[0]);
                  old_tri_right.push_back(old_graticule[1]);
                  old_tri_right.push_back(old_graticule[2]);

                  new_tri_right.push_back(new_graticule[0]);
                  new_tri_right.push_back(new_graticule[1]);
                  new_tri_right.push_back(new_graticule[2]);


                  old_tri_left.push_back(old_graticule[0]);
                  old_tri_left.push_back(old_graticule[2]);
                  old_tri_left.push_back(old_graticule[3]);

                  new_tri_left.push_back(new_graticule[0]);
                  new_tri_left.push_back(new_graticule[2]);
                  new_tri_left.push_back(new_graticule[3]);

                } else if (new_graticule.bounded_side(br_tl_mid) == CGAL::ON_BOUNDED_SIDE) {

                  old_tri_right.push_back(old_graticule[1]);
                  old_tri_right.push_back(old_graticule[2]);
                  old_tri_right.push_back(old_graticule[3]);

                  new_tri_right.push_back(new_graticule[1]);
                  new_tri_right.push_back(new_graticule[2]);
                  new_tri_right.push_back(new_graticule[3]);

                  old_tri_left.push_back(old_graticule[0]);
                  old_tri_left.push_back(old_graticule[1]);
                  old_tri_left.push_back(old_graticule[3]);

                  new_tri_left.push_back(new_graticule[0]);
                  new_tri_left.push_back(new_graticule[1]);
                  new_tri_left.push_back(new_graticule[3]);

                } else {
                  std::cout << "Both diaganols outside graticule cell!" << '\n';
                }

                // Suppose we find that, before the cartogram transformation, a point (x, y)
                // is in the triangle (a, b, c). We want to find its position in
                // the projected triangle (p, q, r). We locally approximate the cartogram
                // transformation by an affine transformation T such that T(a) = p,
                // T(b) = q and T(c) = r. We can think of T as a 3x3 matrix
                //  /t11 t12 t13\
                // | t21 t22 t23 |  such that
                //  \ 0   0   1 /
                //  /t11 t12 t13\   /a1 b1 c1\     /p1 q1 r1\
                // | t21 t22 t23 | | a2 b2 c2 | = | p2 q2 r2 | or TA = P. Hence T = PA^{-1}
                //  \ 0   0   1 /   \ 1  1  1/     \ 1  1  1/
                //                              /b2-c2 c1-b1 b1*c2-b2*c1\
                // We have A^{-1} = (1/det(A)) | c2-a2 a1-c1 a2*c1-a1*c2 |. By multiplying
                //                              \a2-b2 b1-a1 a1*b2-a2*b1/
                // PA^{-1} we obtain t11, t12, t13, t21, t22, t23. The preimage of (x, y) i
                // the unprojected map is then "pre" with coordinates
                // pre.x = t11*x + t12*y + t13, pre.y = t21*x + t22*y + t23.

                Triangle old_tri;
                Triangle new_tri;

                if (old_tri_left.bounded_side(p) == CGAL::ON_BOUNDED_SIDE ||
                    old_tri_left.bounded_side(p) == CGAL::ON_BOUNDARY) {

                      old_tri = old_tri_left;
                      new_tri = new_tri_left;

                    }
                else if (old_tri_right.bounded_side(p) == CGAL::ON_BOUNDED_SIDE ||
                    old_tri_right.bounded_side(p) == CGAL::ON_BOUNDARY) {

                      old_tri = old_tri_right;
                      new_tri = new_tri_right;

                    }
                else {
                    std::cout << "Point outside graticule cell!" << '\n';
                }

                // Old triangle (a, b, c) as a matrix
                Matrix abc_mA = old_tri; // also matrix A

                // New triangle (p, q, r) as a matrix
                Matrix pqr_mP = new_tri; // also matrix P

                // Calculating transformation matrix
                Matrix mT = pqr_mP.multiply(abc_mA.inverse());

                // Transforming point and pushing back to temporary_ext_boundary
                Point transformed = mT.transform_point(p);
                hole_temp.push_back(transformed);
          }

          holes_temp.push_back(hole_temp);


      }

      Polygon_with_holes temp_pwh(temp_ext_boundary, holes_temp.begin(), holes_temp.end());
      temp_geodiv.push_back(temp_pwh);
    }
    new_map.push_back(temp_geodiv);
  }

  map_state.push_back(gd_old_grat);
  // Rescale map to fit into a rectangular box [0, lx] * [0, ly].
  // rescale_map(long_grid_side_length, &map_state);
  // if (input_polygons_to_eps) {
    std::cout << "Writing input_polygons.eps" << std::endl;
    write_map_to_eps("input_polygons.eps", &map_state);
  // // }

  map_state.set_geo_divs(new_map);

  std::cout << "Writing output_polygons.eps" << std::endl;
  write_map_to_eps("output_polygons.eps", &map_state);

  return EXIT_SUCCESS;
}
