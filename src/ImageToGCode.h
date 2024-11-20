#pragma once
#include <opencv2/opencv.hpp>
#include <fstream>
#include <print>
#include <algorithm>

#include "Common.hpp"

class ImageToGCode
{
public:
    // 激光模式
    enum class LaserMode {
        Cutting,    // 切割 M3 Constant Power
        Engraving,  // 雕刻 M4 Dynamic Power
    };

    // 扫描方式
    enum class ScanMode {
        Unidirection,  // 单向
        Bidirection,   // 双向
        Diagonal,      // 斜向
        Spiral,        // 螺旋
        Block,         // 分块 根据像素的灰度级别进行扫描，例如255像素分8个级别，那么0-32就是一个级别，32-64就是另外一个级别，以此类推。
        // (Block scanning is performed based on the gray level of the pixels. For example, 255 pixels are divided into 8 levels, then 0-32 is one level, 32-64 is another level, and so on.)
    };

    struct kEnumToStringLaserMode {
        constexpr std::string_view operator[](const LaserMode mode) const noexcept {
            switch(mode) {
                case LaserMode::Cutting: return "M3";
                case LaserMode::Engraving: return "M4";
            }
            return {};
        }

        constexpr LaserMode operator[](const std::string_view mode) const noexcept {
            if(mode.compare("M3")) {
                return LaserMode::Cutting;
            }
            if(mode.compare("M4")) {
                return LaserMode::Engraving;
            }
            return {};
        }
    };

    ImageToGCode() = default;

    ~ImageToGCode() = default;

    auto &setInputImage(const cv::Mat &mat) {
        this->mat = mat;
        return *this;
    }

    auto &setOutputTragetSize(double width, double height, double resolution = 10.0 /* lin/mm */) {
        this->width      = width;
        this->height     = height;
        this->resolution = resolution;
        return *this;
    }

    auto &builder() {
        command.clear();
        try {
            matToGCode();
        } catch(cv::Exception &e) {
            std::println("cv Exception {}", e.what());
        }

        std::vector<std::string> header;
        header.emplace_back("G17G21G90G54");                                             // XY平面;单位毫米;绝对坐标模式;选择G54坐标系(XY plane; unit mm; absolute coordinate mode; select G54 coordinate system)
        header.emplace_back(std::format("F{:d}", 30000));                                // 移动速度 毫米/每分钟(Moving speed mm/min)
        header.emplace_back(std::format("G0 X{:.3f} Y{:.3f}", 0.f, 0.f));                // 设置工作起点及偏移(Set the starting point and offset of the work)
        header.emplace_back(std::format("{} S0", kEnumToStringLaserMode()[laserMode]));  // 激光模式(laser mode)
        if(airPump.has_value()) {
            header.emplace_back(std::format("M16 S{:d}", 300));  // 打开气泵(Turn on the air pump)
        }

        std::vector<std::string> footer;
        footer.emplace_back("M5");
        if(airPump.has_value()) {
            footer.emplace_back("M9");  // 关闭气泵，保持 S300 功率(Turn off air pump and maintain S300 power)
        }

        command.insert_range(command.begin(), header);
        command.append_range(footer);

        return *this;
    }

    bool exportGCode(const std::string &fileName) {
        std::fstream file;
        file.open(fileName, std::ios_base::out | std::ios_base::trunc);
        if(!file.is_open()) {
            std::println("can not export gcode");
            return false;
        }

        for(auto &&v: command | std::views::transform([](auto item) { return item += "\n"; })) {
            file.write(v.c_str(), v.length());
        }

        return true;
    }

    auto setLaserMode(LaserMode mode) {
        laserMode = mode;
        return *this;
    }

