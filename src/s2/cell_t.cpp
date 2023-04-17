#include "s2/cell_t.hpp"

#include <algorithm>
#include <cmath>

#include "definitions/coordinate_t.hpp"
#include "s2/ecef_coordinate_t.hpp"

namespace ingress_drone_explorer {

namespace s2 {

cell_t::cell_t(const coordinate_t& coordinate, const uint8_t level) {
    _level = level;
    double s, t;
    ecef_coordinate_t(coordinate).face_s_t(_face, s, t);
    const int32_t max = 1 << level;
    _i = std::clamp(static_cast<decltype(_i)>(std::floor(s * max)), 0, max - 1);
    _j = std::clamp(static_cast<decltype(_j)>(std::floor(t * max)), 0, max - 1);
}

bool cell_t::intersects_with_cap_of(const coordinate_t& center, const double radius) const {
    auto corners = shape();
    /// TODO: We only needs the cloest one or two at most
    std::sort(
        corners.begin(), corners.end(),
        [&](const auto& a, const auto& b) {
            return center.closer(a, b);
        }
    );
    return center.distance_to(corners[0]) < radius || center.distance_to(corners[0], corners[1]) < radius;
}

std::set<cell_t> cell_t::neighboured_cells_covering_cap_of(const coordinate_t& center, const double radius) const {
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

std::set<cell_t> cell_t::neighboured_cells_in(const int32_t rounds) const {
    std::set<cell_t> result;
    /// TODO: Check if correct near the edge of face?
    /// Maybe we need the algorithm of neighboured_cells_covering_cap_of to "search" instead of generate?
    for (int32_t round = 0; round < rounds; ++round) {
        const int32_t steps = (round + 1) * 2;
        for (int32_t step = 0; step < steps; ++step) {
            result.insert(cell_t(_face, _i - round - 1   , _j - round + step, _level)); // Left, upward
            result.insert(cell_t(_face, _i - round + step, _j + round + 1   , _level)); // Top, rightward
            result.insert(cell_t(_face, _i + round + 1   , _j + round - step, _level)); // Right, downward
            result.insert(cell_t(_face, _i + round - step, _j - round - 1   , _level)); // Bottom, leftward
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

inline cell_t::cell_t(const uint8_t face, const int32_t i, const int32_t j, uint8_t level) {
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

inline coordinate_t cell_t::coordinate(const double d_i, const double d_j) const {
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

} // namespace s2

} // namespace ingress_drone_explorer