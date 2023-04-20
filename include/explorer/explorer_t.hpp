#pragma once

#include <map>
#include <set>
#include <vector>

#include "definitions/portal_t.hpp"
#include "s2/cell_t.hpp"

namespace ingress_drone_explorer {

class explorer_t {
public:
    void load_portals(const std::vector<std::string>& filenames);
    void load_keys(const std::string& filename);
    void explore_from(const coordinate_t& start);
    void report() const;
    void save_drawn_items_to(const std::string& filename) const;

private:
    using portal_set_t = std::set<portal_t>;
    using cell_set_t = std::set<s2::cell_t>;
    using cell_portals_map_t = std::map<s2::cell_t, portal_set_t>;

    static constexpr double _visible_radius = 500;
    static constexpr double _reachable_radius_with_key = 1250;

    coordinate_t        _start;
    cell_portals_map_t  _cells;
    cell_set_t          _reachable_cells;
    cell_portals_map_t  _cells_containing_keys;
};

} // namespace ingress_drone_explorer