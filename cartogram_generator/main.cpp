// TO DO: positional matching of argument flags

#include "constants.h"
#include "map_state.h"
#include "blur_density.h"
#include "fill_with_density.h"
#include "flatten_density.h"
#include "project.h"
#include "read_csv.h"
#include "read_geojson.h"
#include "rescale_map.h"
#include "write_eps.h"
#include "check_topology.h"
#include "write_to_json.h"
#include <boost/program_options.hpp>
#include <iostream>

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
  bool polygons_to_eps, density_to_eps;

  // Parse command-line options. See
  // https://theboostcpplibraries.com/boost.program_options
  variables_map vm;
  try {
    options_description desc{"Options"};
    desc.add_options()(
      "help,h", "Help screen"
      )(
      "geometry,g",
      value<std::string>(&geo_file_name)
      ->required()
      ->notifier(on_geometry),
      "GeoJSON file"
      )(
      "visual_variable_file,v",
      value<std::string>()
      ->notifier(on_visual_variable_file),
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
      value<bool>(&world)
      ->default_value(false)
      ->implicit_value(true),
      "Boolean: is input a world map in longitude-latitude format?"
      )(
      "polygons_to_eps,e",
      value<bool>(&polygons_to_eps)
      ->default_value(false)
      ->implicit_value(true),
      "Boolean: make EPS image of input and output?"
      )(
      "density_to_eps,d",
      value<bool>(&density_to_eps)
      ->default_value(false)
      ->implicit_value(true),
      "Boolean: make EPS images *_density_*.eps?"
      );
    store(parse_command_line(argc, argv, desc), vm);
    if (vm.count("help") || argc == 1) {
      std::cerr << desc << '\n';
      return EXIT_SUCCESS;
    } else {
      notify(vm);  // Triggers notifier functions such as on_geometry()
    }
  } catch (const error &ex) {
    std::cerr << "ERROR: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  // Initialising map_state (which stores all our map data)
  MapState map_state("test_data.txt",
                     world,
                     density_to_eps);

  // create a polygon with three holes
  Polygon sg_mainland_boundary;
  sg_mainland_boundary.push_back(Point(10, 10)); sg_mainland_boundary.push_back(Point(10, 120));
  sg_mainland_boundary.push_back(Point(120, 120)); sg_mainland_boundary.push_back(Point(120, 10));

  std::vector<Polygon> holes(4);

  // hole 1
  holes[0].push_back(Point(20, 110)); holes[0].push_back(Point(50, 110));
  holes[0].push_back(Point(20, 80));

  // hole 2
  holes[1].push_back(Point(70, 90)); holes[1].push_back(Point(80, 90));
  holes[1].push_back(Point(80, 30)); holes[1].push_back(Point(70, 30));

  // hole 3
  holes[2].push_back(Point(100, 40)); holes[2].push_back(Point(110, 40));
  holes[2].push_back(Point(110, 20)); holes[2].push_back(Point(90, 20));
  holes[2].push_back(Point(100, 30));

  // hole 4 (optioal)
  // holes[3].push_back(Point(30, 70)); holes[3].push_back(Point(60, 70));
  // holes[3].push_back(Point(60, 40)); holes[3].push_back(Point(30, 40));

  Polygon_with_holes sg_mainland(sg_mainland_boundary, holes.begin(), holes.end());

  // Creating another polygon with zero holes

  // Empty vector
  std::vector<Polygon> empty_vec(0);

  Polygon sentosa_boundary;
  sentosa_boundary.push_back(Point(200, 200)); sentosa_boundary.push_back(Point(150, 200));
  sentosa_boundary.push_back(Point(150, 150)); sentosa_boundary.push_back(Point(200, 150));

  Polygon_with_holes sentosa(sentosa_boundary, empty_vec.begin(), empty_vec.end());

  // Creating a GeoDiv
  GeoDiv gd_sg("Singapore"); // inside the quotes should be the GeoDiv ID
                             // the GeoDiv ID should be in the CSV and GeoJSON for every feature

  // Putting the above created polygon with three holes into the GeoDiv
  // Remember: GeoDivs can hold multiple polygons with holes
  gd_sg.push_back(sg_mainland);
  gd_sg.push_back(sentosa);

  Polygon malaysia_boundary;
  malaysia_boundary.push_back(Point(300, 300)); malaysia_boundary.push_back(Point(250, 300));
  malaysia_boundary.push_back(Point(250, 250)); malaysia_boundary.push_back(Point(300, 250));

  Polygon_with_holes malaysia_mainland(malaysia_boundary, empty_vec.begin(), empty_vec.end());

  // Creating another GeoDiv
  GeoDiv gd_ml("Malaysia");

  gd_ml.push_back(malaysia_mainland);

  // Adding all GeoDivs to our map
  map_state.push_back(gd_sg);
  map_state.push_back(gd_ml);

  // CGAL command to make printing pretty
  CGAL::set_pretty_mode(std::cout);

  // Iterating through the entire map
  // Iterating over GeoDivs in map_state
  for (GeoDiv gd : map_state.geo_divs()) {

    std::cout << "GeoDiv ID: " << gd.id() << std::endl << std::endl;
    // Iterating over points in Polygon_with_holes
    int i = 0;
    for (Polygon_with_holes pwh : gd.polygons_with_holes()) {
      std::cout << "Polygon_with_holes number " << i << ": " << std::endl;
      i++;

      // To pring a new line
      std::cout << std::endl;

      std::cout << "Outer boundary: "<< std::endl << std::endl;
      int j = 0;
      // Iterating over points in exterior boundary
      for (Point p : pwh.outer_boundary()) {
        std::cout << "Point number " << j << ": " << std::endl;
        j++;
        std::cout << p << std::endl;

      }
      std::cout << std::endl;

      std::cout << "Holes: "<< std::endl << std::endl;
      int k = 0;
      // Iterating over holes
      for (auto hci = pwh.holes_begin(); hci != pwh.holes_end(); ++hci) {
        std::cout << "Hole number " << k << ": "<< std::endl << std::endl;
        k++;

        Polygon hole = *hci;

        int l = 0;
        // Iterating over points in hole
        for (Point p : hole) {

          std::cout << "Point number " << l << ": " << std::endl;
          l++;
          std::cout << p << std::endl;
        }
        std::cout << std::endl;

      }

      std::cout << std::endl << std::endl;
    }
  }

  return EXIT_SUCCESS;

  // MapState map_state(vm["visual_variable_file"].as<std::string>(),
  //                    world,
  //                    density_to_eps);

  // Read visual variables (e.g. area, color) from CSV
  read_csv(vm, &map_state);

  // Read geometry
  try {
    read_geojson(geo_file_name, &map_state);
  } catch (const std::system_error& e) {
    std::cerr << "ERROR: "
              << e.what()
              << " ("
              << e.code()
              << ")"
              << std::endl;
    return EXIT_FAILURE;
  }

  try {
    holes_inside_polygons(&map_state);
  } catch (const std::system_error& e) {
    std::cerr << "ERROR: "
              << e.what()
              << " ("
              << e.code()
              << ")"
              << std::endl;
    return EXIT_FAILURE;
  }

  // Determining name of input map
  std::string map_name = geo_file_name;
  if (map_name.find_last_of("/\\") != std::string::npos) {
    map_name = map_name.substr(map_name.find_last_of("/\\") + 1);
  }
  if (map_name.find('.') != std::string::npos) {
    map_name = map_name.substr(0, map_name.find('.'));
  }

  // Rescale map to fit into a rectangular box [0, lx] * [0, ly].
  rescale_map(long_grid_side_length, &map_state);

  // Writing EPS, if requested by command line option
  if (polygons_to_eps) {
    std::cout << "Writing " << map_name << "_input.eps" << std::endl;
    write_map_to_eps((map_name + "_input.eps"), &map_state);
  }

  // Start map integration
  while (map_state.n_finished_integrations() < max_integrations &&
         map_state.max_area_err() > max_permitted_area_error) {

    std::cout << "Integration number "
              << map_state.n_finished_integrations()
              <<std::endl;

    fill_with_density(&map_state);
    if (map_state.n_finished_integrations() == 0) {
      blur_density(5.0, &map_state);
    } else{
      blur_density(0.0, &map_state);
    }
    flatten_density(&map_state);
    project(&map_state);
    map_state.inc_integration();
  }

  // Printing final cartogram
  json cart_json = cgal_to_json(&map_state);
  write_to_json(cart_json,
                geo_file_name,
                (map_name + "_cartogram_scaled.geojson"));

  // Printing EPS of output cartogram
  if (polygons_to_eps) {
    std::cout << "Writing " << map_name << "_output.eps" << std::endl;
    write_map_to_eps((map_name + "_output.eps"), &map_state);
  }

  // Removing transformations
  unscale_map(&map_state);

  // Printing unscaled cartogram
  cart_json = cgal_to_json(&map_state);
  write_to_json(cart_json,
                geo_file_name,
                (map_name + "_cartogram_unscaled.geojson"));

  return EXIT_SUCCESS;
}
