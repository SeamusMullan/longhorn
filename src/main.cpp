#include <longhorn/app.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <cstring>

static void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog << " [-b] [-l lines] [-p prompt] [-fn font] [-fs size]\n"
              << "  -b          appear at bottom of screen\n"
              << "  -l lines    number of vertical lines (0 = horizontal)\n"
              << "  -p prompt   prompt string (default: >)\n"
              << "  -fn font    path to TTF font\n"
              << "  -fs size    font size in points (default: 16)\n";
}

int main(int argc, char* argv[]) {
    longhorn::Config config;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-b") == 0) {
            config.bottom = true;
        } else if (std::strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            config.lines = std::stoi(argv[++i]);
        } else if (std::strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            config.prompt = argv[++i];
        } else if (std::strcmp(argv[i], "-fn") == 0 && i + 1 < argc) {
            config.font_path = argv[++i];
        } else if (std::strcmp(argv[i], "-fs") == 0 && i + 1 < argc) {
            config.font_size = std::stoi(argv[++i]);
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    // Read items from stdin
    std::vector<std::string> items;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty()) {
            items.push_back(std::move(line));
        }
    }

    longhorn::App app(std::move(config), std::move(items));
    auto result = app.run();

    if (result.empty()) {
        return 1;
    }

    std::cout << result << '\n';
    return 0;
}
