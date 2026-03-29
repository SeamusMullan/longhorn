#include <longhorn/config_file.hpp>

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace longhorn {

static std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool ConfigFile::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::ostringstream ss;
    ss << file.rdbuf();
    parse(ss.str());
    return true;
}

void ConfigFile::parse(const std::string& content) {
    data_.clear();
    std::istringstream stream(content);
    std::string line;
    std::string current_section;

    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;

        if (line.front() == '[' && line.back() == ']') {
            current_section = trim(line.substr(1, line.size() - 2));
            continue;
        }

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        auto key = trim(line.substr(0, eq));
        auto value = trim(line.substr(eq + 1));
        data_[current_section][key] = value;
    }
}

std::string ConfigFile::get(const std::string& section,
                            const std::string& key,
                            const std::string& default_value) const {
    auto sit = data_.find(section);
    if (sit == data_.end()) return default_value;
    auto kit = sit->second.find(key);
    if (kit == sit->second.end()) return default_value;
    return kit->second;
}

float ConfigFile::get_float(const std::string& section,
                            const std::string& key,
                            float default_value) const {
    auto val = get(section, key);
    if (val.empty()) return default_value;
    try { return std::stof(val); }
    catch (...) { return default_value; }
}

int ConfigFile::get_int(const std::string& section,
                        const std::string& key,
                        int default_value) const {
    auto val = get(section, key);
    if (val.empty()) return default_value;
    try { return std::stoi(val); }
    catch (...) { return default_value; }
}

bool ConfigFile::get_bool(const std::string& section,
                          const std::string& key,
                          bool default_value) const {
    auto val = get(section, key);
    if (val.empty()) return default_value;
    // Normalize to lowercase
    std::string lower = val;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "true" || lower == "1" || lower == "yes") return true;
    if (lower == "false" || lower == "0" || lower == "no") return false;
    return default_value;
}

bool ConfigFile::has(const std::string& section, const std::string& key) const {
    auto sit = data_.find(section);
    if (sit == data_.end()) return false;
    return sit->second.count(key) > 0;
}

std::string default_config_path() {
    const char* xdg = std::getenv("XDG_CONFIG_HOME");
    if (xdg && xdg[0] != '\0') {
        return std::string(xdg) + "/longhorn/config.ini";
    }
    const char* home = std::getenv("HOME");
    if (home && home[0] != '\0') {
        return std::string(home) + "/.config/longhorn/config.ini";
    }
    return "";
}

} // namespace longhorn
