#pragma once

#include <array>
#include <set>

namespace ingress_drone_explorer {

struct coordinate_t;

namespace s2 {

struct cell_t {
    uint8_t _face;
    uint8_t _level;
    int32_t _i;
    int32_t _j;

    cell_t(const coordinate_t& lngLat, const uint8_t level = 16);

public:
    inline auto operator<=>(const cell_t& other) const = default;
    
    std::set<cell_t> neighboured_cells_covering_cap_of(const coordinate_t& center, const double radius) const;
    std::array<coordinate_t, 4> shape() const;

private:
    inline cell_t(const uint8_t face, const double i, const double j, uint8_t level = 16);

    inline bool intersects_with_cap_of(const coordinate_t& center, const double radius) const;
    inline coordinate_t coordinate(const double d_i, const double d_j) const;
    inline std::set<cell_t> neighbours() const;
};

template<typename T>
static inline auto operator<(const cell_t& a, const std::pair<const cell_t, T>& b) {
    return a < b.first;
}

template<typename T>
static inline auto operator<(const std::pair<const cell_t, T>& a, const cell_t& b) {
    return a.first < b;
}

struct ecef_coordinate_t
{
    double _x;
    double _y;
    double _z;

    inline ecef_coordinate_t(const coordinate_t& lngLat);
    inline ecef_coordinate_t(const uint8_t face, const double s, const double t);

    inline coordinate_t coordinate() const;
    inline void face_s_t(uint8_t& face, double& s, double& t) const;
};

} // namespace s2

} // namespace ingress_drone_explorer