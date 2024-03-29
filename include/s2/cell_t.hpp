#pragma once

#include <array>
#include <cstdint>
#include <set>

namespace ingress_drone_explorer {

struct coordinate_t;

namespace s2 {

struct cell_t {
    uint8_t _face;
    uint8_t _level;
    int32_t _i;
    int32_t _j;

    cell_t(const coordinate_t& coordinate, const uint8_t level = 16);
    inline cell_t(const uint8_t face, const int32_t i, const int32_t j, uint8_t level = 16);

public:
    inline auto operator<=>(const cell_t& other) const = default;

public:
    bool intersects_with_cap_of(const coordinate_t& center, const double radius) const;
    std::set<cell_t> neighbored_cells_covering_cap_of(const coordinate_t& center, const double radius) const;
    std::set<cell_t> neighbored_cells_in(const int32_t rounds) const;
    std::array<coordinate_t, 4> shape() const;

private:

    inline coordinate_t coordinate(const double d_i, const double d_j) const;
    inline std::set<cell_t> neighbors() const;
};

} // namespace s2

} // namespace ingress_drone_explorer