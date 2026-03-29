#include <longhorn/filter.hpp>
#include <algorithm>
#include <cctype>
#include <numeric>

namespace longhorn {

bool fuzzy_match(std::string_view pattern, std::string_view target) {
    if (pattern.empty()) return true;

    auto pi = pattern.begin();
    for (auto ti = target.begin(); ti != target.end() && pi != pattern.end(); ++ti) {
        if (std::tolower(static_cast<unsigned char>(*pi)) ==
            std::tolower(static_cast<unsigned char>(*ti))) {
            ++pi;
        }
    }
    return pi == pattern.end();
}

std::vector<std::size_t> filter_items(
    std::string_view input,
    const std::vector<std::string>& items
) {
    std::vector<std::size_t> result;
    if (input.empty()) {
        result.resize(items.size());
        std::iota(result.begin(), result.end(), 0);
        return result;
    }

    for (std::size_t i = 0; i < items.size(); ++i) {
        if (fuzzy_match(input, items[i])) {
            result.push_back(i);
        }
    }
    return result;
}

std::vector<std::size_t> filter_items(
    std::string_view input,
    const std::vector<std::string>& items,
    const std::function<int(const std::string&)>& scorer
) {
    auto result = filter_items(input, items);

    std::sort(result.begin(), result.end(),
        [&](std::size_t a, std::size_t b) {
            int sa = scorer(items[a]);
            int sb = scorer(items[b]);
            if (sa != sb) return sa > sb;
            return items[a] < items[b];
        });

    return result;
}

} // namespace longhorn
