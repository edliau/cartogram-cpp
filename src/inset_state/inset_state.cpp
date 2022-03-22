#include "../inset_state.h"
#include "../constants.h"

InsetState::InsetState(std::string pos) : pos_(pos)
{
  n_finished_integrations_ = 0;
  return;
}

double InsetState::area_error_at(const std::string id) const
{
  return area_errors_.at(id);
}

Bbox InsetState::bbox() const
{
  // Find joint bounding for all polygons with holes in this inset
  double inset_xmin = dbl_inf;
  double inset_xmax = -dbl_inf;
  double inset_ymin = dbl_inf;
  double inset_ymax = -dbl_inf;
  for (const auto &gd : geo_divs_) {
    for (const auto &pwh : gd.polygons_with_holes()) {
      const auto bb = pwh.bbox();
      inset_xmin = std::min(bb.xmin(), inset_xmin);
      inset_ymin = std::min(bb.ymin(), inset_ymin);
      inset_xmax = std::max(bb.xmax(), inset_xmax);
      inset_ymax = std::max(bb.ymax(), inset_ymax);
    }
  }
  Bbox inset_bb(inset_xmin, inset_ymin, inset_xmax, inset_ymax);
  return inset_bb;
}

bool InsetState::color_found(const std::string id) const
{
  return colors_.count(id);
}

const Color InsetState::color_at(const std::string id) const
{
  return colors_.at(id);
}

bool InsetState::colors_empty() const
{
  return colors_.empty();
}

unsigned int InsetState::colors_size() const
{
  return colors_.size();
}

void InsetState::destroy_fftw_plans_for_rho()
{
  fftw_destroy_plan(fwd_plan_for_rho_);
  fftw_destroy_plan(bwd_plan_for_rho_);
  return;
}

void InsetState::execute_fftw_bwd_plan() const
{
  fftw_execute(bwd_plan_for_rho_);
  return;
}

void InsetState::execute_fftw_fwd_plan() const
{
  fftw_execute(fwd_plan_for_rho_);
  return;
}

const std::vector<GeoDiv> InsetState::geo_divs() const
{
  return geo_divs_;
}

const std::vector<GeoDiv> InsetState::geo_divs_original() const
{
  return geo_divs_original_;
}

void InsetState::increment_integration()
{
  n_finished_integrations_ += 1;
  return;
}

void InsetState::initialize_cum_proj()
{
  cum_proj_.resize(boost::extents[lx_][ly_]);
  for (unsigned int i = 0; i < lx_; ++i) {
    for (unsigned int j = 0; j < ly_; ++j) {
      cum_proj_[i][j].x = i + 0.5;
      cum_proj_[i][j].y = j + 0.5;
    }
  }
}

void InsetState::initialize_original_proj()
{
  original_proj_.resize(boost::extents[lx_][ly_]);
  for (unsigned int i = 0; i < lx_; ++i) {
    for (unsigned int j = 0; j < ly_; ++j) {
      original_proj_[i][j].x = i + 0.5;
      original_proj_[i][j].y = j + 0.5;
    }
  }
  return;
}

void InsetState::insert_color(const std::string id, const Color c)
{
  if (colors_.count(id)) {
    colors_.erase(id);
  }
  colors_.insert(std::pair<std::string, Color>(id, c));
  return;
}

void InsetState::insert_color(const std::string id, std::string color)
{
  if (colors_.count(id)) {
    colors_.erase(id);
  }

  // From https://stackoverflow.com/questions/313970/how-to-convert-stdstring-
  // to-lower-case
  std::transform(color.begin(), color.end(), color.begin(), ::tolower);
  const Color c(color);
  colors_.insert(std::pair<std::string, Color>(id, c));
  return;
}

void InsetState::insert_label(const std::string id, const std::string label)
{
  labels_.insert(std::pair<std::string, std::string>(id, label));
  return;
}

void InsetState::insert_target_area(const std::string id, const double area)
{
  target_areas_.insert(std::pair<std::string, double>(id, area));
  return;
}

 std::vector<std::vector<intersection>>
 InsetState::intersections_of_rays_with_original_geodivs() const {
   return original_intersections_;
 }

void InsetState::insert_whether_input_target_area_is_missing(
  const std::string id,
  const bool is_missing)
{
  is_input_target_area_missing_.insert(
    std::pair<std::string, bool>(id, is_missing));
  return;
}

const std::string InsetState::inset_name() const
{
  return inset_name_;
}

bool InsetState::is_input_target_area_missing(const std::string id) const
{
  return is_input_target_area_missing_.at(id);
}

double InsetState::latt_const() const
{
  return latt_const_;
}

unsigned int InsetState::lx() const
{
  return lx_;
}

unsigned int InsetState::ly() const
{
  return ly_;
}

