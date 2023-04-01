#include "Task.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>

#include <boost/json.hpp>

#include "boost_extensions/json_tag_invoke.hpp"
#include "std_extensions/iostream.hpp"
#include "utils.hpp"

namespace ingress_drone_explorer {

void Task::loadPortalLists(const std::vector<std::string>& filenames) {
    const auto startTime = std::chrono::steady_clock::now();
    std::cout << "⏳ Loading Portals..." << std::endl;

    size_t portalCount = 0;

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
        const auto portals = boost::json::value_to<std::vector<Portal>>(value);
        size_t fileAddPortalCount = 0;
        size_t fileAddCellCount = 0;
        for (const auto& portal : portals) {
            const auto cell = s2::Cell(portal.m_lngLat);
            const auto itCell = m_cells.find(cell);
            if (m_cells.end() == itCell) {
                m_cells[cell] = { portal };
                ++fileAddCellCount;
                ++fileAddPortalCount;
                continue;
            }
            auto itPortal = itCell->second.find(portal);
            if (itCell->second.end() == itPortal) {
                itCell->second.insert(portal);
                ++fileAddPortalCount;
            } else if (!portal.m_title.empty()) {
                itCell->second.erase(itPortal);
                itCell->second.insert(portal);
            }
        }
        portalCount += fileAddPortalCount;
        std::cout
            << "  📃 Added "
            << std::setw(5) << fileAddPortalCount
            << " portal(s) and "
            << std::setw(4) << fileAddCellCount
            << " cell(s) from " << url
            << std::endl;
    }
    const auto endTime = std::chrono::steady_clock::now();
    std::cout
        << "📍 Loaded " << portalCount << " Portal(s) "
        << "in " << m_cells.size() << " cell(s) "
        << "from " << urls.size() << " file(s), "
        << "which took "
        << 1E-6 * std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()
        << " seconds"
        << std::endl;
}

void Task::loadKeys(const std::string& filename) {
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
    const auto loadCount = keys.size();
    for (const auto& entry : m_cells) {
        decltype(entry.second) keysInCell;
        std::set_intersection(
            entry.second.begin(), entry.second.end(),
            keys.begin(), keys.end(),
            std::inserter(keysInCell, keysInCell.begin())
        );
        if (keysInCell.empty()) {
            continue;
        }
        decltype(keys) leftKeys;
        std::set_difference(
            keys.begin(), keys.end(),
            keysInCell.begin(), keysInCell.end(),
            std::inserter(leftKeys, leftKeys.begin())
        );
        keys.swap(leftKeys);
        m_cellsContainingKeys[entry.first] = std::move(keysInCell);
    }
    std::cout
        << "🔑 Loaded " << loadCount << " Key(s) "
        << "and matched " << loadCount - keys.size() << " "
        << "in " << m_cellsContainingKeys.size() << " cell(s)"
        << std::endl;
}

