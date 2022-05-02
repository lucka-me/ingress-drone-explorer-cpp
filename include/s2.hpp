#pragma once

#include <array>
#include <compare>
#include <cstdint>
#include <set>

#include "type/LngLat.hpp"

namespace ingress_drone_explorer {

namespace s2 {

struct Cell {
    uint8_t m_face;
    uint8_t m_level;
    int32_t m_i;
    int32_t m_j;

    Cell(const LngLat& lngLat, const uint8_t level = 16);

public:
    inline auto operator<=>(const Cell& other) const = default;
    
    std::set<Cell> queryNeighbouredCellsCoveringCapOf(const LngLat& center, const double radius) const;
    std::array<LngLat, 4> shape() const;

private:
    inline Cell(const uint8_t face, const double i, const double j, uint8_t level = 16);

    inline bool intersectsWithCapOf(const LngLat& center, const double radius) const;
    inline LngLat lngLat(const double dI, const double dJ) const;
    inline std::set<Cell> neighbours() const;
};

template<typename T>
static inline auto operator<(const Cell& a, const std::pair<const Cell, T>& b) {
    return a < b.first;
}

template<typename T>
static inline auto operator<(const std::pair<const Cell, T>& a, const Cell& b) {
    return a.first < b;
}

struct ECEFCoordinate
{
    double m_x;
    double m_y;
    double m_z;

    inline ECEFCoordinate(const LngLat& lngLat);
    inline ECEFCoordinate(const uint8_t face, const double s, const double t);

    inline LngLat lngLat() const;
    inline void faceST(uint8_t& face, double& s, double& t) const;
};

} // namespace s2

} // namespace ingress_drone_explorer