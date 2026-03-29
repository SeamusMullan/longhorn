#pragma once

#include <memory>
#include <string>
#include <vector>

namespace longhorn {

struct Config {
    std::string prompt = ">";
    std::string font_path;
    int font_size = 16;
    bool bottom = false;
    int lines = 0; // 0 = horizontal mode, >0 = vertical with N lines

    // Layout preset name: "top", "bottom", "centered", "left", "right"
    std::string layout_preset = "top";

    // Glass appearance
    float corner_radius = 0.0f;
    float tint_r = 0.15f;
    float tint_g = 0.20f;
    float tint_b = 0.35f;
    float tint_a = 0.4f;

    // Config file path (empty = auto-detect ~/.config/longhorn/config.ini)
    std::string config_path;

    // History
    bool use_history = true;
    std::string history_path; // empty = ~/.cache/longhorn/history
};

// Load an INI config file and apply values to config.
// CLI args applied after this call will override file values.
void load_config(Config& config, const std::string& path);

class App {
public:
    explicit App(Config config, std::vector<std::string> items);
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    // Returns selected item, or empty string if cancelled
    [[nodiscard]] std::string run();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace longhorn
