#include "task_t.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>

#include <boost/json.hpp>

#include "extensions/iostream_extensions.hpp"
#include "extensions/tag_invoke.hpp"
#include "type/drawn_item_t.hpp"
#include "utils.hpp"

namespace ingress_drone_explorer {

void task_t::load_portals(const std::vector<std::string>& filenames) {
    const auto start_time = std::chrono::steady_clock::now();
    std::cout << "⏳ Loading Portals..." << std::endl;

    size_t portal_count = 0;

    std::set<std::string> urls;
    for (const auto& filename : filenames) {
        if (std::string::npos == filename.find('*')) {
            urls.insert(filename);
            continue;
        }
        // Resolve wildcard
        const std::filesystem::path url(filename);
        const auto parent = url.parent_path();
        if (std::string::npos != parent.string().find('*')) {
            throw std::runtime_error("Invalid wildcard.");
        }
        const auto pattern = url.filename().string();
        std::filesystem::directory_iterator iterator(parent);
        for (const auto& entry : iterator) {
            const auto realPath = entry.path();
            if (match(realPath.filename().string(), pattern)) {
                urls.insert(realPath.string());
            }
        }
    }

    for (const auto& url : urls) {
        std::ifstream in(url);
        if (!in.is_open()) {
            throw std::runtime_error("Unable to open portal list file.");
        }
        boost::json::stream_parser parser;
        std::string line;
        while (!in.eof() && !parser.done()) {
            std::getline(in, line);
            parser.write(line);
        }
        const auto value = parser.release();
        const auto portals = boost::json::value_to<std::vector<portal_t>>(value);
        size_t file_add_portal_count = 0;
        size_t file_add_cell_count = 0;
        for (const auto& portal : portals) {
            const auto cell = s2::cell_t(portal._coordinate);
            const auto it_cell = _cells.find(cell);
            if (_cells.end() == it_cell) {
                _cells[cell] = { portal };
                ++file_add_cell_count;
                ++file_add_portal_count;
                continue;
            }
            auto it_portal = it_cell->second.find(portal);
            if (it_cell->second.end() == it_portal) {
                it_cell->second.insert(portal);
                ++file_add_portal_count;
            } else if (!portal._title.empty()) {
                it_cell->second.erase(it_portal);
                it_cell->second.insert(portal);
            }
        }
        portal_count += file_add_portal_count;
        std::cout
            << "  📃 Added "
            << std::setw(5) << file_add_portal_count
            << " portal(s) and "
            << std::setw(4) << file_add_cell_count
            << " cell(s) from " << url
            << std::endl;
    }
    const auto end_time = std::chrono::steady_clock::now();
    std::cout
        << "📍 Loaded " << portal_count << " Portal(s) "
        << "in " << _cells.size() << " cell(s) "
        << "from " << urls.size() << " file(s), "
        << "which took "
        << 1E-6 * std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()
        << " seconds"
        << std::endl;
}

void task_t::load_keys(const std::string& filename) {
    std::cout << "⏳ Loading Keys from " << filename << "..." << std::endl;
    std::ifstream in(filename);
    if (!in.is_open()) {
        throw std::runtime_error("Unable to open key list file.");
    }
    boost::json::stream_parser parser;
    std::string line;
    while (!in.eof() && !parser.done()) {
        std::getline(in, line);
        parser.write(line);
    }
    const auto value = parser.release();
    const auto list = boost::json::value_to<std::vector<std::string>>(value);
    std::set<std::string> keys(list.begin(), list.end());
    const auto load_count = keys.size();
    for (const auto& entry : _cells) {
        decltype(entry.second) keys_in_cell;
        std::set_intersection(
            entry.second.begin(), entry.second.end(),
            keys.begin(), keys.end(),
            std::inserter(keys_in_cell, keys_in_cell.begin())
        );
        if (keys_in_cell.empty()) {
            continue;
        }
        decltype(keys) left_keys;
        std::set_difference(
            keys.begin(), keys.end(),
            keys_in_cell.begin(), keys_in_cell.end(),
            std::inserter(left_keys, left_keys.begin())
        );
        keys.swap(left_keys);
        _cells_containing_keys[entry.first] = std::move(keys_in_cell);
    }
    std::cout
        << "🔑 Loaded " << load_count << " Key(s) "
        << "and matched " << load_count - keys.size() << " "
        << "in " << _cells_containing_keys.size() << " cell(s)"
        << std::endl;
}

void task_t::explore_from(const coordinate_t& start) {
    _start = start;
    const auto start_cell = s2::cell_t(start);
    const auto start_time = std::chrono::steady_clock::now();
    std::cout
        << "⏳ Explore from " << start
        << " in cell #" << start_cell
        << std::endl;
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

void task_t::report() const {
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
    const auto total_numberDigits = digits(portals_count);
    const auto reachable_number_digits = digits(reachable_portals_count);
    const auto unreachable_number_digits = digits(portals_count - reachable_portals_count);
    std::cout
        << "⬜️ In "
        << std::setw(total_numberDigits) << _cells.size()
        << "   cell(s), "
        << std::setw(reachable_number_digits) << _reachable_cells.size()
        << " are ✅ reachable, "
        << std::setw(unreachable_number_digits) << _cells.size() - _reachable_cells.size()
        << " are ⛔️ not."
        << std::endl;
    std::cout
        << "📍 In "
        << std::setw(total_numberDigits) << portals_count
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

void task_t::save_drawn_items_to(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        throw std::runtime_error("Unable to open drawn items file.");
    }
    std::vector<drawn_item_t> items;
    items.reserve(_cells.size());
    std::transform(
        _cells.begin(), _cells.end(),
        std::back_inserter(items),
        [&](const auto& entry) {
            const auto shape = entry.first.shape();
            return std::move(
                drawn_item_t(
                    _reachable_cells.contains(entry.first) ? "#783cbd" : "#404040",
                    std::move(std::vector<coordinate_t>(shape.begin(), shape.end()))
                )
            );
        }
    );
    const auto value = boost::json::value_from(items);
    out << value;
    std::cout << "💾 Saved drawn items to " << filename << std::endl;
}

} // namespace ingress_drone_explorer