#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

#include "config.hpp"
#include "macros.hpp"

using namespace cv;
using namespace std;

// #define USE_CAMERA

#ifdef USE_CAMERA
constexpr int WAITKEY_DELAY = 16;
#else
constexpr int WAITKEY_DELAY = 0;
#endif

int main(int argc, char **argv)
{
    namedWindow("params");
    ALGORITHM_PARAM(hlow, 54, 255);
    ALGORITHM_PARAM(hhigh, 77, 255);
    ALGORITHM_PARAM(slow, 137, 255);
    ALGORITHM_PARAM(shigh, 255, 255);
    ALGORITHM_PARAM(vlow, 0, 255);
    ALGORITHM_PARAM(vhigh, 89, 255);

#ifdef USE_CAMERA
    VideoCapture cam(0);
#else
    cv::String path("../c2018/vision/test-img/*.jpg"); //select only jpg
    vector<cv::String> fn;
    vector<cv::Mat> data;
    cv::glob(path, fn, true);
    size_t k = 0;
    if (fn.empty())
    {
        return 0;
    }
#endif
    Mat raw;
    Mat resized;
    Mat undistorted;
    Mat blurred;
    Mat hsv;
    Mat mask;
    // auto contours;

    for (;;)
    {
#ifdef USE_CAMERA
        cam >> raw;
#else
        raw = cv::imread(fn[min(k, fn.size() - 1)]);
#endif
        SHOW("raw", raw);

        resize(raw, resized, Size(320, 240));
        SHOW("resized", resized);

        // TODO undistort
        resized.copyTo(undistorted);
        SHOW("undistorted", undistorted)

        GaussianBlur(undistorted, blurred, Size(3, 3), 0, 0);
        SHOW("blurred", blurred);

        cvtColor(blurred, hsv, CV_RGB2HSV);
        SHOW("hsv", hsv);

        inRange(hsv, Scalar(hlow, slow, vlow), Scalar(hhigh, shigh, vhigh), mask);
        SHOW("mask", mask);

        switch (waitKey(WAITKEY_DELAY))
        {
        case 27:
            return 0;
#ifndef USE_CAMERA
        case 'n':
            k++;
            break;
        case 'b':
            k--;
            break;
#endif
        }
    };
    return 0;
}
