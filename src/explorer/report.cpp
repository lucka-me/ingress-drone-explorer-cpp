#include "explorer/explorer_t.hpp"

#include <fstream>
#include <iomanip>

#include <boost/json.hpp>

#include "definitions/drawn_item_t.hpp"
#include "extensions/iostream_extensions.hpp"
#include "extensions/tag_invoke.hpp"
#include "utils/digits.hpp"

namespace ingress_drone_explorer {

void explorer_t::report() const {
    size_t portals_count = 0;
    size_t reachable_portals_count = 0;
    portal_t furthest_portal;
    furthest_portal._coordinate = _start;
    for (const auto& entry : _cells) {
        portals_count += entry.second.size();
        if (!_reachable_cells.contains(entry.first)) {
            continue;
        }
        reachable_portals_count += entry.second.size();
        for (const auto& portal : entry.second) {
            if (_start.closer(furthest_portal._coordinate, portal._coordinate)) {
                furthest_portal = portal;
            }
        }
    }
    if (reachable_portals_count == 0) {
        std::cout
            << "⛔️ There is no reachable portal in "
            << portals_count
            << " portal(s) from "
            << _start
            << std::endl;
        return;
    }
    const auto total_number_digits = digits(portals_count);
    const auto reachable_number_digits = digits(reachable_portals_count);
    const auto unreachable_number_digits = digits(portals_count - reachable_portals_count);
    std::cout
        << "⬜️ In "
        << std::setw(total_number_digits) << _cells.size()
        << "   cell(s), "
        << std::setw(reachable_number_digits) << _reachable_cells.size()
        << " are ✅ reachable, "
        << std::setw(unreachable_number_digits) << _cells.size() - _reachable_cells.size()
        << " are ⛔️ not."
        << std::endl;
    std::cout
        << "📍 In "
        << std::setw(total_number_digits) << portals_count
        << " Portal(s), "
        << std::setw(reachable_number_digits) << reachable_portals_count
        << " are ✅ reachable, "
        << std::setw(unreachable_number_digits) << portals_count - reachable_portals_count
        << " are ⛔️ not."
        << std::endl;
    std::cout
        << "🛬 The furthest Portal is "
        << (furthest_portal._title.empty() ? "Untitled" : furthest_portal._title)
        << "." << std::endl
        << "  📍 It's located at " << furthest_portal._coordinate << std::endl
        << "  📏 Where is "
            << _start.distance_to(furthest_portal._coordinate) / 1000 << " km away"
            << std::endl
        << "  🔗 Check it out: https://intel.ingress.com/?pll="
            << furthest_portal._coordinate._lat << "," << furthest_portal._coordinate._lng
            << std::endl;
}

void explorer_t::save_drawn_items_to(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        throw std::runtime_error("Unable to open drawn items file.");
    }
    std::vector<drawn_item_t> items;
    items.reserve(_cells.size());
    std::transform(
        _cells.begin(), _cells.end(),
        std::back_inserter(items),
        [&](const auto& entry) -> drawn_item_t {
            const auto shape = entry.first.shape();
            return {
                _reachable_cells.contains(entry.first) ? "#783cbd" : "#404040",
                { shape.begin(), shape.end() }
            };
        }
    );
    const auto value = boost::json::value_from(items);
    out << value;
    std::cout << "💾 Saved drawn items to " << filename << std::endl;
}

} // namespace ingress_drone_explorer