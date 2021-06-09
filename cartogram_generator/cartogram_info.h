#ifndef CARTOGRAM_INFO_H_
#define CARTOGRAM_INFO_H_

#include "inset_state.h"
#include <vector>

class CartogramInfo {
private:
  std::vector<InsetState> inset_states_;
  std::map<std::string, std::string> gd_to_inset_;
  std::map<std::string, std::string> csv_row_to_gd_;
  std::string id_header_;
  std::string visual_variable_file_;
  std::set<std::string> ids_in_visual_variables_file_;
  bool is_world_map_;
  bool write_density_to_eps_;
  std::string map_name_;
public:
  explicit CartogramInfo(const std::string, const bool, const bool);
  void set_id_header(const std::string);
  void insert_id_in_visual_variables_file(const std::string);
  const std::set<std::string> ids_in_visual_variables_file() const;
  const std::string id_header() const;
  const std::string visual_variable_file() const;
  bool is_world_map() const;
  bool trigger_write_density_to_eps() const;
  void set_map_name(std::string map_name);
  int n_insets() const;
  const std::vector<InsetState> inset_states() const;
  std::vector<InsetState> *ref_to_inset_states();
  void push_back(const InsetState);
  void gd_to_inset_insert(const std::string, const std::string);
  const std::string inset_at_gd(const std::string) const;
  void csv_row_to_gd_insert(const std::string, const std::string);
  const std::string gd_at_csv_row(std::string) const;
};
#endif
