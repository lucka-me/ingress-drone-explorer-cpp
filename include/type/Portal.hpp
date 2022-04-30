#pragma once

#include <string>

#include "LngLat.hpp"

namespace ingress_drone_explorer {

struct Portal {
    std::string m_guid = "";
    std::string m_title = "";
    LngLat      m_lngLat;

    inline Portal() = default;

    inline auto operator==(const Portal& other) const {
        return m_guid == other.m_guid;
    }

    inline auto operator<(const Portal& other) const {
        return m_guid < other.m_guid;
    }
};

static inline auto operator<(const Portal& a, const std::string& b) {
    return a.m_guid < b;
}

static inline auto operator<(const std::string& a, const Portal& b) {
    return a < b.m_guid;
}

} // namespace ingress_drone_explorer