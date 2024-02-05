# ImageToGCode
本文从浅到深详细阐述了从简单的二进制转GCode到灰度图像转GCode。

### 如何阅读？
本文使用 rust mdbook 编写，如何查看该文档？

请先安装 rust，然后运行 `cargo install mdbook` 命令安装 `mdbook` 工具。

最后使用以下命令打开文档。
```
mdbook serve --open
```

系列文章：
1. 基于二值化图像转GCode的单向扫描实现
2. 基于二值化图像转GCode的双向扫描实现
3. 基于二值化图像转GCode的斜向扫描实现
4. 基于二值化图像转GCode的螺旋扫描实现
5. 基于OpenCV灰度图像转GCode的单向扫描实现
6. 基于OpenCV灰度图像转GCode的双向扫描实现
7. 基于OpenCV灰度图像转GCode的斜向扫描实现
8. 基于OpenCV灰度图像转GCode的螺旋扫描实现

### 如何编译和运行？
```
git clone https://github.com/cheungxiongwei/ImageToCode
cd ImageToCode/src
clion .
```

如果需要运行 ImageToGCode，需要配置 OpenCV
```
list(APPEND CMAKE_PREFIX_PATH "~/opencv/build/x64/vc16/lib")
```
编辑 CMakeLists.txt 文件，把 `~/opencv/build/x64/vc16/lib` 替换成你自己的 OpenCV 路径。

