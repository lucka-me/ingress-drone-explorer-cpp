#include "s2.hpp"

#include <algorithm>
#include <cmath>

#include "type/coordinate_t.hpp"

namespace ingress_drone_explorer {

namespace s2 {

cell_t::cell_t(const coordinate_t& lngLat, const uint8_t level) {
    _level = level;
    double s, t;
    ecef_coordinate_t(lngLat).face_s_t(_face, s, t);
    const int32_t max = 1 << level;
    _i = std::clamp(static_cast<decltype(_i)>(std::floor(s * max)), 0, max - 1);
    _j = std::clamp(static_cast<decltype(_j)>(std::floor(t * max)), 0, max - 1);
}

std::set<cell_t> cell_t::neighboured_cells_covering_cap_of(const coordinate_t& center, const double radius) const
{
    std::set<cell_t> result;
    std::set<cell_t> outside;
    std::set<cell_t> queue { *this };
    for (auto it = queue.begin(); it != queue.end(); it = queue.begin()) {
        const auto cell = *it;
        queue.erase(it);
        if (result.contains(cell) || outside.contains(cell)) {
            continue;
        }
        if (cell.intersects_with_cap_of(center, radius)) {
            queue.merge(cell.neighbours());
            result.insert(std::move(cell));
        } else {
            outside.insert(std::move(cell));
        }
    }
    return std::move(result);
}

std::array<coordinate_t, 4> cell_t::shape() const {
    return {
        coordinate(0, 0),
        coordinate(0, 1),
        coordinate(1, 1),
        coordinate(1, 0),
    };
}

inline cell_t::cell_t(const uint8_t face, const double i, const double j, uint8_t level)
{
    _level = level;
    const int32_t max = 1 << level;
    if (i >= 0 && j >= 0 && i < max && j < max) {
        _face = face;
        _i = i;
        _j = j;
        return;
    }
    double s, t;
    ecef_coordinate_t(face, (0.5 + i) / max, (0.5 + j) / max).face_s_t(_face, s, t);
    _i = std::clamp(static_cast<decltype(_i)>(std::floor(s * max)), 0, max - 1);
    _j = std::clamp(static_cast<decltype(_j)>(std::floor(t * max)), 0, max - 1);
}

inline bool cell_t::intersects_with_cap_of(const coordinate_t& center, const double radius) const {
    auto corners = shape();
    std::sort(
        corners.begin(), corners.end(),
        [&](const auto& a, const auto& b) {
            return center.closer(a, b);
        }
    );
    return center.distance_to(corners[0]) < radius || center.distance_to(corners[0], corners[1]) < radius;
}

inline coordinate_t cell_t::coordinate(const double d_i, const double d_j) const
{
    const double max = 1 << _level;
    return std::move(
        ecef_coordinate_t(
            _face,
            (d_i + _i) / max,
            (d_j + _j) / max
        )
        .coordinate()
    );
}

inline std::set<cell_t> cell_t::neighbours() const {
    return {
        cell_t(_face, _i - 1, _j       , _level),
        cell_t(_face, _i    , _j - 1   , _level),
        cell_t(_face, _i + 1, _j + 1   , _level),
        cell_t(_face, _i    , _j       , _level),
    };
}

inline ecef_coordinate_t::ecef_coordinate_t(const coordinate_t& lngLat) {
    const auto theta = lngLat.theta();
    const auto phi = lngLat.phi();
    const auto cosPhi = std::cos(phi);
    _x = std::cos(theta) * cosPhi;
    _y = std::sin(theta) * cosPhi;
    _z = std::sin(phi);
}

inline ecef_coordinate_t::ecef_coordinate_t(const uint8_t face, const double s, const double t)
{
    const auto u = (1.0 / 3.0) * (s >= 0.5 ? (4.0 * s * s - 1) : (1.0 - (4.0 * (1.0 - s) * (1.0 - s))));
    const auto v = (1.0 / 3.0) * (t >= 0.5 ? (4.0 * t * t - 1) : (1.0 - (4.0 * (1.0 - t) * (1.0 - t))));
    switch (face) {
    case 0:
        _x = 1;
        _y = u;
        _z = v;
        return;
    case 1:
        _x = -u;
        _y = 1;
        _z = v;
        return;
    case 2:
        _x = -u;
        _y = -v;
        _z = 1;
        return;
    case 3:
        _x = -1;
        _y = -v;
        _z = -u;
        return;
    case 4:
        _x = v;
        _y = -1;
        _z = -u;
        return;
    case 5:
        _x = v;
        _y = u;
        _z = -1;
        return;
    }
}

inline coordinate_t ecef_coordinate_t::coordinate() const {
    return std::move(
        coordinate_t(
            std::atan2(_y, _x) / std::numbers::pi * 180.0,
            std::atan2(_z, std::sqrt(_x * _x + _y * _y)) / std::numbers::pi * 180.0
        )
    );
}

inline void ecef_coordinate_t::face_s_t(uint8_t& face, double& s, double& t) const {
    face = _x > _y ? (_x > _z ? 0 : 2) : (_y > _z ? 1 :2);
    if ((face == 0 && _x < 0) || (face == 1 && _y < 0) || (face == 2 && _z < 0)) {
        face += 3;
    }
    // (s, t) as (u, v)
    switch (face) {
    case 0:
        s = _y / _x;
        t = _z / _x;
        break;
    case 1:
        s = -_x / _y;
        t = _z / _y;
        break;
    case 2:
        s = -_x / _z;
        t = _y / _z;
        break;
    case 3:
        s = _z / _x;
        t = _y / _x;
        break;
    case 4:
        s = _z / _y;
        t = -_x / _y;
        break;
    case 5:
        s = -_y / _z;
        t = _x / _z;
        break;
    }
    s = s >= 0 ? (0.5 * std::sqrt(1 + 3 * s)) : (1.0 - 0.5 * std::sqrt(1 - 3 * s));
    t = t >= 0 ? (0.5 * std::sqrt(1 + 3 * t)) : (1.0 - 0.5 * std::sqrt(1 - 3 * t));
}

} // namespace s2

} // namespace ingress_drone_explorer