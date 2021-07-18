#include "cartogram_info.h"
#include "inset_state.h"
#include "csv.hpp"
#include <boost/program_options.hpp>
#include <iostream>

void read_csv(const boost::program_options::variables_map vm,
              CartogramInfo *cart_info)
{
  // Get name of CSV file from vm
  std::string csv_name;
  if (vm.count("visual_variable_file")) {
    csv_name = vm["visual_variable_file"].as<std::string>();
  } else {
    std::cerr << "ERROR: No CSV file given!" << std::endl;
    _Exit(15);
  }

  // Opening CSV Reader
  csv::CSVReader reader(csv_name);

  // Finding index of column with IDs
  std::string id_header;
  int id_col = 0;
  if (vm.count("id")) {
    id_header = vm["id"].as<std::string>();
    id_col = reader.index_of(id_header);
  } else {
    id_header = reader.get_col_names()[0];
  }

  // Store header name of identifiers to read GeoJSON
  cart_info->set_id_header(id_header);

  // Finding index of column with target areas
  double total_cart_target_area = 0;  // Total of all GeoDivs
  int area_col = 1;
  if (vm.count("area")) {
    area_col = reader.index_of(vm["area"].as<std::string>());
  }

  // Finding index of column with header name "Inset"
  std::string inset_header = "Inset";
  if (vm.count("inset")) {
    inset_header = vm["inset"].as<std::string>();
  }
  int inset_col = reader.index_of(inset_header);

  // Finding index of column with header name "Color/Colour"
  std::string color_header = "Color";
  if (vm.count("color")) {
    color_header = vm["color"].as<std::string>();
  }
  int color_col = reader.index_of(color_header);
  if (color_col == csv::CSV_NOT_FOUND) {
    color_col = reader.index_of("Colour");
  }

  // Reading CSV
  for (auto &row : reader) {
    if (row.size() < 2) {
      std::cerr << "ERROR: CSV with >= 2 columns (IDs, target areas) required"
                << std::endl
                << "Some rows in your CSV may not have values for all columns"
                << std::endl;
      _Exit(17);
    }

    // Read ID of geographic division
    std::string id = row[id_col].get();
    if (cart_info->ids_in_visual_variables_file().contains(id)) {
      std::cerr << "ERROR: ID "
                << id
                << " appears more than once in CSV"
                << std::endl;
      _Exit(301);
    }
    cart_info->insert_id_in_visual_variables_file(id);

    // Get target area
    csv::CSVField area_field = row[area_col];
    double area;
    if (!area_field.is_num()) {
      std::cerr << "area_field: " << area_field.get() << std::endl;
      if (area_field.get().compare("NA") == 0) {
        area = -1.0;  // Use negative area as sign of a missing value
      } else {
        std::cerr << "ERROR: Areas must be numeric or NA" << std::endl;
        _Exit(201);
      }
    } else {
      area = area_field.get<double>();
      if (area < 0.0) {
        std::cerr << "ERROR: negative area in CSV" << std::endl;
        _Exit(101);
      }
    }

    // Adding target_area (i.e. population, GDP) value
    // to store target area of all GeoDivs
    total_cart_target_area += area;

    // Read color
    std::string color = "";
    if (color_col != csv::CSV_NOT_FOUND) {
      color = row[color_col].get();
    }

    // Read inset
    std::string inset_pos = "C"; // Assuming inset_pos is C
    if (inset_col != csv::CSV_NOT_FOUND) {
      inset_pos = row[inset_col].get();
      std::string inset_pos_original = inset_pos;

    // Set to C if inset position is blank
    if (inset_pos == "") {
      inset_pos = "C";
    }

      // Now it can process inputs like "center"/"left"/"right"
      inset_pos = std::toupper(inset_pos[0]);

      // Enables user to give inset position "U"/"D" for top and bottom inset
      if (inset_pos == "U") {
        inset_pos = "T";
      } else if (inset_pos == "D") {
        inset_pos = "B";
      }

      // If unrecognized, set inset position to "C"
      if (inset_pos != "C" && inset_pos != "L" &&
          inset_pos != "R" && inset_pos != "T" && inset_pos != "B") {
        std::cerr << "Unrecongnized inset position : "
                  << inset_pos_original
                  << " for Region: "
                  << id
                  << "\nSetting "
                  << id
                  << "\'s inset position to Center (C)."
                  << std::endl;
        inset_pos = "C";
      }
    }

    // Associating GeoDiv ID with Inset Positon
    cart_info->gd_to_inset_insert(id, inset_pos);

    // Checking whether inset_state for inset_pos already exists
    bool found = false;
    for (auto &inset_state : *cart_info->ref_to_inset_states()) {
      if (inset_state.pos() == inset_pos) {
        inset_state.target_areas_insert(id, area);
        if (color != "") {
          inset_state.colors_insert(id, color);
        }
        found = true;
      }
    }
    if (!found) {
      InsetState inset_state(inset_pos);
      inset_state.target_areas_insert(id, area);
      if (color != "") {
        inset_state.colors_insert(id, color);
      }
      cart_info->push_back(inset_state);
    }
  }
  return;
}
