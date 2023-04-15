#pragma once

#include <string>

#include "coordinate_t.hpp"

namespace ingress_drone_explorer {

struct portal_t {
    std::string     _guid = "";
    std::string     _title = "";
    coordinate_t    _coordinate;

    inline portal_t() = default;

    inline auto operator==(const portal_t& other) const {
        return _guid == other._guid;
    }

    inline auto operator<(const portal_t& other) const {
        return _guid < other._guid;
    }
};

static inline auto operator<(const portal_t& a, const std::string& b) {
    return a._guid < b;
}

static inline auto operator<(const std::string& a, const portal_t& b) {
    return a < b._guid;
}

} // namespace ingress_drone_explorer