#include "constants.h"
#include "cartogram_info.h"
#include "inset_state.h"
#include <iostream>

void InsetState::blur_density(const double blur_width,
                              bool plot_density)
{
  const unsigned int lx = this->lx();
  const unsigned int ly = this->ly();
  const double prefactor = -0.5 * blur_width * blur_width * pi * pi;
  for (unsigned int i = 0; i<lx; ++i) {
    const double scaled_i = static_cast<double>(i) / lx;
    const double scaled_i_squared = scaled_i * scaled_i;
    for (unsigned int j = 0; j<ly; ++j) {
      const double scaled_j = static_cast<double>(j) / ly;
      const double scaled_j_squared = scaled_j * scaled_j;
      this->rho_ft_(i, j) *=
        exp(prefactor * (scaled_i_squared + scaled_j_squared)) / (4*lx*ly);
    }
  }
  this->execute_fftw_bwd_plan();
  if (plot_density) {
    std::string file_name =
      this->pos() +
      "_blurred_density_" +
      std::to_string(this->n_finished_integrations()) +
      ".eps";
    std::cerr << "Writing " << file_name << std::endl;
    this->write_density_to_eps(file_name);
  }
  return;
}
