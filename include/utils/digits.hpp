#pragma once

#include <type_traits>

namespace ingress_drone_explorer {

template<typename T>
inline typename std::enable_if<std::is_integral<T>::value, int>::type digits(T number) {
    int result = number < 0 ? 1 : 0;
    while (number) {
        number /= 10;
        ++result;
    }
    return result;
}

} // namespace ingress_drone_explorer