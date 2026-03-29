#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace longhorn {

// Case-insensitive fuzzy match: all characters in pattern appear in order in target
[[nodiscard]] bool fuzzy_match(std::string_view pattern, std::string_view target);

// Filter items by fuzzy matching against input, returns indices into original vector
[[nodiscard]] std::vector<std::size_t> filter_items(
    std::string_view input,
    const std::vector<std::string>& items
);

// Filter and sort by history score (descending), then alphabetically
[[nodiscard]] std::vector<std::size_t> filter_items(
    std::string_view input,
    const std::vector<std::string>& items,
    const std::function<int(const std::string&)>& scorer
);

} // namespace longhorn
