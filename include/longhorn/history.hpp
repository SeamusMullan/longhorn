#pragma once

#include <string>
#include <unordered_map>

namespace longhorn {

class History {
public:
    explicit History(const std::string& path);

    void load();
    void save();
    void record(const std::string& item);

    [[nodiscard]] int score(const std::string& item) const;

private:
    std::string path_;
    std::unordered_map<std::string, int> counts_;
};

} // namespace longhorn
