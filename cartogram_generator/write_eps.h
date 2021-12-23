#ifndef WRITE_EPS_H_
#define WRITE_EPS_H_

void write_map_to_eps(std::string, bool, InsetState*);
void write_density_to_eps(std::string, double*, InsetState*);
void write_intersections_to_eps(std::string, InsetState*);

#endif
