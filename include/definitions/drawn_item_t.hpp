#pragma once

#include <string>
#include <vector>

#include "coordinate_t.hpp"

namespace ingress_drone_explorer {

struct drawn_item_t {
    std::string                 _type           = "polygon";
    std::string                 _color;
    std::vector<coordinate_t>   _coordinates;

    inline drawn_item_t(const std::string& color, const std::vector<coordinate_t>& coordinates) {
        _color = color;
        _coordinates = coordinates;
    }
};

} // namespace ingress_drone_explorer