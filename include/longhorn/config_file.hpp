#pragma once

#include <string>
#include <unordered_map>

namespace longhorn {

// Simple INI file parser. Supports [section], key = value, # comments.
class ConfigFile {
public:
    // Parse an INI file from disk. Returns false if file cannot be opened.
    bool load(const std::string& path);

    // Parse INI content from a string (useful for testing).
    void parse(const std::string& content);

    // Get a value. Returns empty string if not found.
    [[nodiscard]] std::string get(const std::string& section,
                                  const std::string& key,
                                  const std::string& default_value = "") const;

    [[nodiscard]] float get_float(const std::string& section,
                                  const std::string& key,
                                  float default_value = 0.0f) const;

    [[nodiscard]] int get_int(const std::string& section,
                              const std::string& key,
                              int default_value = 0) const;

    [[nodiscard]] bool get_bool(const std::string& section,
                                const std::string& key,
                                bool default_value = false) const;

    [[nodiscard]] bool has(const std::string& section,
                           const std::string& key) const;

private:
    // section -> (key -> value)
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>> data_;
};

// Resolve the default config path (~/.config/longhorn/config.ini)
std::string default_config_path();

} // namespace longhorn
