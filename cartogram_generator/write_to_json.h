#ifndef WRITE_TO_JSON_H_
#define WRITE_TO_JSON_H_

#include "cartogram_info.h"
#include <nlohmann/json.hpp>

nlohmann::json inset_to_json(InsetState*);
std::vector<double> divider_points(double, double, double, double);
nlohmann::json cgal_to_json(CartogramInfo *cart_info);
void write_to_json(nlohmann::json,
                   std::string,
                   std::string,
                   std::ostream&,
                   bool,
                   CartogramInfo *cart_info);
#endif
