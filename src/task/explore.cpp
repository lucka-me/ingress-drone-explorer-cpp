#include "task/task_t.hpp"

#include <chrono>
#include <iomanip>

#include "extensions/iostream_extensions.hpp"
#include "utils/digits.hpp"

namespace ingress_drone_explorer {

void task_t::explore_from(const coordinate_t& start) {
    _start = start;
    const auto start_cell = s2::cell_t(start);
    const auto start_time = std::chrono::steady_clock::now();
    std::cout << "⏳ Explore from " << start << " in cell #" << start_cell << std::endl;
    cell_set_t queue;

    if (_cells.contains(start_cell)) {
        queue.insert(start_cell);
    } else {
        queue = start_cell.neighboured_cells_covering_cap_of(start, _visible_radius);
    }

    auto previous_time = start_time;
    const auto progress_digits = digits(_cells.size());

    for (auto it = queue.begin(); it != queue.end(); it = queue.begin()) {
        const auto cell = *it;
        queue.erase(it);

        if (_reachable_cells.contains(cell)) {
            continue;
        }
        const auto portals = _cells.find(cell);
        if (_cells.end() == portals) {
            continue;
        }
        _reachable_cells.insert(cell);
        _cells_containing_keys.erase(cell);

        for (const auto& portal : portals->second) {
            auto candidates = cell.neighboured_cells_covering_cap_of(
                portal._coordinate, _visible_radius
            );
            queue.merge(candidates);
            std::erase_if(_cells_containing_keys, [&](const auto& item) {
                if (queue.contains(item.first)) {
                    return true;
                }
                bool reachable = false;
                for (const auto& target : item.second) {
                    if (portal._coordinate.distance_to(target._coordinate) < _reachable_radius_with_key) {
                        reachable = true;
                        break;
                    }
                }
                if (reachable) {
                    queue.insert(item.first);
                }
                return reachable;
            });

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
    }

    const auto end_time = std::chrono::steady_clock::now();
    std::cout
        << "🔍 Exploration finished after "
        << 1E-6 * std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()
        << " seconds"
        << std::endl;
}

} // namespace ingress_drone_explorer