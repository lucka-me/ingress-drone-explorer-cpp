#pragma once

#include <string>
#include <vector>

#include "LngLat.hpp"

namespace ingress_drone_explorer {

struct DrawnItem {
    std::string         m_type  = "polygon";
    std::string         m_color;
    std::vector<LngLat> m_lngLats;

    inline DrawnItem(const std::string& color, const std::vector<LngLat>& lngLats) {
        m_color = color;
        m_lngLats = lngLats;
    }
};

} // namespace ingress_drone_explorer