    auto setScanMode(ScanMode mode) {
        scanMode = mode;
        return *this;
    }

private:
    void matToGCode() {
        assert(mat.channels() == 1);
        assert(std::isgreaterequal(resolution, 1e-5f));
        assert(!((width * resolution < 1.0) || (height * resolution < 1.0)));

        // different conversion strategy functions are called here

        switch(scanMode) {
            case ScanMode::Unidirection: unidirectionOptStrategy(); break;
            case ScanMode::Bidirection: bidirectionOptStrategy(); break;
            case ScanMode::Diagonal:  diagonalStrategy(); break;
            case ScanMode::Spiral: spiralStrategy(); break;
            case ScanMode::Block: break;
        }
    }

    // 单向扫描
    // One-way scanning
    // 未做任何优化处理，像素和G0、G1一一映射对应。
    // No optimization has been done, and pixels are mapped one-to-one to G0 and G1.
    void unidirectionStrategy() {
        cv::Mat image;
        cv::resize(mat, image, cv::Size(static_cast<int>(width * resolution), static_cast<int>(height * resolution)));
        for(int y = 0; y < image.rows; ++y) {
            command.emplace_back(G0(0, y / resolution, std::nullopt).toString());
            for(int x = 0; x < image.cols; ++x) {
                auto pixel = image.at<uchar>(y, x);
                if(pixel == 255) {
                    command.emplace_back(G0(x / resolution, std::nullopt, std::nullopt));
                } else {
                    auto power = static_cast<int>((1.0 - static_cast<double>(pixel) / 255.0) * 1000.0);
                    command.emplace_back(G1(x / resolution, std::nullopt, power));
                }
            }
        }
    }

    // 单向扫描优化版本V1
    // One-way scanning optimized version V1
    // 删除多余空行程，这里空行程指连续的无用的G0。
    // Delete excess empty strokes, where empty strokes refer to continuous useless G0.
    void unidirectionOptStrategy() {
        cv::Mat image;
        cv::resize(mat, image, cv::Size(static_cast<int>(width * resolution), static_cast<int>(height * resolution)));
        int offset = 0;  // The frist consecutive G0
        int length = 0;
        for(int y = 0; y < image.rows; ++y) {
            command.emplace_back(G0(offset / resolution, y / resolution, std::nullopt).toString());
            for(int x = 0; x < image.cols; ++x) {
                auto pixel = image.at<uchar>(y, x);
                length     = 0;
                if(pixel == 255) {
                    while(++x < image.cols && image.at<std::uint8_t>(y, x) == 255) {
                        length++;
                    }
                    --x;

                    // Whether continuous GO exists
                    if(length) {
                        if(x - length == 0) {  // skip The frist consecutive G0
                            offset = length;
                            command.emplace_back(G0((x) / resolution, std::nullopt, std::nullopt));
                            continue;
                        }

                        if(x == image.cols - 1) {  // skip The last consecutive G0
                            command.emplace_back(G0((x - length) / resolution, std::nullopt, std::nullopt));
                            continue;
                        }
                        // Continuous GO
                        command.emplace_back(G0(x / resolution, std::nullopt, std::nullopt));
                    } else {
                        // Independent GO
                        command.emplace_back(G0(x / resolution, std::nullopt, std::nullopt));
                    }
                } else {
                    auto power = static_cast<int>((1.0 - static_cast<double>(pixel) / 255.0) * 1000.0);
                    command.emplace_back(G1(x / resolution, std::nullopt, power));
                }
            }
        }
    }

    // 双向扫描
    // Bidirectional scanning
    void bidirectionStrategy() {
        cv::Mat image;
        cv::resize(mat, image, cv::Size(static_cast<int>(width * resolution), static_cast<int>(height * resolution)));

        for(int y = 0; y < image.rows; ++y) {
            bool isEven = !(y & 1);
            int start   = isEven ? 0 : image.cols - 1;
            int end     = isEven ? image.cols : -1;
            int step    = isEven ? 1 : -1;

            command.emplace_back(G0 {std::nullopt, y / resolution, std::nullopt});
            for(int x = start; x != end; x += step) {
                if(auto const pixel = image.at<cv::uint8_t>(y, x); pixel == 255) {
                    command.emplace_back(G0 {x / resolution, std::nullopt, std::nullopt});
                } else {
                    auto power = static_cast<int>((1.0 - static_cast<double>(pixel) / 255.0) * 1000.0);
                    command.emplace_back(G1(x / resolution, std::nullopt, power));
                }
            }
        }
    }

