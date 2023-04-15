#pragma once

#include <boost/json/value_to.hpp>

namespace ingress_drone_explorer {

struct coordinate_t;
struct drawn_item_t;
struct portal_t;

template<typename T>
inline void extract(const boost::json::object& object, const boost::json::string_view& key, T& to);

coordinate_t tag_invoke(const boost::json::value_to_tag<coordinate_t>&, const boost::json::value& value);
portal_t tag_invoke(const boost::json::value_to_tag<portal_t>&, const boost::json::value& value);

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& value, const drawn_item_t& tag);
void tag_invoke(const boost::json::value_from_tag&, boost::json::value& value, const coordinate_t& tag);

} // namespace ingress_drone_explorer