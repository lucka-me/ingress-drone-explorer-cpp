#pragma once

#include <string>
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

bool match(const std::string& text, const std::string& pattern) {
    size_t posT = 0;
    size_t posP = 0;
    // Match till first *
    while (posT < text.size() && posP < pattern.size() && pattern[posP] != '*') {
        if (text[posT] != pattern[posP] && pattern[posP] != '?') {
            return false;
        }
        ++posT;
        ++posP;
    }
    auto startT = std::string::npos;
    auto startP = std::string::npos;
    while (posT < text.size()) {
        if (pattern[posP] == '*') {
            ++posP;
            // Latest *
            if (pattern.size() == posP) {
                break;
            }
            startT = posT;
            startP = posP;
        } else if (text[posT] == pattern[posP] || pattern[posP] == '?') {
            ++posT;
            ++posP;
        } else if (startP != std::string::npos) {
            // Cover the segment with * and retry
            ++startT;
            posP = startP;
            posT = startT;
        } else {
            // Can not retry
            break;
        }
    }
    return pattern.size() == posP;
}

} // namespace ingress_drone_explorer