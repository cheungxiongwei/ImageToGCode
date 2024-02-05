#include <string>
#include <print>
#include <format>
#include <vector>
#include "Plane.h"

// 单向扫描 UnidirectionalScanning
int main() {
    constexpr int imageWidth  = 10;
    constexpr int imageHeight = 10;
    // clang-format off
    // 假设图像数据，0表示非激光刻蚀部分，1表示进行激光刻蚀的区域
    constexpr int image[imageWidth][imageHeight] = {
        {0, 1, 1, 1, 0, 1, 1, 1, 0, 0}, // G0 G1 G1 G1 G0 G1 G1 G1 G0 G0
        {1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
        {1, 1, 1, 0, 0, 1, 1, 1, 1, 1},
        {0, 0, 1, 1, 1, 1, 1, 0, 0, 0},
        {1, 1, 0, 0, 1, 0, 0, 1, 1, 1},
        {0, 1, 1, 0, 0, 1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1, 0, 1, 0, 0, 0},
        {0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };
    // clang-format on

    std::vector<std::string> command;
    for(int y = 0; y < imageHeight; ++y) {
        command.emplace_back(G0 {0, y, 0});
        for(int x = 0; x < imageWidth; ++x) {
            if(auto const value = image[y][x]; value) {
                command.emplace_back(G1 {x, std::nullopt, 1000});  // 最大激光功率 S=1000
            } else {
                command.emplace_back(G0 {x, std::nullopt, 0});
            }
        }
    }

    // 导出GCode
    Plane::create()->setCommand(std::move(command)).builder().exportGCode("gcode.nc");
    std::println("Export data to gcode.nc");
    return 0;
}
