# 深入理解G0和G1指令_C++中的实现与激光雕刻应用
摘要：本文深入解析了数控编程和激光雕刻中的两个关键指令——G0和G1。通过一个简单的C++实现示例，我们展示了如何使用结构体和可选值灵活表示这两个指令，使其适应不同的应用需求。这种设计模式不仅提高了代码的可读性，还为激光雕刻应用提供了更灵活的编程选项。阅读本文，深入了解这两个指令的重要性，以及它们在激光加工领域中的实际应用。

系列文章
- 深入理解G0和G1指令：C++中的实现与激光雕刻应用
- 基于二值化图像转GCode的单向扫描实现
- 基于二值化图像转GCode的双向扫描实现
- 基于二值化图像转GCode的斜向扫描实现
- 基于二值化图像转GCode的螺旋扫描实现
- 基于OpenCV灰度图像转GCode的单向扫描实现
- 基于OpenCV灰度图像转GCode的双向扫描实现
- 基于OpenCV灰度图像转GCode的斜向扫描实现
- 基于OpenCV灰度图像转GCode的螺旋扫描实现

---

- [深入理解G0和G1指令：C++中的实现与激光雕刻应用](#深入理解g0和g1指令c中的实现与激光雕刻应用)
    - [G0和G1概述](#g0和g1概述)
        - [G0：快速移动](#g0快速移动)
        - [G1：线性插补运动](#g1线性插补运动)
    - [C++实现示例](#c实现示例)
    - [使用示例](#使用示例)
    - [优化](#优化)
    - [总结](#总结)

# 深入理解G0和G1指令：C++中的实现与激光雕刻应用

在激光雕刻领域，G0和G1指令是至关重要的命令，用于控制激光雕刻机的运动。在本篇博客中，我们将深入探讨这两个指令的意义，并展示一个用C++23实现的简单示例。

## G0和G1概述
G0和G1是G代码中的两种基本运动指令，广泛应用于激光雕刻等领域。它们分别代表了快速移动和线性插补运动。

### G0：快速移动
快速运动是激光器在不进行切割或雕刻的情况下以较高的速度快速移动到目标位置。这是一种非加工移动，通常用于快速定位或避免工件碰撞。

### G1：线性插补运动
G1指令用于进行线性插补运动，即在两个点之间以较慢的速度沿直线路径移动。这是实际的切削或激光刻蚀运动，用于加工工件表面。

## C++实现示例
在 2D 激光雕刻领域，通常只需关注 G0 和 G1 指令,他们的格式如下：

**Rapid Linear Motion — G0**
```
G0 [X<pos>] [Y<pos>] [S<power>]
```

**Linear Motion at Feed Rate — G1**
```
G1 [X<pos>] [Y<pos>] [S<power>]
```

其中，X、Y 是坐标轴的数值，表示激光器在各个轴上的目标位置。

通过在运动控制程序中使用 G1 或 G0 指令，可以控制激光器的运动，实现进给运动或快速运动，具体取决于所使用的指令。

S 表示激光器功率，激光器区间值范围 [0,1000]。

> 注意：每个行业不同，指令的定义略有不同。
> 例如 3D打印领域 添加一个层(Layer)的概念,他们就会存在 Z 轴，那么他们的指令就如下：
> `G0 [E<pos>] [F<rate>] [S<power>] [X<pos>] [Y<pos>] [Z<pos>]`
> `G1 [E<pos>] [F<rate>] [S<power>] [X<pos>] [Y<pos>] [Z<pos>]`
> 具体的格式可能因运动控制系统的不同而有所差异，建议查阅相应运动控制系统的文档以获取准确的指令格式信息。

下面是一个简单的C++示例，展示了如何使用结构体和可选值（optional）来表示G0和G1指令：

```c++
#include <optional>
#include <string>
#include <format>

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
```

这个示例使用了C++中的结构体和可选值，使得G0和G1指令的参数可以选择性地存在。这种设计符合G代码的灵活性，因为在实际应用中，并非所有的参数都是必需的。

## 使用示例
下面是一个使用这些结构体的简单示例：

```c++
#include <optional>
#include <string>
#include <print>

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

int main()
{
    std::println("{}", G0{0,0,0}.toString());
    std::println("{}", G1{6,std::nullopt,1000}.toString());
    return 0;
}
```
打印输出：
```gcode
G0 X0.000 Y0.000 S0
G1 X6.000 S1000
```

这段代码展示了如何创建G0和G1指令对象，并输出它们的字符串表示形式。这样的设计使得在实际应用中可以方便地构建和修改G代码指令。

## GCode 优化
如果需要生成更简短 GCode ，例如没有小数时，不需要打印出精度信息。

使用以下代码

```c++
if (x.has_value()) {
    if (std::floor(x.value()) == x.value()) {
        // 如果浮点数没有小数点，不输出精度
        command += std::format(" X{:.0f}", x.value());
    } else {
        // 有小数点时输出指定精度
        command += std::format(" X{:.3f}", x.value());
    }
}
```

替换

```c++
if(x.has_value()) {
    command += std::format(" X{:.3f}", x.value());
}
```
完成 x 成员替换后，类似的 y 成员部分也需要替换。

这样替换后，最后输出

```gcode
G0 X0 Y0 S0
G1 X6 S1000
```

## 总结
通过这篇博客，我们深入了解了G0和G1指令在激光雕刻中的重要性，并展示了一个简单的C++实现示例。这种设计可以在实际的激光雕刻应用中发挥巨大的作用，为工程师和编程人员提供了更灵活、更可读的代码。在未来的激光加工领域中，这样的设计模式可能会进一步发展，以满足更复杂的加工需求。下一篇文章将学习二值化图像转GCode，并实际应用刚刚学到的G0和G1知识。