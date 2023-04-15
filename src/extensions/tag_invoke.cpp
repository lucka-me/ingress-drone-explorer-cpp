#include "extensions/tag_invoke.hpp"

#include <boost/json/value_from.hpp>

#include "type/portal_t.hpp"
#include "type/drawn_item_t.hpp"

namespace ingress_drone_explorer {

template<typename T>
inline void extract(const boost::json::object& object, const boost::json::string_view& key, T& to)
{
    to = boost::json::value_to<T>(object.at(key));
}

coordinate_t tag_invoke(const boost::json::value_to_tag<coordinate_t>&, const boost::json::value& value) {
    const auto& object = value.as_object();
    coordinate_t tag;
    extract(object, "lng", tag._lng);
    extract(object, "lat", tag._lat);
    return std::move(tag);
}

portal_t tag_invoke(const boost::json::value_to_tag<portal_t>&, const boost::json::value& value) {
    const auto& object = value.as_object();
    portal_t tag;
    extract(object, "guid", tag._guid);
    if (object.contains("title")) {
        extract(object, "title", tag._title);
    }
    extract(object, "lngLat", tag._coordinate);
    return std::move(tag);
}

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& value, const drawn_item_t& tag) {
    value = {
        { "type", tag._type },
        { "color", tag._color },
        { "latLngs", boost::json::value_from(tag._coordinates) }
    };
}

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& value, const coordinate_t& tag) {
    value = {
        { "lng", tag._lng },
        { "lat", tag._lat }
    };
}

} // namespace ingress_drone_explorer