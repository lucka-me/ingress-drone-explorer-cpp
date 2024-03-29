#pragma once

#include <iostream>

namespace ingress_drone_explorer {

struct coordinate_t;

std::istream& operator>>(std::istream& in, coordinate_t& value);
std::ostream& operator<<(std::ostream& out, const coordinate_t& value);

namespace s2 {

struct cell_t;

std::ostream& operator<<(std::ostream& out, const cell_t& value);

} // namespace s2

} // namespace ingress_drone_explorer
