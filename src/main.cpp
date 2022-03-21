#include <chrono>

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
#include "simplify_inset.h"
#include "write_geojson.h"
#include "write_ps.h"
#include "xy_point.h"

typedef std::chrono::steady_clock::time_point time_point;
typedef std::chrono::high_resolution_clock clock_time;
typedef std::chrono::milliseconds ms;

template <typename T> std::chrono::milliseconds inMilliseconds(T duration)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}

int main(const int argc, const char *argv[]) {

  // Start of main function time
  time_point start_main = clock_time::now();
  std::string geo_file_name, visual_file_name;  // Default values

  // Default number of grid cells along longer Cartesian coordinate axis
  unsigned int max_n_graticule_rows_or_cols =
    default_max_n_graticule_rows_or_cols;

  // Target number of points to retain after simplification
  unsigned int target_points_per_inset = default_target_points_per_inset;

  // World maps need special projections. By default, we assume that the
  // input map is not a world map.
  bool world;

  // Another cartogram projection method based on triangulation of graticule
  // cells. It can eliminate intersections that occur when the projected
  // graticule lines are strongly curved.
  bool triangulation;

  // Shall the polygons be simplified?
  bool simplify;

  // Other boolean values that are needed to parse the command line arguments
  bool make_csv,
       make_polygon_ps,
       output_equal_area,
       output_to_stdout,
       plot_density,
       plot_graticule,
       plot_graticule_heatmap,
       plot_intersections;

  // Parse command-line arguments
  argparse::ArgumentParser arguments = parsed_arguments(
      argc,
      argv,
      geo_file_name,
      visual_file_name,
      max_n_graticule_rows_or_cols,
      target_points_per_inset,
      world,
      triangulation,
      simplify,
      make_csv,
      make_polygon_ps,
      output_equal_area,
      output_to_stdout,
      plot_density,
      plot_graticule,
      plot_graticule_heatmap,
      plot_intersections);

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

  // Parsing start time
  time_point start_parse = clock_time::now();

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

  // Parsing end time
  time_point end_parse = clock_time::now();

  std::cerr << "Coordinate reference system: " << crs << std::endl;

  // Progress measured on a scale from 0 (start) to 1 (end)
  double progress = 0.0;

  // Store total number of GeoDivs to monitor progress
  double total_geo_divs = 0;
  for (const auto &inset_info : *cart_info.ref_to_inset_states()) {

    // 'auto' will automatically deduce the const qualifier
    auto &inset_state = inset_info.second;
    total_geo_divs += inset_state.n_geo_divs();
  }

  // Albers projection start time
  time_point start_albers_proj = clock_time::now();

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
    const Bbox bb = inset_state.bbox();
    if (bb.xmin() >= -180.0 && bb.xmax() <= 180.0 &&
        bb.ymin() >= -90.0 && bb.ymax() <= 90.0 &&
        crs == "+proj=longlat") {

      // If yes, transform the coordinates with the Albers projection
      transform_to_albers_projection(&inset_state);
    } else if (output_equal_area) {
      std::cerr << "ERROR: Input GeoJSON is not a longitude-latitude map."
                << std::endl;
      return EXIT_FAILURE;
    }
  }

  // Albers projection end time
  time_point end_albers_proj = clock_time::now();

  // Replace missing and zero target areas with absolute values
  cart_info.replace_missing_and_zero_target_areas();

  // Create map to store duration of each inset integrations
  std::map<std::string, ms> insets_integration_times;

  // Loop over insets
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

      // Simplify inset if -s flag is passed. This option reduces the number
      // of points used to represent the GeoDivs in the inset, thereby
      // reducing the output file sizes and run times.
      simplify_inset(&inset_state, target_points_per_inset);
    }
    if (output_equal_area) {
      normalize_inset_area(
          &inset_state,
          cart_info.cart_total_target_area(),
          output_equal_area);
    } else {

      // Rescale map to fit into a rectangular box [0, lx] * [0, ly]
      rescale_map(
          max_n_graticule_rows_or_cols,
          &inset_state,
          cart_info.is_world_map());

      // Store original coordinates
      inset_state.store_original_geo_divs();

      // Set up Fourier transforms
      const unsigned int lx = inset_state.lx();
      const unsigned int ly = inset_state.ly();
      inset_state.ref_to_rho_init()->allocate(lx, ly);
      inset_state.ref_to_rho_ft()->allocate(lx, ly);
      inset_state.make_fftw_plans_for_rho();
      inset_state.initialize_cum_proj();
      inset_state.initialize_original_proj();
      inset_state.set_area_errors();

      // Automatically color GeoDivs if no colors are provided
      if (inset_state.colors_empty()) {
        inset_state.auto_color();
      }

      // Write PNG and PS files if requested by command-line option
      if (make_polygon_ps) {
        std::string input_filename = inset_state.inset_name();
        if (plot_graticule) {
          input_filename += "_input_graticule.ps";
        } else {
          input_filename += "_input.ps";
        }
        std::cerr << "Writing " << input_filename << std::endl;
        write_map_to_ps(input_filename, true, plot_graticule, &inset_state);
      }

      // We make the approximation that the progress towards generating the
      // cartogram is proportional to the number of GeoDivs that are in the
      // finished insets
      const double inset_max_frac = inset_state.n_geo_divs() / total_geo_divs;

      // Integration start time
      time_point start_integration = clock_time::now();

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
        // temporary fix, we set blur_width to always be non-zero, regardless
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

        inset_state.fill_with_density(plot_density, plot_graticule_heatmap);
        if (blur_width > 0.0) {
          blur_density(blur_width, plot_density, &inset_state);
        }
        if (plot_intersections) {
          inset_state.write_intersections_to_ps(intersections_resolution);
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

      // Integration end time
      time_point end_integration = clock_time::now();

      // Add integration time to the map
      insets_integration_times[inset_pos] =
          inMilliseconds(end_integration - start_integration);
      progress += inset_max_frac;
      std::cerr << "Finished inset "
                << inset_pos
                << "\nProgress: "
                << progress
                << std::endl;
      if (plot_intersections) {
        inset_state.write_intersections_to_ps(intersections_resolution);
      }

      // Print PS files of cartogram
      if (make_polygon_ps) {
        std::string output_filename = inset_state.inset_name();
        if (plot_graticule) {
          output_filename += "_output_graticule.ps";
        } else {
          output_filename += "_output.ps";
        }
        std::cerr << "Writing "
                  << output_filename << std::endl;
        write_map_to_ps(output_filename, true, plot_graticule, &inset_state);
      }

      if(plot_graticule_heatmap) {
        std::string inset_filename = inset_state.inset_name();
        std::string output_filename = inset_filename + "_cartogram_graticule_heatmap.ps";
        std::cerr << "Writing "
                  << output_filename << std::endl;
        write_graticule_heatmap_to_ps(output_filename, false, &inset_state);
        output_filename = inset_filename + "_equalarea_graticule_heatmap.ps";
        std::cerr << "Writing "
                  << output_filename << std::endl;
        write_graticule_heatmap_to_ps(output_filename, true, &inset_state);
      }

      // Rescale insets in correct proportion to each other
      normalize_inset_area(&inset_state, cart_info.cart_total_target_area());

      // Clean up after finishing all Fourier transforms for this inset
      inset_state.destroy_fftw_plans_for_rho();
      inset_state.ref_to_rho_init()->free();
      inset_state.ref_to_rho_ft()->free();
    } // End of loop over insets
  }

  // Output a density heatmap's bar
  if (plot_density) {
    std::string output_filename = "density_heatmap_bar.ps";
    std::cerr << "Writing "
              << output_filename << std::endl;
    write_density_bar_to_ps(output_filename);
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
  const nlohmann::json cart_json = cgal_to_json(&cart_info);
  write_geojson(cart_json,
                geo_file_name,
                output_file_name,
                std::cout,
                output_to_stdout,
                &cart_info);

  // End of main function time
  time_point end_main = clock_time::now();

  // Calculate differences in time
  ms parsing_time = inMilliseconds(end_parse - start_parse);
  ms albers_proj_time = inMilliseconds(end_albers_proj - start_albers_proj);
  ms total_time = inMilliseconds(end_main - start_main);

  // Show Time Report
  std::cerr << std::endl;
  std::cerr << "********** Time Report **********" << std::endl;
  std::cerr << "Parsing time: " << parsing_time.count() << " ms" << std::endl;
  std::cerr << "Albers projection time: " << albers_proj_time.count() << " ms"
            << std::endl;

  // Iterate over the map and print integration times
  for (auto [inset_pos, inset_integration_time] : insets_integration_times) {
    std::cerr << "Integration time for inset " << inset_pos << ": "
              << inset_integration_time.count() << " ms" << std::endl;
  }
  std::cerr << "Total time: " << total_time.count() << " ms" << std::endl;
  std::cerr << "*********************************" << std::endl;

  return EXIT_SUCCESS;
}
