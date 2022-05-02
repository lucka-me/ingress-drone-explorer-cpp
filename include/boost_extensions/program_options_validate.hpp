#pragma once

#include <boost/any.hpp>
#include <vector>

#include "type/LngLat.hpp"

namespace ingress_drone_explorer {

void validate(boost::any& v, const std::vector<std::string>& values, LngLat*, int);

} // namespace ingress_drone_explorer