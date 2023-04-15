#include "boost_extensions/program_options_validate.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/program_options/value_semantic.hpp>

#include "type/coordinate_t.hpp"

namespace ingress_drone_explorer {

void validate(boost::any& v, const std::vector<std::string>& values, coordinate_t*, int) {
    boost::program_options::validators::check_first_occurrence(v);
    const auto& value = boost::program_options::validators::get_single_string(values);
    std::vector<std::string> components;
    boost::algorithm::split(components, value, boost::is_any_of(","));
    if (components.size() != 2 || components[0].empty() || components[1].empty()) {
        throw boost::program_options::validation_error(
            boost::program_options::validation_error::kind_t::invalid_option_value
        );
    }
    const auto lng = std::stod(components[0]);
    const auto lat = std::stod(components[1]);
    if (std::abs(lng) > 180 || std::abs(lat) > 90) {
        throw boost::program_options::validation_error(
            boost::program_options::validation_error::kind_t::invalid_option_value
        );
    }
    v = boost::any(coordinate_t(lng, lat));
}

} // namespace ingress_drone_explorer