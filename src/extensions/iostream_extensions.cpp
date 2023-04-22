#include "extensions/iostream_extensions.hpp"

#include <string>

#include "definitions/coordinate_t.hpp"
#include "s2/cell_t.hpp"

namespace ingress_drone_explorer {

std::istream& operator>>(std::istream& in, coordinate_t& value) {
    std::string component;
    std::getline(in, component, ',');
    value._lng = std::stod(component);
    std::getline(in, component);
    value._lat = std::stod(component);
    return in;
}

std::ostream& operator<<(std::ostream& out, const coordinate_t& value) {
    return out << value._lng << "," << value._lat;
}

namespace s2 {

std::ostream& operator<<(std::ostream& out, const cell_t& value) {
    return out
        << +value._face
        << "," << +value._level
        << "," << value._i
        << "," << value._j;
}

} // namespace s2

} // namespace ingress_drone_explorer
