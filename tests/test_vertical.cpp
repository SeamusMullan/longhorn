#include <doctest/doctest.h>
#include <algorithm>

// Test scroll offset logic independently (mirrors App::Impl::update_scroll)
static void update_scroll(int& scroll_offset, int selected, int lines) {
    if (lines <= 0) return;
    if (selected < scroll_offset) scroll_offset = selected;
    if (selected >= scroll_offset + lines) scroll_offset = selected - lines + 1;
    scroll_offset = std::max(0, scroll_offset);
}

TEST_CASE("vertical scroll offset") {
    int scroll = 0;
    int lines = 3;

    SUBCASE("initial state") {
        update_scroll(scroll, 0, lines);
        CHECK(scroll == 0);
    }

    SUBCASE("selection within view does not scroll") {
        update_scroll(scroll, 2, lines);
        CHECK(scroll == 0);
    }

    SUBCASE("selection past end scrolls down") {
        update_scroll(scroll, 3, lines);
        CHECK(scroll == 1);

        update_scroll(scroll, 5, lines);
        CHECK(scroll == 3);
    }

    SUBCASE("selection before start scrolls up") {
        scroll = 4;
        update_scroll(scroll, 2, lines);
        CHECK(scroll == 2);
    }

    SUBCASE("scroll never goes negative") {
        scroll = 0;
        update_scroll(scroll, 0, lines);
        CHECK(scroll >= 0);
    }

    SUBCASE("lines=0 is no-op") {
        scroll = 5;
        update_scroll(scroll, 0, 0);
        CHECK(scroll == 5); // unchanged
    }
}