void InsetState::make_fftw_plans_for_rho()
{
  fwd_plan_for_rho_ =
    fftw_plan_r2r_2d(lx_, ly_,
                     rho_init_.as_1d_array(), rho_ft_.as_1d_array(),
                     FFTW_REDFT10, FFTW_REDFT10, FFTW_ESTIMATE);
  bwd_plan_for_rho_ =
    fftw_plan_r2r_2d(lx_, ly_,
                     rho_ft_.as_1d_array(), rho_init_.as_1d_array(),
                     FFTW_REDFT01, FFTW_REDFT01, FFTW_ESTIMATE);
  return;
}

struct max_area_error_info InsetState::max_area_error() const
{
  double value = -dbl_inf;
  std::string worst_gd = "";
  for (const auto &[gd_id, area_error] : area_errors_) {
    if (area_error > value) {
      value = area_error;
      worst_gd = gd_id;
    }
  }
  return {value, worst_gd};
}

unsigned int InsetState::n_finished_integrations() const
{
  return n_finished_integrations_;
}

unsigned int InsetState::n_geo_divs() const
{
  return geo_divs_.size();
}


unsigned long InsetState::n_points() const
{
  unsigned long n_pts = 0;
  for (const auto &gd : geo_divs_) {
    n_pts += gd.n_points();
  }
  return n_pts;
}

unsigned int InsetState::n_rings() const
{
  unsigned int n_rings = 0;
  for (const auto &gd: geo_divs_) {
    n_rings += gd.n_rings();
  }
  return n_rings;
}

const std::string InsetState::pos() const
{
  return pos_;
}

void InsetState::push_back(const GeoDiv gd)
{
  geo_divs_.push_back(gd);
  return;
}

boost::multi_array<XYPoint, 2> *InsetState::ref_to_cum_proj()
{
  return &cum_proj_;
}

boost::multi_array<XYPoint, 2> *InsetState::ref_to_original_proj()
{
  return &original_proj_;
}

std::vector<GeoDiv> *InsetState::ref_to_geo_divs()
{
  return &geo_divs_;
}

std::vector<GeoDiv> *InsetState::ref_to_geo_divs_original()
{
  return &geo_divs_original_;
}

boost::multi_array<int, 2> *InsetState::ref_to_graticule_diagonals()
{
  return &graticule_diagonals_;
}

boost::multi_array<XYPoint, 2> *InsetState::ref_to_proj()
{
  return &proj_;
}

FTReal2d *InsetState::ref_to_rho_ft()
{
  return &rho_ft_;
}

FTReal2d *InsetState::ref_to_rho_init()
{
  return &rho_init_;
}

void InsetState::replace_target_area(const std::string id, const double area)
{
  target_areas_[id] = area;
  return;
}

void InsetState::set_area_errors()
{
  // Formula for relative area error:
  // area_on_cartogram / target_area - 1
  double sum_target_area = 0.0;
  double sum_cart_area = 0.0;
  for (const auto &gd : geo_divs_) {
    sum_target_area += target_area_at(gd.id());
    sum_cart_area += gd.area();
  }
  for (const auto &gd : geo_divs_) {
    const double obj_area =
      target_area_at(gd.id()) * sum_cart_area / sum_target_area;
    area_errors_[gd.id()] = std::abs((gd.area() / obj_area) - 1);
  }
  return;
}

void InsetState::set_geo_divs(const std::vector<GeoDiv> geo_divs_new)
{
  geo_divs_.clear();
  geo_divs_ = geo_divs_new;
  return;
}

void InsetState::set_grid_dimensions(
  const unsigned int lx, const unsigned int ly)
{
  lx_ = lx;
  ly_ = ly;
  return;
}

void InsetState::set_inset_name(const std::string inset_name)
{
  inset_name_ = inset_name;
  return;
}

void InsetState::set_latt_const(const double latt_const)
{
  latt_const_ = latt_const;
  return;
}

void InsetState::set_pos(const std::string pos)
{
  pos_ = pos;
  return;
}

void InsetState::store_original_geo_divs()
{
  for (const auto &gd : geo_divs_) {
    GeoDiv gd_original(gd.id()); // + "_original");
    for(auto pwh : gd.polygons_with_holes()) {
      gd_original.push_back(pwh);
    }
    geo_divs_original_.push_back(gd_original);
  }

  original_intersections_ =
    intersections_with_rays_parallel_to_axis('x', 1);
}

bool InsetState::target_area_is_missing(const std::string id) const
{
  // We use negative area as indication that GeoDiv has no target area
  return target_areas_.at(id) < 0.0;
}

double InsetState::target_area_at(const std::string id) const
{
  return target_areas_.at(id);
}

double InsetState::total_inset_area() const
{
  double total_inset_area = 0.0;
  for (const auto &gd : geo_divs_) {
    total_inset_area += gd.area();
  }
  return total_inset_area;
}

double InsetState::total_target_area() const
{
  double inset_total_target_area = 0;
  for (const auto &geo_div_target_area : target_areas_) {
    inset_total_target_area += geo_div_target_area.second;
  }
  return inset_total_target_area;
}

std::string InsetState::label_at(const std::string id) const
{
  if (labels_.find(id) == labels_.end()) {
    return "";
  }
  return labels_.at(id);
}
