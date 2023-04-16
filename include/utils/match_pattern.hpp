#pragma once

#include <string>

namespace ingress_drone_explorer {

inline bool match_pattern(const std::string& text, const std::string& pattern) {
    size_t pos_text = 0;
    size_t pos_pattern = 0;
    // Match till first *
    while (pos_text < text.size() && pos_pattern < pattern.size() && pattern[pos_pattern] != '*') {
        if (text[pos_text] != pattern[pos_pattern] && pattern[pos_pattern] != '?') {
            return false;
        }
        ++pos_text;
        ++pos_pattern;
    }
    auto start_text = std::string::npos;
    auto start_pattern = std::string::npos;
    while (pos_text < text.size()) {
        if (pattern[pos_pattern] == '*') {
            ++pos_pattern;
            // End with *
            if (pattern.size() == pos_pattern) {
                break;
            }
            start_text = pos_text;
            start_pattern = pos_pattern;
        } else if (text[pos_text] == pattern[pos_pattern] || pattern[pos_pattern] == '?') {
            ++pos_text;
            ++pos_pattern;
        } else if (start_pattern != std::string::npos) {
            // Cover the segment with * and retry
            ++start_text;
            pos_pattern = start_pattern;
            pos_text = start_text;
        } else {
            // Can not retry
            return false;
        }
    }
    while (pos_pattern < pattern.size() && pattern[pos_pattern] == '*') {
        ++pos_pattern;
    }
    return pattern.size() == pos_pattern;
}

} // namespace ingress_drone_explorer