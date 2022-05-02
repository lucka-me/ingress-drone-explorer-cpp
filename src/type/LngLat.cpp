#include "type/LngLat.hpp"

#include <cmath>

namespace ingress_drone_explorer {

double LngLat::distanceTo(const LngLat& other) const {
    const auto sinT = std::sin((other.theta() - theta()) / 2);
    const auto sinP = std::sin((other.phi() - phi()) / 2);
    const auto a = sinP * sinP + sinT * sinT * std::cos(phi()) * std::cos(other.phi());
    return std::atan2(std::sqrt(a), std::sqrt(1.0 - a)) * 2 * s_earthRadius;
}

double LngLat::distanceTo(const LngLat& a, const LngLat& b) const {
    const auto c1 = (b.m_lat - a.m_lat) * (m_lat - a.m_lat)
        + (b.m_lng - a.m_lng) * (m_lng - a.m_lng);
    if (c1 <= 0) {
        return distanceTo(a);
    }
    const auto c2 = (b.m_lat - a.m_lat) * (b.m_lat - a.m_lat)
        + (b.m_lng - a.m_lng) * (b.m_lng - a.m_lng);
    if (c2 <= c1) {
        return distanceTo(b);
    }
    const auto ratio = c1 / c2;
    return distanceTo(
        LngLat(
            a.m_lng + ratio * (b.m_lng - a.m_lng),
            a.m_lat + ratio * (b.m_lat - a.m_lat)
        )
    );
}

bool LngLat::closer(const LngLat& a, const LngLat& b) const {
    const auto dA = (m_lng - a.m_lng) * (m_lng - a.m_lng) + (m_lat - a.m_lat) * (m_lat - a.m_lat);
    const auto dB = (m_lng - b.m_lng) * (m_lng - b.m_lng) + (m_lat - b.m_lat) * (m_lat - b.m_lat);
    return dA < dB;
}

} // namespace ingress_drone_explorer