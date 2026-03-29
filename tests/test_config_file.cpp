#include <doctest/doctest.h>
#include <longhorn/config_file.hpp>

using longhorn::ConfigFile;

TEST_CASE("ConfigFile: basic section and key parsing") {
    ConfigFile cfg;
    cfg.parse(R"(
[general]
prompt = >>
font_size = 20

[appearance]
corner_radius = 12.5
tint_r = 0.1
)");

    CHECK(cfg.get("general", "prompt") == ">>");
    CHECK(cfg.get_int("general", "font_size") == 20);
    CHECK(cfg.get_float("appearance", "corner_radius") == doctest::Approx(12.5f));
    CHECK(cfg.get_float("appearance", "tint_r") == doctest::Approx(0.1f));
}

TEST_CASE("ConfigFile: comments and blank lines") {
    ConfigFile cfg;
    cfg.parse(R"(
# This is a comment
; This is also a comment

[section]
key = value
# inline section comment
other = stuff
)");

    CHECK(cfg.get("section", "key") == "value");
    CHECK(cfg.get("section", "other") == "stuff");
}

TEST_CASE("ConfigFile: default values for missing keys") {
    ConfigFile cfg;
    cfg.parse("[general]\nprompt = >\n");

    CHECK(cfg.get("general", "missing", "default") == "default");
    CHECK(cfg.get_int("general", "missing", 42) == 42);
    CHECK(cfg.get_float("general", "missing", 3.14f) == doctest::Approx(3.14f));
    CHECK(cfg.get_bool("general", "missing", true) == true);
}

TEST_CASE("ConfigFile: missing section returns default") {
    ConfigFile cfg;
    cfg.parse("[general]\nkey = val\n");

    CHECK(cfg.get("nonexistent", "key", "nope") == "nope");
}

TEST_CASE("ConfigFile: boolean parsing") {
    ConfigFile cfg;
    cfg.parse(R"(
[bools]
a = true
b = false
c = 1
d = 0
e = yes
f = no
g = TRUE
)");

    CHECK(cfg.get_bool("bools", "a") == true);
    CHECK(cfg.get_bool("bools", "b") == false);
    CHECK(cfg.get_bool("bools", "c") == true);
    CHECK(cfg.get_bool("bools", "d") == false);
    CHECK(cfg.get_bool("bools", "e") == true);
    CHECK(cfg.get_bool("bools", "f") == false);
    CHECK(cfg.get_bool("bools", "g") == true);
}

TEST_CASE("ConfigFile: has() checks") {
    ConfigFile cfg;
    cfg.parse("[s]\nk = v\n");

    CHECK(cfg.has("s", "k"));
    CHECK_FALSE(cfg.has("s", "missing"));
    CHECK_FALSE(cfg.has("missing", "k"));
}

TEST_CASE("ConfigFile: whitespace trimming") {
    ConfigFile cfg;
    cfg.parse("  [  general  ]  \n  key  =  value  \n");

    // Section name gets trimmed, key and value get trimmed
    CHECK(cfg.get("general", "key") == "value");
}

TEST_CASE("ConfigFile: keys before any section go to empty section") {
    ConfigFile cfg;
    cfg.parse("orphan = val\n[s]\nk = v\n");

    CHECK(cfg.get("", "orphan") == "val");
    CHECK(cfg.get("s", "k") == "v");
}

TEST_CASE("ConfigFile: load nonexistent file returns false") {
    ConfigFile cfg;
    CHECK_FALSE(cfg.load("/tmp/nonexistent_longhorn_test_config.ini"));
}

TEST_CASE("default_config_path returns non-empty") {
    auto path = longhorn::default_config_path();
    // Should at least contain "longhorn" if HOME is set
    if (!path.empty()) {
        CHECK(path.find("longhorn") != std::string::npos);
    }
}
