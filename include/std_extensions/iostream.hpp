#pragma once

#include <iostream>

#include "type/coordinate_t.hpp"
#include "s2.hpp"

namespace ingress_drone_explorer {

static inline std::istream& operator>>(std::istream& in, coordinate_t& value) {
    std::string component;
    std::getline(in, component, ',');
    value._lng = std::stod(component);
    std::getline(in, component);
    value._lat = std::stod(component);
    return in;
}

static inline std::ostream& operator<<(std::ostream& out, const coordinate_t& value) {
    return out << value._lng << "," << value._lat;
}

namespace s2
{

static inline std::ostream& operator<<(std::ostream& out, const cell_t& value) {
    return out
        << +value._face
        << "," << +value._level
        << "," << value._i
        << "," << value._j;
}

} // namespace s2

} // namespace ingress_drone_explorer
