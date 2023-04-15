#pragma once

#include <numbers>

namespace ingress_drone_explorer {

struct coordinate_t {
public:
    double _lng = 0;
    double _lat = 0;

public:
    inline coordinate_t() = default;
    inline coordinate_t(const double lng, const double lat) {
        _lng = lng;
        _lat = lat;
    }

public:
    inline double theta() const {
        return _lng * std::numbers::pi / 180.0;
    }

    inline double phi() const {
        return _lat * std::numbers::pi / 180.0;
    }

    double distance_to(const coordinate_t& other) const;
    double distance_to(const coordinate_t& a, const coordinate_t& b) const;
    bool closer(const coordinate_t& a, const coordinate_t& b) const;

private:
    static constexpr double _earth_radius = 6371008.8;
};

} // namespace ingress_drone_explorer