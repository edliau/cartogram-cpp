#ifndef MAP_STATE_H_
#define MAP_STATE_H_

#include "ft_real_2d_array.h"
#include "geo_div.h"
#include <fftw3.h>
#include <vector>

class MapState {
private:
  bool world;
  std::vector<GeoDiv> geo_divs;
  int lx, ly;  // Lattice dimensions
  FTReal2dArray rho_init;  // Raterized density
  FTReal2dArray rho_ft;  // Fourier transform
  fftw_plan plan_fwd, plan_bwd;  // Plan the Fourier transform
  MapState();
public:
  explicit MapState(const bool);
  //~MapState();
  int n_geo_divs() const;
  std::vector<GeoDiv> get_geo_divs() const;
  std::vector<GeoDiv> *ref_to_geo_divs();
  bool is_world_map() const;
  void make_grid(const unsigned int, const unsigned int);
  int get_lx();
  int get_ly();
  double *get_rho_init();
  double *get_rho_ft();
  fftw_plan get_plan_fwd();
  void free_rho();
  void push_back(const GeoDiv);
};

#endif
