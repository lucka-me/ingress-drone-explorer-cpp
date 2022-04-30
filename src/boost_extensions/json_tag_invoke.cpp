#include "boost_extensions/json_tag_invoke.hpp"

namespace ingress_drone_explorer {

template<typename T>
inline void extract(const boost::json::object& object, const boost::json::string_view& key, T& to)
{
    to = boost::json::value_to<T>(object.at(key));
}

LngLat tag_invoke(const boost::json::value_to_tag<LngLat>&, const boost::json::value& value) {
    const auto& object = value.as_object();
    LngLat tag;
    extract(object, "lng", tag.m_lng);
    extract(object, "lat", tag.m_lat);
    return std::move(tag);
}

Portal tag_invoke(const boost::json::value_to_tag<Portal>&, const boost::json::value& value) {
    const auto& object = value.as_object();
    Portal tag;
    extract(object, "guid", tag.m_guid);
    if (object.contains("title")) {
        extract(object, "title", tag.m_title);
    }
    extract(object, "lngLat", tag.m_lngLat);
    return std::move(tag);
}

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& value, const DrawnItem& tag) {
    value = {
        { "type", tag.m_type },
        { "color", tag.m_color },
        { "latLngs", boost::json::value_from(tag.m_lngLats) }
    };
}

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& value, const LngLat& tag) {
    value = {
        { "lng", tag.m_lng },
        { "lat", tag.m_lat }
    };
}

} // namespace ingress_drone_explorer