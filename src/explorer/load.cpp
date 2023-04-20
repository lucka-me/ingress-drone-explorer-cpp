#include "explorer/explorer_t.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <boost/json.hpp>

#include "extensions/tag_invoke.hpp"
#include "utils/match_pattern.hpp"

namespace ingress_drone_explorer {

void explorer_t::load_portals(const std::vector<std::string>& filenames) {
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
            if (match_pattern(realPath.filename().string(), pattern)) {
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

void explorer_t::load_keys(const std::string& filename) {
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

} // namespace ingress_drone_explorer