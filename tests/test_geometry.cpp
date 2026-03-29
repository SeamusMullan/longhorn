#include <doctest/doctest.h>
#include <longhorn/geometry.hpp>

using namespace longhorn;

TEST_CASE("Layout::resolve") {
    SUBCASE("TopBar") {
        Layout layout{.preset = LayoutPreset::TopBar};
        auto rect = layout.resolve(1920, 1080, 32);
        CHECK(rect.x == 0);
        CHECK(rect.y == 0);
        CHECK(rect.w == 1920);
        CHECK(rect.h == 32);
    }

    SUBCASE("BottomBar") {
        Layout layout{.preset = LayoutPreset::BottomBar};
        auto rect = layout.resolve(1920, 1080, 32);
        CHECK(rect.x == 0);
        CHECK(rect.y == 1048);
        CHECK(rect.w == 1920);
        CHECK(rect.h == 32);
    }

    SUBCASE("Centered") {
        Layout layout{.preset = LayoutPreset::Centered};
        auto rect = layout.resolve(1920, 1080, 32);
        CHECK(rect.w == 960);
        CHECK(rect.x == 480);
        CHECK(rect.y == 360);
    }

    SUBCASE("Custom") {
        Layout layout{
            .preset = LayoutPreset::Custom,
            .custom_rect = {0.1f, 0.2f, 0.5f, 0.1f},
        };
        auto rect = layout.resolve(1920, 1080, 32);
        CHECK(rect.x == 192);
        CHECK(rect.y == 216);
        CHECK(rect.w == 960);
        CHECK(rect.h == 108);
    }
}

TEST_CASE("Layout::preset_from_string") {
    CHECK(Layout::preset_from_string("top") == LayoutPreset::TopBar);
    CHECK(Layout::preset_from_string("bottom") == LayoutPreset::BottomBar);
    CHECK(Layout::preset_from_string("center") == LayoutPreset::Centered);
    CHECK(Layout::preset_from_string("unknown") == LayoutPreset::TopBar);
}

TEST_CASE("GeometryState interpolation") {
    GeometryState state;
    state.current = {0, 0, 100, 32};
    state.target = {0, 0, 100, 32};

    SUBCASE("no movement when at target") {
        state.update(0.016f);
        auto r = state.interpolated();
        CHECK(r.x == 0);
        CHECK(r.w == 100);
    }

    SUBCASE("interpolates toward target") {
        state.set_target({100, 50, 200, 64});
        CHECK(state.interpolated().x == 0); // starts at current
        state.update(0.016f);
        auto r = state.interpolated();
        CHECK(r.x > 0);
        CHECK(r.x < 100);
    }

    SUBCASE("reaches target eventually") {
        state.set_target({100, 50, 200, 64});
        for (int i = 0; i < 100; ++i) state.update(0.016f);
        auto r = state.interpolated();
        CHECK(r.x == 100);
        CHECK(r.y == 50);
        CHECK(r.w == 200);
        CHECK(r.h == 64);
    }
}
