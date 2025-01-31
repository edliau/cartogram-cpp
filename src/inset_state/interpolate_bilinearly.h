#ifndef INTERPOL_H_
#define INTERPOL_H_

#include <boost/multi_array.hpp>

double interpolate_bilinearly(double,
                              double,
                              const boost::multi_array<double, 2>*,
                              char,
                              const unsigned int,
                              const unsigned int);

#endif
