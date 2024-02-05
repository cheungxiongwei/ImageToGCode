#include <string>
#include <print>
#include <format>
#include <vector>
#include "Plane.h"

// 螺旋扫描 SpiralScanning
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
    auto internal = [&](int x, int y) {
        if(auto const value = image[y][x]; value) {
            command.emplace_back(G1 {x, y, 1000});  // 最大激光功率 S=1000
        } else {
            command.emplace_back(G0 {x, y, 0});
        }
    };
    int top = 0, bottom = imageHeight - 1, left = 0, right = imageWidth - 1;
    while(top <= bottom && left <= right) {
        for(int i = left; i <= right; ++i) {
            internal(top, i);
        }
        ++top;

        for(int i = top; i <= bottom; ++i) {
            internal(i, right);
        }
        --right;

        if(top <= bottom) {
            for(int i = right; i >= left; --i) {
                internal(bottom, i);
            }
            --bottom;
        }

        if(left <= right) {
            for(int i = bottom; i >= top; --i) {
                internal(i, left);
            }
            ++left;
        }
    }

    // 导出GCode
    Plane::create()->setCommand(std::move(command)).builder().exportGCode("gcode.nc");
    std::println("Export data to gcode.nc");
    return 0;
}
