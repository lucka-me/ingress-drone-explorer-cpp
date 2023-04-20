#include "task/task_t.hpp"

#include <chrono>
#include <iomanip>

#include "extensions/iostream_extensions.hpp"
#include "utils/digits.hpp"

namespace ingress_drone_explorer {

void task_t::explore_from(const coordinate_t& start) {
    _start = start;
    const auto start_time = std::chrono::steady_clock::now();
    const auto start_cell = s2::cell_t(start);
    std::cout << "⏳ Explore from " << start << " in cell #" << start_cell << std::endl;
    cell_set_t queue;

    if (_cells.contains(start_cell)) {
        queue.insert(start_cell);
    } else {
        queue = start_cell.neighbored_cells_covering_cap_of(start, _visible_radius);
        std::erase_if(queue, [&](const auto& item) { return !_cells.contains(item); });
    }
    std::erase_if(_cells_containing_keys, [&](const auto& item) { return queue.contains(item.first); });

    auto previous_time = start_time;
    const auto progress_digits = digits(_cells.size());

    for (auto it = queue.begin(); it != queue.end(); it = queue.begin()) {

        const auto cell = *it;
        queue.erase(it);

        const auto portals = _cells.find(cell);
        if (_cells.end() == portals) {
            continue;
        }
        _reachable_cells.insert(cell);

        // Get all neighbors in the visible range (also the possible ones), filter the empty/pending/reached ones and
        // search for reachable ones
        constexpr int32_t safe_rounds_for_visible_radius = (_visible_radius / 80) + 1;
        const auto neighbors = cell.neighbored_cells_in(safe_rounds_for_visible_radius);
        for (const auto& neighbor : neighbors) {
            if (queue.contains(neighbor) || _reachable_cells.contains(neighbor) || !_cells.contains(neighbor)) {
                continue;
            }
            for (const auto& portal : portals->second) {
                if (neighbor.intersects_with_cap_of(portal._coordinate, _visible_radius)) {
                    queue.insert(neighbor);
                    break;
                }
            }
        }

        // Find keys
        /// TODO: Consider to use cell.neighbored_cells_in instead?
        if (!_cells_containing_keys.empty()) {
            for (const auto& portal : portals->second) {
                std::erase_if(_cells_containing_keys, [&](const auto& item) {
                    if (queue.contains(item.first)) {
                        return true;
                    }
                    for (const auto& target : item.second) {
                        if (portal._coordinate.distance_to(target._coordinate) < _reachable_radius_with_key) {
                            queue.insert(item.first);
                            return true;
                        }
                    }
                    return false;
                });
                if (_cells_containing_keys.empty()) {
                    break;
                }
            }
        }

        const auto now = std::chrono::steady_clock::now();
        if (now - previous_time > std::chrono::milliseconds(1000)) {
            std::cout
                << "⏳ Reached "
                << std::setw(progress_digits) << _reachable_cells.size()
                << " / " << _cells.size() << " cell(s)"
                << std::endl;
            previous_time = now;
        }
    }

    const auto end_time = std::chrono::steady_clock::now();
    std::cout
        << "🔍 Exploration finished after "
        << 1E-6 * std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()
        << " seconds"
        << std::endl;
}

} // namespace ingress_drone_explorer