void Task::exploreFrom(const LngLat& start) {
    m_start = start;
    const auto startCell = s2::Cell(start);
    const auto startTime = std::chrono::steady_clock::now();
    std::cout
        << "⏳ Explore from " << start
        << " in cell #" << startCell
        << std::endl;
    CellSet queue;

    if (m_cells.contains(startCell)) {
        queue.insert(startCell);
    } else {
        queue = startCell.queryNeighbouredCellsCoveringCapOf(start, s_visibleRadius);
    }

    auto previousTime = startTime;
    const auto progressDigits = digits(m_cells.size());

    for (auto it = queue.begin(); it != queue.end(); it = queue.begin()) {
        const auto cell = *it;
        queue.erase(it);

        if (m_reachableCells.contains(cell)) {
            continue;
        }
        const auto portals = m_cells.find(cell);
        if (m_cells.end() == portals) {
            continue;
        }
        m_reachableCells.insert(cell);
        m_cellsContainingKeys.erase(cell);

        for (const auto& portal : portals->second) {
            auto candidates = cell.queryNeighbouredCellsCoveringCapOf(
                portal.m_lngLat, s_visibleRadius
            );
            queue.merge(candidates);
            std::erase_if(m_cellsContainingKeys, [&](const auto& item) {
                if (queue.contains(item.first)) {
                    return true;
                }
                bool reachable = false;
                for (const auto& target : item.second) {
                    if (portal.m_lngLat.distanceTo(target.m_lngLat) < s_reachableRadiusWithKey) {
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
            if (now - previousTime > std::chrono::milliseconds(1000)) {
                std::cout
                    << "⏳ Reached "
                    << std::setw(progressDigits) << m_reachableCells.size()
                    << " / " << m_cells.size() << " cell(s)"
                    << std::endl;
                previousTime = now;
            }
        }
    }

    const auto endTime = std::chrono::steady_clock::now();
    std::cout
        << "🔍 Exploration finished after "
        << 1E-6 * std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()
        << " seconds"
        << std::endl;
}

void Task::report() const {
    size_t portalsCount = 0;
    size_t reachablePortalsCount = 0;
    Portal furthestPortal;
    furthestPortal.m_lngLat = m_start;
    for (const auto& entry : m_cells) {
        portalsCount += entry.second.size();
        if (!m_reachableCells.contains(entry.first)) {
            continue;
        }
        reachablePortalsCount += entry.second.size();
        for (const auto& portal : entry.second) {
            if (m_start.closer(furthestPortal.m_lngLat, portal.m_lngLat)) {
                furthestPortal = portal;
            }
        }
    }
    if (reachablePortalsCount == 0) {
        std::cout
            << "⛔️ There is no reachable portal in "
            << portalsCount
            << " portal(s) from "
            << m_start
            << std::endl;
        return;
    }
    const auto totalNumberDigits = digits(portalsCount);
    const auto reachableNumberDigits = digits(reachablePortalsCount);
    const auto unreachableNumberDigits = digits(portalsCount - reachablePortalsCount);
    std::cout
        << "⬜️ In "
        << std::setw(totalNumberDigits) << m_cells.size()
        << "   cell(s), "
        << std::setw(reachableNumberDigits) << m_reachableCells.size()
        << " are ✅ reachable, "
        << std::setw(unreachableNumberDigits) << m_cells.size() - m_reachableCells.size()
        << " are ⛔️ not."
        << std::endl;
    std::cout
        << "📍 In "
        << std::setw(totalNumberDigits) << portalsCount
        << " Portal(s), "
        << std::setw(reachableNumberDigits) << reachablePortalsCount
        << " are ✅ reachable, "
        << std::setw(unreachableNumberDigits) << portalsCount - reachablePortalsCount
        << " are ⛔️ not."
        << std::endl;
    std::cout
        << "🛬 The furthest Portal is "
        << (furthestPortal.m_title.empty() ? "Untitled" : furthestPortal.m_title)
        << "." << std::endl
        << "  📍 It's located at " << furthestPortal.m_lngLat << std::endl
        << "  📏 Where is "
            << m_start.distanceTo(furthestPortal.m_lngLat) / 1000 << " km away"
            << std::endl
        << "  🔗 Check it out: https://intel.ingress.com/?pll="
            << furthestPortal.m_lngLat.m_lat << "," << furthestPortal.m_lngLat.m_lng
            << std::endl;
}

void Task::saveDrawnItemsTo(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        throw std::runtime_error("Unable to open drawn items file.");
    }
    std::vector<DrawnItem> items;
    items.reserve(m_cells.size());
    std::transform(
        m_cells.begin(), m_cells.end(),
        std::back_inserter(items),
        [&](const auto& entry) {
            const auto shape = entry.first.shape();
            return std::move(
                DrawnItem(
                    m_reachableCells.contains(entry.first) ? "#783cbd" : "#404040",
                    std::move(std::vector<LngLat>(shape.begin(), shape.end()))
                )
            );
        }
    );
    const auto value = boost::json::value_from(items);
    out << value;
    std::cout << "💾 Saved drawn items to " << filename << std::endl;
}

} // namespace ingress_drone_explorer