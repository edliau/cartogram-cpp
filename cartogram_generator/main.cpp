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

struct Matrix {

  // all positions of matrix
  double p11, p12, p13, p21, p22, p23, p31, p32, p33;

  Matrix() {
    return;
  }

  // Matrix from a triangle
  Matrix(Polygon p) {

    p31 = 1;
    p32 = 1;
    p33 = 1;

  }

  double scale(double multiplier) {

    p11 = p11 * multiplier;
    p12 = p12 * multiplier;
    p13 = p13 * multiplier;
    p21 = p21 * multiplier;
    p22 = p22 * multiplier;
    p23 = p23 * multiplier;
    p31 = p31 * multiplier;
    p32 = p32 * multiplier;
    p33 = p33 * multiplier;

  }

  void print() {
    std::cout << p11 << " " << p12 << " " << p13 << "\n\n";
    std::cout << p21 << " " << p22 << " " << p23 << "\n\n";
    std::cout << p31 << " " << p32 << " " << p33 << "\n\n";
  }

  // calculate determinant
  double det() {
    return p11 * ((p22 * p33) - (p23 * p32)) -
           p12 * ((p21 * p33) - (p23 * p31)) +
           p13 * ((p21 * p32) - (p22 * p31));
  }

  Matrix adjugate() {
    Matrix inv;

    inv.p11 = ((p22 * p33) - (p23 * p32));
    inv.p21 = - ((p21 * p33) - (p23 * p31));
    inv.p31 = ((p21 * p32) - (p22 * p31));
    inv.p12 = - ((p12 * p33) - (p13 * p32));
    inv.p22 = ((p11 * p33) - (p13 * p31));
    inv.p32 = - ((p11 * p32) - (p12 * p31));
    inv.p13 = ((p12 * p23) - (p13 * p22));
    inv.p23 = - ((p11 * p23) - (p13 * p21));
    inv.p33 = ((p11 * p22) - (p12 * p21));

  }

};

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

  MapState map_state;;
  map_state.make_grid(64, 64);

  std::vector<Polygon> holes;

  GeoDiv gd_old_grat("old_grat");;

  // old graticule cell
  Polygon old_graticule;
  old_graticule.push_back(Point(16, 16));
  old_graticule.push_back(Point(32, 16));
  old_graticule.push_back(Point(32, 32));
  old_graticule.push_back(Point(16, 32));
  Polygon_with_holes old_grat(old_graticule, holes.begin(), holes.end());

  Polygon old_tri_right;
  old_tri_right.push_back(Point(16, 16));
  old_tri_right.push_back(Point(32, 16));
  old_tri_right.push_back(Point(32, 32));

  Polygon old_tri_left;
  old_tri_left.push_back(Point(16, 16));
  old_tri_left.push_back(Point(16, 32));
  old_tri_left.push_back(Point(32, 32));

  gd_old_grat.push_back(old_grat);
  // map_state.push_back(gd_old_grat);

  GeoDiv gd_old_poly("old_poly");

  // create a polygon with 0 holes
  Polygon old_polygon;
  old_polygon.push_back(Point(18, 18));
  old_polygon.push_back(Point(19, 24));
  old_polygon.push_back(Point(23, 21));
  old_polygon.push_back(Point(23, 17));
  Polygon_with_holes old_poly(old_polygon, holes.begin(), holes.end());

  gd_old_poly.push_back(old_poly);
  map_state.push_back(gd_old_poly);

  std::vector<GeoDiv> new_map;

  GeoDiv gd_new_grat("new_grat");

  // new graticule cell
  Polygon new_graticule;
  new_graticule.push_back(Point(16, 16));
  new_graticule.push_back(Point(32, 16));
  new_graticule.push_back(Point(21, 21));
  new_graticule.push_back(Point(16, 32));
  Polygon_with_holes new_grat(new_graticule, holes.begin(), holes.end());

  gd_new_grat.push_back(new_grat);
  new_map.push_back(gd_new_grat);

  Polygon new_tri_right;
  new_tri_right.push_back(Point(16, 16));
  new_tri_right.push_back(Point(32, 16));
  new_tri_right.push_back(Point(21, 21));

  Polygon new_tri_left;
  new_tri_left.push_back(Point(16, 16));
  new_tri_left.push_back(Point(16, 32));
  new_tri_left.push_back(Point(21, 21));

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

        // use triangle formula, depending on wheher point is in triangle or not

        double ainv11, ainv12, ainv13, ainv21, ainv22, ainv23, ainv31, ainv32, ainv33, t11, t12, t13, t21, t22, t23, det, new_x, new_y;

        if (old_tri_left.bounded_side(p) == CGAL::ON_BOUNDED_SIDE ||
            old_tri_left.bounded_side(p) == CGAL::ON_BOUNDARY) {


              /**************************** Determinant of A. ****************************/

             det = old_tri_left[0][0] * old_tri_left[1][1] + old_tri_left[1][0] * old_tri_left[2][1] + old_tri_left[2][0] * old_tri_left[0][1]
                   - old_tri_left[1][0] * old_tri_left[0][1] - old_tri_left[2][0] * old_tri_left[1][1] - old_tri_left[0][0] * old_tri_left[2][1];

             /*********** Compute det(A) * A^{-1}. We divide by det(A) later. ***********/

             ainv11 = old_tri_left[1][1] - old_tri_left[2][1];
             ainv12 = old_tri_left[2][0] - old_tri_left[1][0];
             ainv13 = old_tri_left[1][0] * old_tri_left[2][1] - old_tri_left[1][1] * old_tri_left[2][0];
             ainv21 = old_tri_left[2][1] - old_tri_left[0][1];
             ainv22 = old_tri_left[0][0] - old_tri_left[2][0];
             ainv23 = old_tri_left[0][1] * old_tri_left[2][0] - old_tri_left[0][0] * old_tri_left[2][1];
             ainv31 = old_tri_left[0][1] - old_tri_left[1][1];
             ainv32 = old_tri_left[1][0] - old_tri_left[0][0];
             ainv33 = old_tri_left[0][0] * old_tri_left[1][1] - old_tri_left[0][1] * old_tri_left[1][0];

             /******************************** Compute T. *******************************/

             t11 = new_tri_left[0][0] * ainv11 + new_tri_left[1][0] * ainv21 + new_tri_left[2][0] * ainv31;
             t12 = new_tri_left[0][0] * ainv12 + new_tri_left[1][0] * ainv22 + new_tri_left[2][0] * ainv32;
             t13 = new_tri_left[0][0] * ainv13 + new_tri_left[1][0] * ainv23 + new_tri_left[2][0] * ainv33;
             t21 = new_tri_left[0][1] * ainv11 + new_tri_left[1][1] * ainv21 + new_tri_left[2][1] * ainv31;
             t22 = new_tri_left[0][1] * ainv12 + new_tri_left[1][1] * ainv22 + new_tri_left[2][1] * ainv32;
             t23 = new_tri_left[0][1] * ainv13 + new_tri_left[1][1] * ainv23 + new_tri_left[2][1] * ainv33;

             /********************* Transform the input coordinates. ********************/

             new_x = (t11*(p[0]) + t12*(p[1]) + t13) / det;
             new_y = (t21*(p[0]) + t22*(p[1]) + t23) / det;

              // find transformation matrix

              // transform point using transformation matrix
            }
            else if (old_tri_right.bounded_side(p) == CGAL::ON_BOUNDED_SIDE ||
                old_tri_right.bounded_side(p) == CGAL::ON_BOUNDARY) {

                  /**************************** Determinant of A. ****************************/

                 det = old_tri_right[0][0] * old_tri_right[1][1] + old_tri_right[1][0] * old_tri_right[2][1] + old_tri_right[2][0] * old_tri_right[0][1]
                       - old_tri_right[1][0] * old_tri_right[0][1] - old_tri_right[2][0] * old_tri_right[1][1] - old_tri_right[0][0] * old_tri_right[2][1];

                 /*********** Compute det(A) * A^{-1}. We divide by det(A) later. ***********/

                 ainv11 = old_tri_right[1][1] - old_tri_right[2][1];
                 ainv12 = old_tri_right[2][0] - old_tri_right[1][0];
                 ainv13 = old_tri_right[1][0] * old_tri_right[2][1] - old_tri_right[1][1] * old_tri_right[2][0];
                 ainv21 = old_tri_right[2][1] - old_tri_right[0][1];
                 ainv22 = old_tri_right[0][0] - old_tri_right[2][0];
                 ainv23 = old_tri_right[0][1] * old_tri_right[2][0] - old_tri_right[0][0] * old_tri_right[2][1];
                 ainv31 = old_tri_right[0][1] - old_tri_right[1][1];
                 ainv32 = old_tri_right[1][0] - old_tri_right[0][0];
                 ainv33 = old_tri_right[0][0] * old_tri_right[1][1] - old_tri_right[0][1] * old_tri_right[1][0];

                 /******************************** Compute T. *******************************/

                 t11 = new_tri_right[0][0] * ainv11 + new_tri_right[1][0] * ainv21 + new_tri_right[2][0] * ainv31;
                 t12 = new_tri_right[0][0] * ainv12 + new_tri_right[1][0] * ainv22 + new_tri_right[2][0] * ainv32;
                 t13 = new_tri_right[0][0] * ainv13 + new_tri_right[1][0] * ainv23 + new_tri_right[2][0] * ainv33;
                 t21 = new_tri_right[0][1] * ainv11 + new_tri_right[1][1] * ainv21 + new_tri_right[2][1] * ainv31;
                 t22 = new_tri_right[0][1] * ainv12 + new_tri_right[1][1] * ainv22 + new_tri_right[2][1] * ainv32;
                 t23 = new_tri_right[0][1] * ainv13 + new_tri_right[1][1] * ainv23 + new_tri_right[2][1] * ainv33;

                 /********************* Transform the input coordinates. ********************/

                 new_x = (t11*(p[0]) + t12*(p[1]) + t13) / det;
                 new_y = (t21*(p[0]) + t22*(p[1]) + t23) / det;

                  // find transformation matrix

                  // transform point using transformation matrix
                }

            Point transformed(new_x, new_y);
            temp_ext_boundary.push_back(transformed);

          }

          Polygon_with_holes temp_pwh(temp_ext_boundary, holes.begin(), holes.end());
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
