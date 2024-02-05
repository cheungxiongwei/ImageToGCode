#include <fstream>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

inline bool ExportGCode(const std::string &fileName, std::vector<std::string> &&gcode) {
    std::fstream file;
    file.open(fileName, std::ios_base::out | std::ios_base::trunc);
    if (!file.is_open()) {
        return false;
    }

    for (auto &&v: gcode | std::views::transform([](auto item) { return item += "\n"; })) {
        file.write(v.c_str(), v.length());
    }

    return true;
}

struct G0 {
    std::optional<float> x, y;
    std::optional<int> s;

    std::string toString() {
        std::string command = "G0";
        if (x.has_value()) {
            command += std::format(" X{:.3f}", x.value());
        }
        if (y.has_value()) {
            command += std::format(" Y{:.3f}", y.value());
        }
        if (s.has_value()) {
            command += std::format(" S{:d}", s.value());
        }
        return command;
    }

    explicit operator std::string() const {
        std::string command = "G0";
        if (x.has_value()) {
            command += std::format(" X{:.3f}", x.value());
        }
        if (y.has_value()) {
            command += std::format(" Y{:.3f}", y.value());
        }
        if (s.has_value()) {
            command += std::format(" S{:d}", s.value());
        }
        return command;
    }
};

struct G1 {
    std::optional<float> x, y;
    std::optional<int> s;

    std::string toString() {
        std::string command = "G1";
        if (x.has_value()) {
            command += std::format(" X{:.3f}", x.value());
        }
        if (y.has_value()) {
            command += std::format(" Y{:.3f}", y.value());
        }
        if (s.has_value()) {
            command += std::format(" S{:d}", s.value());
        }
        return command;
    }

    explicit operator std::string() const {
        std::string command = "G1";
        if (x.has_value()) {
            command += std::format(" X{:.3f}", x.value());
        }
        if (y.has_value()) {
            command += std::format(" Y{:.3f}", y.value());
        }
        if (s.has_value()) {
            command += std::format(" S{:d}", s.value());
        }
        return command;
    }
};