    // 双向扫描优化
    // Bidirectional scanning optimization
    void bidirectionOptStrategy() {
        cv::Mat image;
        cv::resize(mat, image, cv::Size(static_cast<int>(width * resolution), static_cast<int>(height * resolution)));

        bool leftToRight {false};
        bool rightToLeft {false};

        // 可以使用 C++ 迭代器查找距离
        // https://en.cppreference.com/w/cpp/iterator/advance
        // https://en.cppreference.com/w/cpp/iterator/prev

        for(int y = 0; y < image.rows; ++y) {
            bool isEven = !(y & 1);
            int start   = isEven ? 0 : image.cols - 1;
            int end     = isEven ? image.cols : -1;
            int step    = isEven ? 1 : -1;

            for(int x = start; x != end; x += step) {
                if(auto const pixel = image.at<cv::uint8_t>(y, x); pixel == 255) {
                    // 偶数从左到右扫描
                    // 奇数从右到左扫描
                    if(isEven) {
                        // 从左到右寻找连续的G0
                        // |----->
                        int length {0};
                        while(++x < end && image.at<cv::uint8_t>(y, x) == 255) {
                            length++;
                        }
                        --x;

                        // 使用 do{}while(false) 结构，最后统一判断是否会更好
                        // find
                        if(length) {
                            // 起点存在连续G0
                            if(x - length == 0) {
                                // 此时需要把奇数行延迟的Y轴移动进行上移操作
                                if(rightToLeft) {
                                    command.emplace_back(G0((x+1) / resolution, y / resolution, std::nullopt));
                                    rightToLeft = false;
                                } else {
                                    // 偶数从左到右在起点永远不会向上移动，所以这里不需要 y
                                    command.emplace_back(G0 {(x+1) / resolution, std::nullopt, std::nullopt});
                                }
                                continue;
                            }

                            // 终点存在连续G0
                            if(x == image.cols - 1) {
                                // 终点需要向上移动，但这个移动我们放在奇数行处理，所以这里只需要做好标记即可。
                                leftToRight = true;
                                command.emplace_back(G0(((x+1) - length) / resolution, std::nullopt, std::nullopt));
                                continue;
                            }

                            // 中间段存在连续从左到右方向的G0
                            // 中间段不需要向上移动
                            command.emplace_back(G0((x+1) / resolution, std::nullopt, std::nullopt));
                        } else {
                            // 没有找到连续的G0
                            // 终点唯一的G0,需要向上移动，这里做标记放到奇数行移动。
                            if(x == image.cols - 1) {
                                leftToRight = true;
                            } else if(x == start) {
                                command.emplace_back(G0((x+1) / resolution, y / resolution, std::nullopt));
                                rightToLeft = false;
                                continue;
                            }
                            command.emplace_back(G0((x+1) / resolution, std::nullopt, std::nullopt));
                        }
                    } else {
                        // <-----|
                        // 从右到左寻找连续的G0 此时起点在右边，终点在左边
                        int length {0};
                        while(--x > end && image.at<cv::uint8_t>(y, x) == 255) {
                            length++;
                        }
                        ++x;

                        if(length) {
                            // 起点存在连续的G0
                            if(x + length == start) {
                                // 此时需要把偶数行延迟的Y轴移动进行上移操作
                                if(leftToRight) {
                                    command.emplace_back(G0(x / resolution, y / resolution, std::nullopt));
                                    leftToRight = false;
                                } else {
                                    // 标记
                                    command.emplace_back(G0(x / resolution, std::nullopt, std::nullopt));
                                }
                                continue;
                            }

                            // 终点存在连续的G0
                            if(x == 0) {
                                rightToLeft = true;
                                ;
                                continue;
                            }
                            command.emplace_back(G0(x / resolution, std::nullopt, std::nullopt));
                        } else {
                            // 没有找到连续的G0
                            // 终点需要向上移动
                            if(x == 0) {
                                rightToLeft = true;
                            } else if(x == start) {
                                // 起点也需要处理上一行的y轴移动
                                if(leftToRight) {
                                    command.emplace_back(G0(x / resolution, y / resolution, std::nullopt));
                                    leftToRight = false;
                                }
                                continue;
                            }
                            command.emplace_back(G0(x / resolution, std::nullopt, std::nullopt));
                        }
                    }
                } else {
                    auto power = static_cast<int>((1.0 - static_cast<double>(pixel) / 255.0) * 1000.0);
                    // 处理G1 开头和结尾情况
                    if(isEven) {
                        // 从左到右
                        if(x == start) {
                            if(rightToLeft) {
                                command.emplace_back(G0 {(x+1) / resolution, y / resolution, power});  // 最大激光功率 S=1000
                                rightToLeft = false;
                                continue;
                            }
                        } else if(x == image.cols - 1) {
                            // 终点需要标记
                            leftToRight = true;
                        }
                        command.emplace_back(G1 {(x+1) / resolution, std::nullopt, power});  // 最大激光功率 S=1000
                    } else {
                        // 从右到左
                        if(x == start) {
                            if(leftToRight) {
                                command.emplace_back(G0 {x / resolution, y / resolution, power});  // 最大激光功率 S=1000
                                leftToRight = false;
                                continue;
                            }
                        } else if(x == 0) {
                            // 终点需要标记
                            rightToLeft = true;
                        }
                        command.emplace_back(G1 {x / resolution, std::nullopt, power});  // 最大激光功率 S=1000
                    }
                }  // end if G0
            }      // end for x
        }          // end for y
    }

