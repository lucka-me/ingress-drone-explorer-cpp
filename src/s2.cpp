#include "s2.hpp"

#include <algorithm>

namespace ingress_drone_explorer {

namespace s2 {

Cell::Cell(const LngLat& lngLat, const uint8_t level) {
    m_level = level;
    double s, t;
    ECEFCoordinate(lngLat).faceST(m_face, s, t);
    const int32_t max = 1 << level;
    m_i = std::clamp(static_cast<decltype(m_i)>(std::floor(s * max)), 0, max - 1);
    m_j = std::clamp(static_cast<decltype(m_j)>(std::floor(t * max)), 0, max - 1);
}

std::set<Cell> Cell::queryNeighbouredCellsCoveringCapOf(const LngLat& center, const double radius) const
{
    std::set<Cell> result;
    std::set<Cell> outside;
    std::set<Cell> queue { *this };
    for (auto it = queue.begin(); it != queue.end(); it = queue.begin()) {
        const auto cell = *it;
        queue.erase(it);
        if (result.contains(cell) || outside.contains(cell)) {
            continue;
        }
        if (cell.intersectsWithCapOf(center, radius)) {
            queue.merge(cell.neighbours());
            result.insert(std::move(cell));
        } else {
            outside.insert(std::move(cell));
        }
    }
    return std::move(result);
}

std::array<LngLat, 4> Cell::shape() const {
    return {
        lngLat(0, 0),
        lngLat(0, 1),
        lngLat(1, 1),
        lngLat(1, 0),
    };
}

inline Cell::Cell(const uint8_t face, const double i, const double j, uint8_t level)
{
    m_level = level;
    const int32_t max = 1 << level;
    if (i >= 0 && j >= 0 && i < max && j < max) {
        m_face = face;
        m_i = i;
        m_j = j;
        return;
    }
    double s, t;
    ECEFCoordinate(face, (0.5 + i) / max, (0.5 + j) / max).faceST(m_face, s, t);
    m_i = std::clamp(static_cast<decltype(m_i)>(std::floor(s * max)), 0, max - 1);
    m_j = std::clamp(static_cast<decltype(m_j)>(std::floor(t * max)), 0, max - 1);
}

inline bool Cell::intersectsWithCapOf(const LngLat& center, const double radius) const {
    auto corners = shape();
    std::sort(
        corners.begin(), corners.end(),
        [&](const auto& a, const auto& b) {
            return center.closer(a, b);
        }
    );
    return center.distanceTo(corners[0]) < radius || center.distanceTo(corners[0], corners[1]) < radius;
}

inline LngLat Cell::lngLat(const double dI, const double dJ) const
{
    const double max = 1 << m_level;
    return std::move(
        ECEFCoordinate(
            m_face,
            (dI + m_i) / max,
            (dJ + m_j) / max
        )
        .lngLat()
    );
}

inline std::set<Cell> Cell::neighbours() const {
    return {
        Cell(m_face, m_i - 1, m_j       , m_level),
        Cell(m_face, m_i    , m_j - 1   , m_level),
        Cell(m_face, m_i + 1, m_j + 1   , m_level),
        Cell(m_face, m_i    , m_j       , m_level),
    };
}

inline ECEFCoordinate::ECEFCoordinate(const LngLat& lngLat) {
    const auto theta = lngLat.theta();
    const auto phi = lngLat.phi();
    const auto cosPhi = std::cos(phi);
    m_x = std::cos(theta) * cosPhi;
    m_y = std::sin(theta) * cosPhi;
    m_z = std::sin(phi);
}

inline ECEFCoordinate::ECEFCoordinate(const uint8_t face, const double s, const double t)
{
    const auto u = (1.0 / 3.0) * (s >= 0.5 ? (4.0 * s * s - 1) : (1.0 - (4.0 * (1.0 - s) * (1.0 - s))));
    const auto v = (1.0 / 3.0) * (t >= 0.5 ? (4.0 * t * t - 1) : (1.0 - (4.0 * (1.0 - t) * (1.0 - t))));
    switch (face) {
    case 0:
        m_x = 1;
        m_y = u;
        m_z = v;
        return;
    case 1:
        m_x = -u;
        m_y = 1;
        m_z = v;
        return;
    case 2:
        m_x = -u;
        m_y = -v;
        m_z = 1;
        return;
    case 3:
        m_x = -1;
        m_y = -v;
        m_z = -u;
        return;
    case 4:
        m_x = v;
        m_y = -1;
        m_z = -u;
        return;
    case 5:
        m_x = v;
        m_y = u;
        m_z = -1;
        return;
    }
}

inline LngLat ECEFCoordinate::lngLat() const {
    return std::move(
        LngLat(
            std::atan2(m_y, m_x) / M_PI * 180.0,
            std::atan2(m_z, std::sqrt(m_x * m_x + m_y * m_y)) / M_PI * 180.0
        )
    );
}

inline void ECEFCoordinate::faceST(uint8_t& face, double& s, double& t) const {
    face = m_x > m_y ? (m_x > m_z ? 0 : 2) : (m_y > m_z ? 1 :2);
    if ((face == 0 && m_x < 0) || (face == 1 && m_y < 0) || (face == 2 && m_z < 0)) {
        face += 3;
    }
    // (s, t) as (u, v)
    switch (face) {
    case 0:
        s = m_y / m_x;
        t = m_z / m_x;
        break;
    case 1:
        s = -m_x / m_y;
        t = m_z / m_y;
        break;
    case 2:
        s = -m_x / m_z;
        t = m_y / m_z;
        break;
    case 3:
        s = m_z / m_x;
        t = m_y / m_x;
        break;
    case 4:
        s = m_z / m_y;
        t = -m_x / m_y;
        break;
    case 5:
        s = -m_y / m_z;
        t = m_x / m_z;
        break;
    }
    s = s >= 0 ? (0.5 * std::sqrt(1 + 3 * s)) : (1.0 - 0.5 * std::sqrt(1 - 3 * s));
    t = t >= 0 ? (0.5 * std::sqrt(1 + 3 * t)) : (1.0 - 0.5 * std::sqrt(1 - 3 * t));
}

} // namespace s2

} // namespace ingress_drone_explorer