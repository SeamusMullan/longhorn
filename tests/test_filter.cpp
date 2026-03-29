#include <doctest/doctest.h>
#include <longhorn/filter.hpp>

using namespace longhorn;

TEST_CASE("fuzzy_match") {
    CHECK(fuzzy_match("", "anything"));
    CHECK(fuzzy_match("abc", "aXbXc"));
    CHECK(fuzzy_match("ff", "firefox"));
    CHECK(fuzzy_match("FF", "firefox")); // case insensitive
    CHECK(fuzzy_match("fire", "firefox"));
    CHECK_FALSE(fuzzy_match("zz", "firefox"));
    CHECK_FALSE(fuzzy_match("ba", "abc")); // wrong order
}

TEST_CASE("filter_items") {
    std::vector<std::string> items = {"firefox", "chromium", "alacritty", "foot", "kitty"};

    SUBCASE("empty input returns all") {
        auto result = filter_items("", items);
        CHECK(result.size() == items.size());
    }

    SUBCASE("filters correctly") {
        auto result = filter_items("fi", items);
        CHECK(result.size() == 1);
        CHECK(items[result[0]] == "firefox");
    }

    SUBCASE("fuzzy matching") {
        auto result = filter_items("ft", items);
        // "foot" matches f...t
        bool has_foot = false;
        for (auto idx : result) {
            if (items[idx] == "foot") has_foot = true;
        }
        CHECK(has_foot);
    }

    SUBCASE("no matches") {
        auto result = filter_items("zzz", items);
        CHECK(result.empty());
    }
}