    // 双向扫描使用C++标准库优化
    // Bidirectional scanning uses C++ standard library optimization
    void bidirectionStdOptStrategy() {
        cv::Mat image;
        cv::resize(mat, image, cv::Size(static_cast<int>(width * resolution), static_cast<int>(height * resolution)));

        struct G {
            int x, y, v;  // <X>.<Y>.<Pixel>
        };

        std::vector<std::vector<G>> lines;
        for(int y = 0; y < image.rows; y++) {
            std::vector<G> line;
            for(int x = 0; x < image.cols; x++) {
                auto pixel = image.at<uchar>(y, x);
                line.emplace_back(x, y, pixel);
            }
            lines.emplace_back(line);
        }

        // process lines vector
        // 数据预处理
        for(bool isEven = true; auto &line: lines) {
            if(!isEven) {
                // 奇数调转方向
                std::ranges::reverse(line.begin(), line.end());
            }
            isEven = !isEven;
        }

        // 经过上述处理，数据的顺序现在一致了。
        // 现在只需按顺序处理，删除从左到右连续的G0，仅保留连续G0最右边的元素。

        // 使用 std::ranges::join 合并嵌套的向量
        // std::back_inserter

        // auto container = std::ranges::join_view(lines);

        std::vector<G> container;
        for(auto &line: lines) {
            container.insert(container.end(), line.begin(), line.end());
        }

        while(true) {
            auto it = std::ranges::adjacent_find(container.begin(), container.end(), [](auto left, auto right) -> bool { return left.v == 255 && left.v == right.v; });
            if(it == container.end()) {
                break;
            }
            container.erase(it);
        }

        for(auto g: container) {
            if(g.v == 255) {
                command.emplace_back(G0 {g.x, g.y, std::nullopt});
            } else {
                command.emplace_back(G1 {g.x, g.y, 1000});
            }
        }
    }

