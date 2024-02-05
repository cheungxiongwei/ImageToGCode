- [基于二值化图像转GCode的斜向扫描实现](#基于二值化图像转gcode的斜向扫描实现)
  - [什么是斜向扫描](#什么是斜向扫描)
  - [斜向扫描代码示例](#斜向扫描代码示例)

# 基于二值化图像转GCode的斜向扫描实现

## 什么是斜向扫描

![Diagonal-Scanning](./asset/斜向扫描.png)

在激光雕刻中，斜向扫描（Diagonal Scanning）是一种雕刻技术，其中激光头沿着对角线方向来回移动，而不是沿着水平或垂直方向。这种扫描方式的目的是在雕刻过程中更均匀地覆盖整个图像表面，从而提高雕刻的效果。

斜向扫描的过程包括以下步骤：

1. 起始位置： 激光头移动到图像的起始位置。

2. 斜向移动： 激光头沿对角线方向开始移动，逐行或逐列地刻蚀图像表面。与水平或垂直扫描不同，激光头沿着斜线移动，使得刻蚀路径更加均匀。

3. 返回： 当一行或一列刻蚀完成后，激光头返回到图像的另一侧，准备进行下一行或下一列的刻蚀。

4. 循环重复： 重复以上步骤，直到整个图像都被刻蚀完成。

相比于传统的水平或垂直扫描，斜向扫描能够减少在图像表面上可能出现的痕迹或线条，使得雕刻效果更为平滑和均匀。这对于一些需要高质量表面的雕刻应用，如艺术品或装饰品制作，具有重要意义。

选择采用斜向扫描还是其他扫描方式通常取决于具体的雕刻需求和设计目标。一些激光雕刻机允许用户根据实际情况选择不同的扫描方式以获得最佳的雕刻效果。

## 斜向扫描代码示例
```c++
#include <optional>
#include <string>
#include <print>
#include <format>
#include <vector>
#include <fstream>
#include <ranges>

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

inline bool ExportGCode(const std::string &fileName, std::vector<std::string> &&gcode) {
    std::fstream file;
    file.open(fileName, std::ios_base::out | std::ios_base::trunc);
    if(!file.is_open()) {
        return false;
    }

    for(auto &&v: gcode | std::views::transform([](auto item) { return item += "\n"; })) {
        file.write(v.c_str(), v.length());
    }

    return true;
}

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
        if constexpr (false) {
            // 单斜向
            for(int x = std::min(k, imageHeight - 1); x >= 0; --x) {
                int y = k - x;
                if(y < imageWidth) {
                    if(auto const value = image[y][x];value) {
                        command.emplace_back(G1 {x, y, 1000});  // 最大激光功率 S=1000
                    } else {
                        command.emplace_back(G0 {x, y, 0});
                    }
                }
            }
        }else {
            // 双斜向
            if((k & 1) == 0) {
                // even
                for(int x = std::min(k, imageHeight - 1); x >= 0; --x) {
                    int y = k - x;
                    if(x < imageHeight && y < imageWidth) {
                        if(auto const value = image[y][x];value) {
                            command.emplace_back(G1 {x, y, 1000});  // 最大激光功率 S=1000
                        } else {
                            command.emplace_back(G0 {x, y, 0});
                        }
                    }
                }
            }else {
                // odd
                for(int y = std::min(k, imageWidth - 1); y >= 0; --y) {
                    int x = k - y;
                    if(x < imageHeight && y < imageWidth) {
                        if(auto const value = image[y][x];value) {
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
    ExportGCode("gcode.nc",std::move(command));
    std::println("Export data to gcode.nc");
    return 0;
}
```
单向斜向扫描
![Diagonal-Scanning](./asset/单向斜向扫描-仿真.png)

双向斜向扫描
![Diagonal-Scanning](./asset/双向斜向扫描-仿真.png)

上述示例展示了一个10x10的二维图像数据，其中0表示非激光刻蚀部分，1表示进行激光刻蚀的区域。通过遍历图像数据，代码生成了相应的G代码指令序列，用于描述激光头在工件表面的运动路径。