#pragma once

#include <array>
#include <functional>
#include <set>

#include "definitions/coordinate_t.hpp"

namespace ingress_drone_explorer {

namespace s2 {

struct ecef_coordinate_t {
    double _x;
    double _y;
    double _z;

    inline ecef_coordinate_t(const coordinate_t& coordinate) {
        const auto theta = coordinate.theta();
        const auto phi = coordinate.phi();
        const auto cos_phi = std::cos(phi);
        _x = std::cos(theta) * cos_phi;
        _y = std::sin(theta) * cos_phi;
        _z = std::sin(phi);
    }

    inline ecef_coordinate_t(const uint8_t face, const double s, const double t) {
        const auto u = (1.0 / 3.0) * (s >= 0.5 ? (4.0 * s * s - 1) : (1.0 - (4.0 * (1.0 - s) * (1.0 - s))));
        const auto v = (1.0 / 3.0) * (t >= 0.5 ? (4.0 * t * t - 1) : (1.0 - (4.0 * (1.0 - t) * (1.0 - t))));
        switch (face) {
        case 0: _x =  1; _y =  u; _z =  v; return;
        case 1: _x = -u; _y =  1; _z =  v; return;
        case 2: _x = -u; _y = -v; _z =  1; return;
        case 3: _x = -1; _y = -v; _z = -u; return;
        case 4: _x =  v; _y = -1; _z = -u; return;
        case 5: _x =  v; _y =  u; _z = -1; return;
        }
    }

    inline coordinate_t coordinate() const {
        return coordinate_t(
            std::atan2(_y, _x) / std::numbers::pi * 180.0,
            std::atan2(_z, std::hypot(_x, _y)) / std::numbers::pi * 180.0
        );
    }

    inline void face_s_t(uint8_t& face, double& s, double& t) const {
        const auto abs_x = std::abs(_x);
        const auto abs_y = std::abs(_y);
        face = abs_x > abs_y ? (abs_x > std::abs(_z) ? 0 : 2) : (abs_y > std::abs(_z) ? 1 : 2);
        if ((face == 0 && _x < 0) || (face == 1 && _y < 0) || (face == 2 && _z < 0)) {
            face += 3;
        }
        // (s, t) as (u, v)
        switch (face) {
        case 0: s =  _y / _x; t =  _z / _x; break;
        case 1: s = -_x / _y; t =  _z / _y; break;
        case 2: s = -_x / _z; t = -_y / _z; break;
        case 3: s =  _z / _x; t =  _y / _x; break;
        case 4: s =  _z / _y; t = -_x / _y; break;
        case 5: s = -_y / _z; t = -_x / _z; break;
        }

        s = s >= 0 ? (0.5 * std::sqrt(1 + 3 * s)) : (1.0 - 0.5 * std::sqrt(1 - 3 * s));
        t = t >= 0 ? (0.5 * std::sqrt(1 + 3 * t)) : (1.0 - 0.5 * std::sqrt(1 - 3 * t));
    }
};

} // namespace s2

} // namespace ingress_drone_explorer