    void internal(cv::Mat &image, auto x /*width*/, auto y /*height*/,bool isEven) {
        auto pixel = image.at<cv::uint8_t>(y, x);
        if(pixel == 255) {
            command.emplace_back(G0(isEven ? (x + 1) / resolution : x / resolution, y / resolution, std::nullopt));
        } else {
            auto power = static_cast<int>((1.0 - static_cast<double>(pixel) / 255.0) * 1000.0);
            command.emplace_back(G1(isEven ? (x + 1) / resolution : x / resolution, y / resolution, power));
        }
    }

    // 双向斜向扫描
    // Bidirectional oblique scanning
    // 优化的方式同 bidirectionStdOptStrategy 函数相似
    // The optimization method is similar to the bidirectionStdOptStrategy function
    void diagonalStrategy() {
        cv::Mat image;
        cv::resize(mat, image, cv::Size(static_cast<int>(width * resolution), static_cast<int>(height * resolution)));
        for(int k /*diagonal*/ = 0; k < image.rows + image.cols - 1 /*cond = height + width - 1*/; ++k) {
            if((k & 1) == 0) {
                // even
                for(int i = std::min(k, image.rows - 1); i >= 0; --i) {
                    int j = k - i;
                    if(i < image.rows && j < image.cols) {
                        internal(image, j, i,true);
                    }
                }
            } else {
                // odd
                for(int j = std::min(k, image.cols - 1); j >= 0; --j) {
                    int i = k - j;
                    if(i < image.rows && j < image.cols) {
                        internal(image, j, i,false);
                    }
                }
            }
        }
    }

    // 螺旋扫描 从外到里的方向
    // Spiral scan from outside to inside direction
    void spiralStrategy() {
        cv::Mat image;
        cv::resize(mat, image, cv::Size(static_cast<int>(width * resolution), static_cast<int>(height * resolution)));

        int top = 0, bottom = image.rows - 1, left = 0, right = image.cols - 1;
        while(top <= bottom && left <= right) {
            for(int i = left; i <= right; ++i) {
                internal(image, i, top);
            }
            ++top;

            for(int i = top; i <= bottom; ++i) {
                internal(image, right, i);
            }
            --right;

            if(top <= bottom) {
                for(int i = right; i >= left; --i) {
                    internal(image, i, bottom);
                }
                --bottom;
            }

            if(left <= right) {
                for(int i = bottom; i >= top; --i) {
                    internal(image, left, i);
                }
                ++left;
            }
        }
    }

    void strategySample() {
        auto &image = mat;

        // scaling
        auto sx = width / (image.cols * resolution);
        auto sy = height / (image.rows * resolution);

        for(std::size_t y = 0; y < image.rows; y++) {
            command.emplace_back(G0(0, y * sy, std::nullopt));
            for(int x = 0; x < image.cols; x++) {
                if(auto pixel = image.at<uchar>(y, x); pixel == 255) {
                    // G0 移动指令，不包含功率参数
                    command.emplace_back(G0(x * sx, y * sy, std::nullopt));
                } else {
                    // 像素不为 255，使用 G1 移动指令，包含功率参数
                    auto power = static_cast<int>((pixel / 255.0) * 1000.0);
                    command.emplace_back(G1(x * sx, y * sy, power));
                }
            }
        }
    }

    // Define additional strategy functions here

private:
    cv::Mat mat;                                 // 灰度图像
    double width {0};                            // 工作范围 x 轴
    double height {0};                           // 工作范围 y 轴
    double resolution {0};                       // 精度 lin/mm
    ScanMode scanMode {ScanMode::Bidirection};   // 默认双向
    LaserMode laserMode {LaserMode::Engraving};  // 默认雕刻模式
    std::optional<int> airPump;                  // 自定义指令 气泵 用于吹走加工产生的灰尘 范围 [0,1000]
    // add more custom cmd
    std::vector<std::string> command;  // G 代码
};
