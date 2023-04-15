#pragma once

#include <vector>

namespace boost { class any; }

namespace ingress_drone_explorer {

struct coordinate_t;

void validate(boost::any& v, const std::vector<std::string>& values, coordinate_t*, int);

} // namespace ingress_drone_explorer