#pragma once

#include <boost/json/value_to.hpp>

#include "type/DrawnItem.hpp"
#include "type/Portal.hpp"

namespace ingress_drone_explorer {

template<typename T>
inline void extract(const boost::json::object& object, const boost::json::string_view& key, T& to);

LngLat tag_invoke(const boost::json::value_to_tag<LngLat>&, const boost::json::value& value);
Portal tag_invoke(const boost::json::value_to_tag<Portal>&, const boost::json::value& value);

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& value, const DrawnItem& tag);
void tag_invoke(const boost::json::value_from_tag&, boost::json::value& value, const LngLat& tag);

} // namespace ingress_drone_explorer