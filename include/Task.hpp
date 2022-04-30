#pragma once

#include <map>

#include "type/Portal.hpp"
#include "s2.hpp"

namespace ingress_drone_explorer {

class Task {
public:
    void loadPortalLists(const std::vector<std::string>& filenames);
    void loadKeys(const std::string& filename);
    void exploreFrom(const LngLat& start);
    void report() const;
    void saveDrawnItemsTo(const std::string& filename) const;

private:
    using PortalSet = std::set<Portal>;
    using CellSet = std::set<s2::Cell>;
    using CellPortalsMap = std::map<s2::Cell, PortalSet>;

    static constexpr double s_visibleRadius = 500;
    static constexpr double s_reachableRadiusWithKey = 1250;

    LngLat          m_start;
    CellPortalsMap  m_cells;
    CellSet         m_reachableCells;
    CellPortalsMap  m_cellsContainingKeys;
};

} // namespace ingress_drone_explorer