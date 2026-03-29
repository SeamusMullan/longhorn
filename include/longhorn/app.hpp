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
};

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
