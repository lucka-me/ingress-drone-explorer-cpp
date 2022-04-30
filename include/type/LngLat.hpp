#pragma once

#include <cmath>

namespace ingress_drone_explorer {

struct LngLat {
    double m_lng = 0;
    double m_lat = 0;

    inline LngLat() = default;
    inline LngLat(const double lng, const double lat) {
        m_lng = lng;
        m_lat = lat;
    }

    inline double theta() const {
        return m_lng * M_PI / 180.0;
    }

    inline double phi() const {
        return m_lat * M_PI / 180.0;
    }

    double distanceTo(const LngLat& other) const;
    double distanceTo(const LngLat& a, const LngLat& b) const;
    bool closer(const LngLat& a, const LngLat& b) const;

private:
    static constexpr double s_earthRadius = 6371008.8;
};

} // namespace ingress_drone_explorer