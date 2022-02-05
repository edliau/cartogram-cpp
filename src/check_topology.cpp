#include "cartogram_info.h"
#include "inset_state.h"
#include <CGAL/Boolean_set_operations_2.h>

// Returns error if there are holes not inside their respective polygons
void holes_inside_polygons(InsetState *inset_state)
{
  for (auto gd : inset_state->geo_divs()) {
    for (auto pwh : gd.polygons_with_holes()) {
      Polygon ext_ring = pwh.outer_boundary();
      for (auto h = pwh.holes_begin(); h != pwh.holes_end(); ++h) {
        Polygon hole = *h;
        for (unsigned int i = 0; i < hole.size(); ++i) {

          // TODO: In the future, a better method would be to only check
          // whether one point in each hole is on the bounded side of the
          // exterior ring. Next, check whether the hole intersects the
          // polygon at any point. For this, the function
          // "do_intersect(Polygon, Polygon)" may help.
          if (ext_ring.bounded_side(hole[i]) == CGAL::ON_UNBOUNDED_SIDE) {
            CGAL::set_pretty_mode(std::cerr);
            std::cerr << "Hole detected outside polygon!" << std::endl;
            std::cerr << "Hole: " << hole << std::endl;
            std::cerr << "Polygon: " << ext_ring << std::endl;
            std::cerr << "GeoDiv: " << gd.id() << std::endl;
            _Exit(20);
          }
        }
      }
    }
  }
  return;
}
