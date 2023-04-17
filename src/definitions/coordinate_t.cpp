#include "definitions/coordinate_t.hpp"

#include <cmath>

namespace ingress_drone_explorer {

double coordinate_t::distance_to(const coordinate_t& other) const {
    const auto sin_theta = std::sin((other.theta() - theta()) / 2);
    const auto sin_phi = std::sin((other.phi() - phi()) / 2);
    const auto a = sin_phi * sin_phi + sin_theta * sin_theta * std::cos(phi()) * std::cos(other.phi());
    return std::atan2(std::sqrt(a), std::sqrt(1.0 - a)) * 2 * _earth_radius;
}

double coordinate_t::distance_to(const coordinate_t& a, const coordinate_t& b) const {
    const auto c_1 = (b._lat - a._lat) * (_lat - a._lat)
        + (b._lng - a._lng) * (_lng - a._lng);
    if (c_1 <= 0) {
        return distance_to(a);
    }
    const auto c_2 = (b._lat - a._lat) * (b._lat - a._lat)
        + (b._lng - a._lng) * (b._lng - a._lng);
    if (c_2 <= c_1) {
        return distance_to(b);
    }
    const auto ratio = c_1 / c_2;
    return distance_to({ a._lng + ratio * (b._lng - a._lng), a._lat + ratio * (b._lat - a._lat) });
}

bool coordinate_t::closer(const coordinate_t& a, const coordinate_t& b) const {
    const auto d_a = (_lng - a._lng) * (_lng - a._lng) + (_lat - a._lat) * (_lat - a._lat);
    const auto d_b = (_lng - b._lng) * (_lng - b._lng) + (_lat - b._lat) * (_lat - b._lat);
    return d_a < d_b;
}

} // namespace ingress_drone_explorer