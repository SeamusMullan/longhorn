#include <longhorn/app.hpp>
#include <longhorn/config_file.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <cstring>

static void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog << " [-b] [-l lines] [-p prompt] [-fn font] [-fs size] [-c config]\n"
              << "  -b          appear at bottom of screen\n"
              << "  -l lines    number of vertical lines (0 = horizontal)\n"
              << "  -p prompt   prompt string (default: >)\n"
              << "  -fn font    path to TTF font\n"
              << "  -fs size    font size in points (default: 16)\n"
              << "  -c config   path to INI config file\n"
              << "  -nh         disable selection history\n";
}

int main(int argc, char* argv[]) {
    longhorn::Config config;

    // First pass: look for -c flag to get config path
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            config.config_path = argv[++i];
        }
    }

    // Load config file (file values first, CLI overrides after)
    std::string cfg_path = config.config_path.empty()
                               ? longhorn::default_config_path()
                               : config.config_path;
    if (!cfg_path.empty()) {
        longhorn::load_config(config, cfg_path);
    }

    // Second pass: CLI args override config file
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
        } else if (std::strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            ++i; // already handled
        } else if (std::strcmp(argv[i], "-nh") == 0) {
            config.use_history = false;
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
