#include <doctest/doctest.h>
#include <longhorn/history.hpp>

#include <cstdio>
#include <filesystem>
#include <fstream>

TEST_CASE("History record and score") {
    auto tmp = std::filesystem::temp_directory_path() / "longhorn_test_history";
    std::filesystem::remove(tmp);

    longhorn::History h(tmp.string());

    CHECK(h.score("foo") == 0);

    h.record("foo");
    CHECK(h.score("foo") == 1);

    h.record("foo");
    h.record("bar");
    CHECK(h.score("foo") == 2);
    CHECK(h.score("bar") == 1);

    std::filesystem::remove(tmp);
}

TEST_CASE("History save and load") {
    auto tmp = std::filesystem::temp_directory_path() / "longhorn_test_history_sl";
    std::filesystem::remove(tmp);

    {
        longhorn::History h(tmp.string());
        h.record("alpha");
        h.record("alpha");
        h.record("beta");
        h.save();
    }

    {
        longhorn::History h(tmp.string());
        CHECK(h.score("alpha") == 0); // not loaded yet
        h.load();
        CHECK(h.score("alpha") == 2);
        CHECK(h.score("beta") == 1);
        CHECK(h.score("gamma") == 0);
    }

    std::filesystem::remove(tmp);
}

TEST_CASE("History creates parent dirs") {
    auto tmp = std::filesystem::temp_directory_path() / "longhorn_test_nested" / "sub" / "history";
    std::filesystem::remove_all(std::filesystem::temp_directory_path() / "longhorn_test_nested");

    longhorn::History h(tmp.string());
    h.record("test");
    h.save();

    CHECK(std::filesystem::exists(tmp));

    std::filesystem::remove_all(std::filesystem::temp_directory_path() / "longhorn_test_nested");
}
