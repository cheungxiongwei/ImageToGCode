#include <print>
#include <string>
#include <vector>
#include "ImageToGCode.h"

int main() {
    // 写入你自己的路径
    cv::Mat mat = cv::imread(R"(\ImageToGCode\image\tigger.jpg)", cv::IMREAD_GRAYSCALE);
    cv::flip(mat, mat, 0);

    cv::threshold(mat, mat, 128, 255, cv::ThresholdTypes::THRESH_BINARY);

    //cv::imshow("mat",mat);
    //cv::waitKey(0);

    ImageToGCode ins;
    // 50x50 mm 1.0 line/mm
    ins.setInputImage(mat).setOutputTragetSize(50, 50, 10).builder().exportGCode(R"(\ImageToGCode\output\tigger.nc)");
}
