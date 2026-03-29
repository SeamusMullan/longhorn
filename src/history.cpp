#include <longhorn/history.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace longhorn {

History::History(const std::string& path) : path_(path) {}

void History::load() {
    std::ifstream in(path_);
    if (!in.is_open()) return;

    std::string line;
    while (std::getline(in, line)) {
        auto tab = line.find('\t');
        if (tab == std::string::npos) continue;

        int count = 0;
        try {
            count = std::stoi(line.substr(0, tab));
        } catch (...) {
            continue;
        }

        std::string item = line.substr(tab + 1);
        if (!item.empty()) {
            counts_[item] = count;
        }
    }
}

void History::save() {
    namespace fs = std::filesystem;
    fs::path p(path_);
    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path());
    }

    std::ofstream out(path_);
    if (!out.is_open()) return;

    for (const auto& [item, count] : counts_) {
        out << count << '\t' << item << '\n';
    }
}

void History::record(const std::string& item) {
    ++counts_[item];
}

int History::score(const std::string& item) const {
    auto it = counts_.find(item);
    return it != counts_.end() ? it->second : 0;
}

} // namespace longhorn
