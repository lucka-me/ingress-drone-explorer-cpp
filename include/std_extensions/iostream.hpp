#pragma once

#include <iostream>

#include "type/LngLat.hpp"
#include "s2.hpp"

namespace ingress_drone_explorer {

static inline std::istream& operator>>(std::istream& in, LngLat& value) {
    std::string component;
    std::getline(in, component, ',');
    value.m_lng = std::stod(component);
    std::getline(in, component);
    value.m_lat = std::stod(component);
    return in;
}

static inline std::ostream& operator<<(std::ostream& out, const LngLat& value) {
    return out << value.m_lng << "," << value.m_lat;
}

namespace s2
{

static inline std::ostream& operator<<(std::ostream& out, const Cell& value) {
    return out
        << +value.m_face
        << "," << +value.m_level
        << "," << value.m_i
        << "," << value.m_j;
}

} // namespace s2

} // namespace ingress_drone_explorer
