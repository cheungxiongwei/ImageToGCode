- [基于二值化图像转GCode的螺旋扫描实现](#基于二值化图像转gcode的螺旋扫描实现)
  - [什么是螺旋扫描](#什么是螺旋扫描)
  - [螺旋扫描代码示例](#螺旋扫描代码示例)

# 基于二值化图像转GCode的螺旋扫描实现

## 什么是螺旋扫描
![Diagonal-Scanning](./asset/螺旋扫描.png)

螺旋扫描（Spiral Scanning）是激光雕刻中一种特殊的扫描方式，其特点是激光头按照螺旋形状逐渐向外移动，覆盖整个图像表面。与传统的水平、垂直或对角线扫描方式不同，螺旋扫描以一种旋转螺旋的方式进行移动，创造出一种独特的雕刻效果。

螺旋扫描的基本过程如下：

1. 起始位置： 激光头移动到图像的起始位置。

2. 螺旋移动： 激光头按照螺旋形状逐渐向外移动，同时进行雕刻。通常，激光头以螺旋的方式覆盖整个图像表面，从中心向外辐射。

3. 循环重复： 重复螺旋移动的过程，直到整个图像都被刻蚀完成。

螺旋扫描的优点之一是可以在图像表面创建一种渐变的雕刻效果。由于激光头的移动轨迹呈螺旋形状，不同区域的刻蚀深度和密度可能有所不同，从而形成独特的视觉效果。这种扫描方式通常用于一些艺术品、装饰品或个性化雕刻项目，以增加雕刻作品的艺术感和独特性。

在选择扫描方式时，螺旋扫描可能不适用于所有应用，但在某些情况下，它为雕刻师提供了一种创造性的选项，以实现独特和有趣的雕刻效果。

## 螺旋扫描代码示例
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
    auto internal = [&](int x,int y){
        if(auto const value = image[y][x];value) {
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
    ExportGCode("gcode.nc",std::move(command));
    std::println("Export data to gcode.nc");
    return 0;
}
```

![Diagonal-Scanning](./asset/螺旋扫描-仿真.png)

上述示例展示了一个10x10的二维图像数据，其中0表示非激光刻蚀部分，1表示进行激光刻蚀的区域。通过遍历图像数据，代码生成了相应的G代码指令序列，用于描述激光头在工件表面的运动路径。