#include "cartogram_info.h"

CartogramInfo::CartogramInfo(std::string v, const bool w, const bool wd2eps) :
  visual_variable_file_(v),
  is_world_map_(w),
  write_density_to_eps_(wd2eps)
{
  return;
}

void CartogramInfo::set_id_header(const std::string id)
{
  id_header_ = id;
  return;
}

const std::string CartogramInfo::id_header() const
{
  return id_header_;
}

const std::string CartogramInfo::visual_variable_file() const
{
  return visual_variable_file_;
}

void CartogramInfo::insert_id_in_visual_variables_file(const std::string id)
{
  ids_in_visual_variables_file_.insert(id);
}

const std::set<std::string> CartogramInfo::ids_in_visual_variables_file() const
{
  return ids_in_visual_variables_file_;
}

bool CartogramInfo::is_world_map() const
{
  return is_world_map_;
}

bool CartogramInfo::trigger_write_density_to_eps() const
{
  return write_density_to_eps_;
}

void CartogramInfo::set_map_name(std::string map_name)
{
  map_name_ = map_name;
}

int CartogramInfo::n_insets() const
{
  return inset_states_.size();
}

const std::vector<InsetState> CartogramInfo::inset_states() const
{
  return inset_states_;
}

std::vector<InsetState> *CartogramInfo::ref_to_inset_states()
{
  return &inset_states_;
}

void CartogramInfo::push_back(const InsetState is)
{
  inset_states_.push_back(is);
  return;
}

void CartogramInfo::gd_to_inset_insert(const std::string id,
                                       const std::string inset)
{
  gd_to_inset_.insert(std::pair<std::string, std::string>(id, inset));
  return;
}

const std::string CartogramInfo::inset_at_gd(const std::string id) const
{
  if (gd_to_inset_.count(id)) {
    return gd_to_inset_.at(id);
  }
  std::cout << "No GeoDiv: " << id << std::endl;
  return "null";
}

 void CartogramInfo::csv_row_to_gd_insert(const std::string csv_row,
                                          const std::string id)
 {
   csv_row_to_gd_.insert(std::pair<std::string, std::string>(csv_row, id));
   return;
 }

 const std::string CartogramInfo::gd_at_csv_row(std::string csv_row) const
 {
   return csv_row_to_gd_.at(csv_row);
 }
