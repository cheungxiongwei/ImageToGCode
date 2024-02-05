#include <string>
#include <print>
#include <format>
#include <vector>
#include "Plane.h"

// 斜向扫描 DiagonalScanning
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
    for(int k = 0; k < imageHeight + imageWidth - 1 /*cond = height + width - 1*/; ++k) {  // 线条数
        if constexpr(false) {
            // 单斜向
            for(int x = std::min(k, imageHeight - 1); x >= 0; --x) {
                int y = k - x;
                if(y < imageWidth) {
                    if(auto const value = image[y][x]; value) {
                        command.emplace_back(G1 {x, y, 1000});  // 最大激光功率 S=1000
                    } else {
                        command.emplace_back(G0 {x, y, 0});
                    }
                }
            }
        } else {
            // 双斜向
            if((k & 1) == 0) {
                // even
                for(int x = std::min(k, imageHeight - 1); x >= 0; --x) {
                    int y = k - x;
                    if(x < imageHeight && y < imageWidth) {
                        if(auto const value = image[y][x]; value) {
                            command.emplace_back(G1 {x, y, 1000});  // 最大激光功率 S=1000
                        } else {
                            command.emplace_back(G0 {x, y, 0});
                        }
                    }
                }
            } else {
                // odd
                for(int y = std::min(k, imageWidth - 1); y >= 0; --y) {
                    int x = k - y;
                    if(x < imageHeight && y < imageWidth) {
                        if(auto const value = image[y][x]; value) {
                            command.emplace_back(G1 {x, y, 1000});  // 最大激光功率 S=1000
                        } else {
                            command.emplace_back(G0 {x, y, 0});
                        }
                    }
                }
            }
        }
    }

    // 导出GCode
    Plane::create()->setCommand(std::move(command)).builder().exportGCode("gcode.nc");
    std::println("Export data to gcode.nc");
    return 0;
}
