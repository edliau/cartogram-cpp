#include "geo_div.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

void print(std::map<std::string, std::vector<std::string> > properties_map)
{
  int i = 0;
  for (auto [key, value_vec] : properties_map)
  {
    i++;
    std::cout << i << ". " << key << ": ";
    for (std::string value : value_vec)
    {
      std::cout << value << ", ";
    }
    std::cout << std::endl
              << std::endl;
  }
}

void geojson_to_csv(const std::string geometry_file_name)
{
  std::ifstream in_file(geometry_file_name);

  // Parse JSON
  nlohmann::json j;
  in_file >> j;

  // Declare map of properties (unique identifiers) and their values
  std::map<std::string, std::vector<std::string> > properties_map;

  // Find all possible identifiers (key-value pairs where the key is present in every feature and the value is unique for every feature)

  for (auto feature : j["features"])
  {
    for (auto property_item : feature["properties"].items())
    {
      auto key = property_item.key();
      auto value = property_item.value();
      auto v = properties_map[key];

      if (value == "")
      {
        continue;
      }
      else if (std::find(v.begin(), v.end(), value) != v.end())
      {
        continue;
      }
      else
      {
        properties_map[key].push_back(value);
      }
    }
  }

  std::map<std::string, std::vector<std::string> > viable_properties_map = properties_map;
  for (auto [key, value_arr] : properties_map)
  {
    if (value_arr.size() < j["features"].size())
    {
      viable_properties_map.erase(key);
    }
  }

  // Present user with all possible identifiers and a few examples
  std::cout << "Please select one by entering the respoctive number." << std::endl;
  print(viable_properties_map);
  std::cout << viable_properties_map.size() + 1 << ". All" << std::endl
            << std::endl;

  // Have the user choose which key they want to use as an identifier
  std::cout << "Please enter your number here: ";
  int chosen_number;
  std::cin >> chosen_number;
  std::cout << std::endl;

  std::map<std::string, std::vector<std::string>> chosen_identifiers;
  int i = 0;
  for (auto [key, value_vec] : viable_properties_map) {
    i++;
    if (chosen_number == i || chosen_number == viable_properties_map.size() + 1) {
      chosen_identifiers[key] = value_vec;
    }
  }
  print(chosen_identifiers);

  return;
}
