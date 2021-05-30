#include "geo_div.h"
#include "map_state.h"
#include <nlohmann/json.hpp>
#include "csv.hpp"
#include <iostream>
#include <fstream>

#include <map>


void print_properties_map(std::map<std::string, std::vector<std::string>> properties_map) {
  int i = 0;
  for (auto [key, value_vec] : properties_map) {
    i++;
    std::cout << i << ". " << key << ": ";
    for (std::string value : value_vec) {
      std::cout << value << ", ";
    }
    std::cout << std::endl;
    std::cout << std::endl;
  }
}

void geojson_to_csv(const std::string geometry_file_name)
{
  // Open file
  std::ifstream in_file(geometry_file_name);

  // Parse JSON
  nlohmann::json j;
  in_file >> j;

  // Declare map of properties and their values
  std::map<std::string, std::vector<std::string>> properties_map;

  std::set<std::string> ids_in_geojson;
  for (auto feature : j["features"]) {
    for (auto property_item : feature["properties"].items()) {
      auto key = property_item.key();
      auto value = property_item.value();
      auto v = properties_map[key];
      
      // If the value == ""
      if (value == "" || value.is_null()) {
        continue;
        // If the map does not contain the key
      } else if (properties_map.count(key) == 0) {
        properties_map[key] = {value};
        // If the value is already inside the vector
      } else if (std::find(v.begin(), v.end(), value) != v.end()) {
        continue;
        // If the map contains the key
      } else {
        properties_map[key].push_back(value);
      }
    }
  }

  for (auto [key, value_arr] : properties_map) {
    if (value_arr.size() < j["features"].size()) {
      properties_map.erase(key);
    }
  }

  // Display available unique identifiers
  std::cout << "Here are the available unique identifiers and their values. ";
  std::cout << "Please select one by entering the respective number." << std::endl;
  print_properties_map(properties_map);
  std::cout << std::endl;

  // Get user input
  std::cout << "Please enter your number here: ";
  int chosen_number;
  std::cin >> chosen_number;
  std::cout << std::endl;

  // Declare chosen identifier
  std::map<std::string, std::vector<std::string>> chosen_identifiers;
  int i = 0;
  for (auto [key, value_vec] : properties_map) {
    i++;
    if (chosen_number == i) {
      chosen_identifiers[key] = value_vec;
        break;
    }
  }

  print_properties_map(chosen_identifiers);
  std::cout << std::endl;

  /*
  std::ofstream csv;
  csv.open("template_with_identifiers.csv");
  auto csv_writer = csv::make_csv_writer(csv);

  std::vector<std::vector<std::string>> id_tuples(chosen_identifiers.begin()->second.size() + 1);

  for (auto const& [column_name, ids] : chosen_identifiers)
  {

    id_tuples[0].push_back(column_name);
    for (size_t i = 0; i < ids.size(); i++) {
      id_tuples[i + 1].push_back(ids[i]);
    }
  }

  for (auto id_tuple : id_tuples) {
    csv_writer << id_tuple;
  }

  csv.close();
  */
}
