#include "albers_projection.h"
#include "blur_density.h"
#include "cartogram_info.h"
#include "check_topology.h"
#include "constants.h"
#include "flatten_density.h"
#include "geo_div.h"
#include "inset_state.h"
#include "parse_arguments.h"
#include "project.h"
#include "read_csv.h"
#include "read_geojson.h"
#include "rescale_map.h"
#include "smyth_projection.h"
#include "simplify_inset.h"
#include "write_eps.h"
#include "write_geojson.h"
#include "write_cairo.h"
#include <iostream>
#include <cmath>

int main(const int argc, const char *argv[])
{
  std::string geo_file_name, visual_file_name;  // Default values

  // Default number of grid cells along longer Cartesian coordinate axis
  unsigned int long_graticule_length = default_long_graticule_length;

  // Target number of points to retain after simplification
  unsigned int target_points_per_inset = default_target_points_per_inset;

  bool world;  // World maps need special projections

  // Another cartogram projection method based on triangulation of graticule
  // cells. It can eliminate intersections that occur when the projected
  // graticule lines are strongly curved.
  bool triangulation;

  // Shall the polygons be simplified?
  bool simplify;

  // Other boolean values that are needed to parse the command line arguments
  bool make_csv,
       output_equal_area,
       output_to_stdout,
       plot_density,
       plot_graticule,
       plot_intersections,
       plot_polygons;

  // Parse command-line arguments
  argparse::ArgumentParser arguments = parsed_arguments(
    argc,
    argv,
    geo_file_name,
    visual_file_name,
    long_graticule_length,
    target_points_per_inset,
    world,
    triangulation,
    simplify,
    make_csv,
    output_equal_area,
    output_to_stdout,
    plot_density,
    plot_graticule,
    plot_intersections,
    plot_polygons);

  // Initialize cart_info. It contains all information about the cartogram
  // that needs to be handled by functions called from main().
  CartogramInfo cart_info(world, visual_file_name);

  // Determine name of input map and store it
  std::string map_name = geo_file_name;
  if (map_name.find_last_of("/\\") != std::string::npos) {
    map_name = map_name.substr(map_name.find_last_of("/\\") + 1);
  }
  if (map_name.find('.') != std::string::npos) {
    map_name = map_name.substr(0, map_name.find('.'));
  }
  cart_info.set_map_name(map_name);
  if (!make_csv) {

    // Read visual variables (e.g. area, color) from CSV
    try {
      read_csv(arguments, &cart_info);
    } catch (const std::system_error& e) {
      std::cerr << "ERROR: "
                << e.what()
                << " ("
                << e.code()
                << ")"
                << std::endl;
      return EXIT_FAILURE;
    } catch (const std::runtime_error& e) {

      // If there is an error, it is probably because of an invalid CSV file
      std::cerr << "ERROR: "
                << e.what()
                << std::endl;
      return EXIT_FAILURE;
    }
  }

  // Read geometry. If the GeoJSON does not explicitly contain a "crs" field,
  // we assume that the coordinates are in longitude and latitude.
  std::string crs = "+proj=longlat";
  try {
    read_geojson(geo_file_name, make_csv, &crs, &cart_info);
  } catch (const std::system_error& e) {
    std::cerr << "ERROR: "
              << e.what()
              << " ("
              << e.code()
              << ")"
              << std::endl;
    return EXIT_FAILURE;
  }
  std::cerr << "Coordinate reference system: " << crs << std::endl;

  // Progress measured on a scale from 0 (start) to 1 (end)
  double progress = 0.0;

  // Store total number of GeoDivs to monitor progress
  double total_geo_divs = 0;
  for (const auto &inset_info : *cart_info.ref_to_inset_states()) {

    // `auto` will automatically deduce the const qualifier
    auto &inset_state = inset_info.second;
    total_geo_divs += inset_state.n_geo_divs();
  }

  // Project map and ensure that all holes are inside polygons
  for (auto &[inset_pos, inset_state] : *cart_info.ref_to_inset_states()) {

    // Check for errors in the input topology
    try {
      holes_inside_polygons(&inset_state);
    } catch (const std::system_error& e) {
      std::cerr << "ERROR: "
                << e.what()
                << " ("
                << e.code()
                << ")"
                << std::endl;
      return EXIT_FAILURE;
    }

    // Can the coordinates be interpreted as longitude and latitude?
    // TODO: the "crs" field for GeoJSON files seems to be deprecated. However,
    // in earlier specifications, the coordinate reference system used to be
    // written in the format specified here:
    // https://geojson.org/geojson-spec.html#coordinate-reference-system-objects.
    // It may be a good idea to make a list of possible entries corresponding
    // to longitude and lattitude projection. "urn:ogc:def:crs:OGC:1.3:CRS84"
    // is one such entry.
    const Bbox bb = inset_state.bbox();
    if (bb.xmin() >= -180.0 && bb.xmax() <= 180.0 &&
        bb.ymin() >= -90.0 && bb.ymax() <= 90.0 &&
        (crs == "+proj=longlat" ||
         crs == "urn:ogc:def:crs:OGC:1.3:CRS84")) {

      // If yes, transform the coordinates with the Albers projection if the
      // input map is not a world map. Otherwise, use the Smyth-Craster
      // projection.
      if (world) {
        project_to_smyth_equal_surface(&inset_state);
      } else {
        transform_to_albers_projection(&inset_state);
      }
    } else if (output_equal_area) {
      std::cerr << "ERROR: Input GeoJSON is not a longitude-latitude map."
                << std::endl;
      return EXIT_FAILURE;
    }
  }

  // Replace missing and zero target areas with absolute values
  cart_info.replace_missing_and_zero_target_areas();

  // Iterate over insets
  for (auto &[inset_pos, inset_state] : *cart_info.ref_to_inset_states()) {

    // Determine the name of the inset
    std::string inset_name = map_name;
    if (cart_info.n_insets() > 1) {
      inset_name = inset_name + "_" + inset_pos;
      std::cerr << "\nWorking on inset at position: "
                << inset_pos
                << std::endl;
    }
    inset_state.set_inset_name(inset_name);
    if (simplify) {

      // Simplification reduces the number of points used to represent the
      // GeoDivs in the inset, thereby reducing output file sizes and run
      // times
      simplify_inset(&inset_state,
                     target_points_per_inset);
    }
    if (output_equal_area) {
      normalize_inset_area(&inset_state,
                           cart_info.cart_total_target_area(),
                           output_equal_area);
    } else {

      // Rescale map to fit into a rectangular box [0, lx] * [0, ly]
      rescale_map(long_graticule_length,
                  &inset_state,
                  cart_info.is_world_map());

      // Set up Fourier transforms
      const unsigned int lx = inset_state.lx();
      const unsigned int ly = inset_state.ly();
      inset_state.ref_to_rho_init()->allocate(lx, ly);
      inset_state.ref_to_rho_ft()->allocate(lx, ly);
      inset_state.make_fftw_plans_for_rho();
      inset_state.initialize_cum_proj();
      inset_state.set_area_errors();

      // Automatically color GeoDivs if no colors are provided
      if (inset_state.colors_empty()) {
        inset_state.auto_color();
      }
      if (plot_polygons) {

        // Write PNG and PS files if requested by command-line option
        std::string input_filename = inset_state.inset_name();
        if (plot_graticule) {
          input_filename += "_input_graticule";
        } else {
          input_filename += "_input";
        }
        std::cerr << "Writing " << input_filename << std::endl;
        write_cairo_map(input_filename, plot_graticule, &inset_state);
      }

      // We make the approximation that the progress towards generating the
      // cartogram is proportional to the number of GeoDivs that are in the
      // finished insets
      const double inset_max_frac = inset_state.n_geo_divs() / total_geo_divs;

      // Start map integration
      while (inset_state.n_finished_integrations() < max_integrations &&
             inset_state.max_area_error().value > max_permitted_area_error) {
        std::cerr << "Integration number "
                  << inset_state.n_finished_integrations()
                  << std::endl;

        // Calculate progress percentage. We assume that the maximum area
        // error is typically reduced to 1/5 of the previous value.
        const double ratio_actual_to_permitted_max_area_error =
          inset_state.max_area_error().value / max_permitted_area_error;
        const double n_predicted_integrations =
          std::max((log(ratio_actual_to_permitted_max_area_error) / log(5)),
                   1.0);

        // Blur density to speed up the numerics in flatten_density() below.
        // We slowly reduce the blur width so that the areas can reach their
        // target values.
        // TODO: whenever blur_width hits 0, the maximum area error will start
        // increasing again and eventually lead to an invalid graticule cell
        // error when projecting with triangulation. Investigate why. As a
        // temporary fix, we set blur_width to be always positive, regardless
        // of the number of integrations.
        double blur_width =
          std::pow(2.0, 5 - int(inset_state.n_finished_integrations()));
        // if (inset_state.n_finished_integrations() < max_integrations) {
        //   blur_width =
        //     std::pow(2.0, 5 - int(inset_state.n_finished_integrations()));
        // } else {
        //   blur_width = 0.0;
        // }
        std::cerr << "blur_width = " << blur_width << std::endl;
        inset_state.fill_with_density(plot_density);
        if (blur_width > 0.0) {
          blur_density(blur_width, plot_density, &inset_state);
        }
        if (plot_intersections) {
          inset_state.write_intersections_to_eps(intersections_resolution);
        }
        flatten_density(&inset_state);
        if (triangulation) {

          // Choose diagonals that are inside graticule cells
          fill_graticule_diagonals(&inset_state);

          // Densify map
          inset_state.densify_geo_divs();

          // Project with triangulation
          project_with_triangulation(&inset_state);
        } else {
          project(&inset_state);
        }
        if (simplify) {
          simplify_inset(&inset_state,
                         target_points_per_inset);
        }
        inset_state.increment_integration();

        // Update area errors
        inset_state.set_area_errors();
        std::cerr << "max. area err: "
                  << inset_state.max_area_error().value
                  << ", GeoDiv: "
                  << inset_state.max_area_error().geo_div
                  << std::endl;
        std::cerr << "Progress: "
                  << progress + (inset_max_frac / n_predicted_integrations)
                  << std::endl
                  << std::endl;
      }
      progress += inset_max_frac;
      std::cerr << "Finished inset "
                << inset_pos
                << "\nProgress: "
                << progress
                << std::endl;
      if (plot_intersections) {
        inset_state.write_intersections_to_eps(intersections_resolution);
      }
      if (plot_polygons) {
        std::string output_filename = inset_state.inset_name();
        if (plot_graticule) {
          output_filename += "_output_graticule";
        } else {
          output_filename += "_output";
        }
        std::cerr << "Writing "
                  << output_filename << std::endl;
        write_cairo_map(output_filename, plot_graticule,
                        &inset_state);
      }
      if (world) {
        project_from_smyth_equal_surface(&inset_state);
      } else {

        // Rescale insets in correct proportion to each other
        normalize_inset_area(&inset_state,
                             cart_info.cart_total_target_area());
      }

      // Clean up after finishing all Fourier transforms for this inset
      inset_state.destroy_fftw_plans_for_rho();
      inset_state.ref_to_rho_init()->free();
      inset_state.ref_to_rho_ft()->free();
    }  // End of loop over insets
  }

  // Shift insets so that they do not overlap
  shift_insets_to_target_position(&cart_info);

  // Output to GeoJSON
  std::string output_file_name;
  if (output_equal_area) {
    output_file_name = map_name + "_equal_area.geojson";
  } else {
    output_file_name = map_name + "_cartogram.geojson";
  }
  const auto cart_json = cgal_to_json(&cart_info);
  write_geojson(cart_json,
                geo_file_name,
                output_file_name,
                std::cout,
                output_to_stdout,
                &cart_info);
  return EXIT_SUCCESS;
}
