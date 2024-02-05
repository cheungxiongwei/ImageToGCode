#pragma once
#include <optional>
#include <vector>
#include <fstream>
#include <ranges>
#include <format>
#include <memory>
#include <string>

class Plane
{
public:
    static std::shared_ptr<Plane> create() { return std::make_shared<Plane>(); }

    Plane &setCommand(std::vector<std::string> &&command) {
        std::exchange(this->command, command);
        return *this;
    }

    Plane &builder() {
        std::vector<std::string> header;
        header.emplace_back("G17G21G90G54");                               // XY平面;单位毫米;绝对坐标模式;选择G54坐标系
        header.emplace_back(std::format("F{:d}", 30000));                  // 移动速度 毫米/每分钟
        header.emplace_back(std::format("G0 X{:.3f} Y{:.3f}", 0.f, 0.f));  // 设置工作起点及偏移
        header.emplace_back("M4 S0");                                      // 激光模式

        std::vector<std::string> footer;
        footer.emplace_back("S0");
        footer.emplace_back("M5");

        command.insert_range(command.begin(), header);
        command.append_range(footer);

        return *this;
    }

    bool exportGCode(const std::string &fileName) {
        std::fstream file;
        file.open(fileName, std::ios_base::out | std::ios_base::trunc);
        if(!file.is_open()) {
            return false;
        }

        for(auto &&v: command | std::views::transform([](auto item) { return item += "\n"; })) {
            file.write(v.c_str(), v.length());
        }

        return true;
    }

private:
    std::vector<std::string> command;
};

struct G0 {
    std::optional<float> x, y;
    std::optional<int> s;

    std::string toString() {
        std::string command = "G0";
        if(x.has_value()) {
            command += std::format(" X{:.3f}", x.value());
        }
        if(y.has_value()) {
            command += std::format(" Y{:.3f}", y.value());
        }
        if(s.has_value()) {
            command += std::format(" S{:d}", s.value());
        }
        return command;
    }

    explicit operator std::string() const {
        std::string command = "G0";
        if(x.has_value()) {
            command += std::format(" X{:.3f}", x.value());
        }
        if(y.has_value()) {
            command += std::format(" Y{:.3f}", y.value());
        }
        if(s.has_value()) {
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
        if(x.has_value()) {
            command += std::format(" X{:.3f}", x.value());
        }
        if(y.has_value()) {
            command += std::format(" Y{:.3f}", y.value());
        }
        if(s.has_value()) {
            command += std::format(" S{:d}", s.value());
        }
        return command;
    }

    explicit operator std::string() const {
        std::string command = "G1";
        if(x.has_value()) {
            command += std::format(" X{:.3f}", x.value());
        }
        if(y.has_value()) {
            command += std::format(" Y{:.3f}", y.value());
        }
        if(s.has_value()) {
            command += std::format(" S{:d}", s.value());
        }
        return command;
    }
};
