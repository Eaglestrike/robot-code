#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <utility>

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
    ALGORITHM_PARAM(minTargetRectArea, 100, 500);
    ALGORITHM_PARAM(minTargetFullness, 500, 1000);

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
    vector<vector<Point>> contours;
    vector<Point> convex_cnt;
    vector<Point> poly;
    vector<RotatedRect> targets;
    vector<pair<RotatedRect, RotatedRect>> matched;
    pair<RotatedRect, RotatedRect> selected;

    for (;;)
    {
#ifdef USE_CAMERA
        cam >> raw;
#else
        raw = cv::imread(fn[((k % fn.size()) + fn.size()) % fn.size()]);
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

        cout << "test" << endl;
        contours.clear();
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_TC89_KCOS);

        drawContours(resized, contours, -1, Scalar(255, 255, 0));
        SHOW("contours", resized);

        targets.clear();
        for (auto &cnt : contours)
        {
            // determine if contour is a valid target
            convex_cnt.clear();
            convexHull(cnt, convex_cnt);
            RotatedRect rect = minAreaRect(convex_cnt);

#ifdef DEBUG
            Point2f rect_points[4];
            rect.points(rect_points);
            for (int j = 0; j < 4; j++)
                line(resized, rect_points[j], rect_points[(j + 1) % 4], Scalar(255, 0, 255), 1, 8);
#endif

            if (rect.size.area() < minTargetRectArea)
            {
                cout << "area too small" << endl;
                continue;
            }
            float area = static_cast<float>(contourArea(convex_cnt));
            if (area / rect.size.area() < static_cast<float>(minTargetFullness) / 1000)
            {
                cout << "fullness too low" << endl;
                continue;
            }

            targets.push_back(rect);
            // store target info
        }

        // preprocess rectangles for system solving
        for (RotatedRect &rect : targets)
        {
#ifdef DEBUG
            Point2f rect_points[4];
            rect.points(rect_points);
            for (int j = 0; j < 4; j++)
                line(resized, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 0, 255), 1, 8);
#endif
            // remember, opencv coordinate system has a flipped y, angles are still from +x towards +y
            // ensures the angle is to the positive side of the rect
            if (rect.size.width < rect.size.height)
            {
                rect.angle -= 90;
            }
            else
            {
                // means height < width
                // ensure height > width
                auto temp = rect.size.height;
                rect.size.height = rect.size.width;
                rect.size.width = temp;
            }
#ifdef DEBUG
            putText(resized, to_string(rect.size.width > rect.size.height), rect.center, CV_FONT_HERSHEY_PLAIN, 1.0, Scalar(0, 255, 255));
            putText(resized, to_string(static_cast<int>(rect.center.x)), rect.center, CV_FONT_HERSHEY_PLAIN, 1.0, Scalar(0, 0, 255));
#endif
        }
        SHOW("boxes", resized);
        cout << "tlen" << targets.size() << endl;

        // iterate over all pairs
        matched.clear();
        for (auto a = targets.begin(); a != targets.end(); ++a)
        {
            auto b = a;
            for (++b; b != targets.end(); ++b)
            {
                // solve the system of two vertical-ish lines through the rectangle center
                // A + a*t = B + b*s
                float ax = cos(a->angle * CV_PI / 180.0);
                float ay = sin(a->angle * CV_PI / 180.0);
                float bx = cos(b->angle * CV_PI / 180.0);
                float by = sin(b->angle * CV_PI / 180.0);

#ifdef DEBUG
                line(resized, a->center, Point(a->center.x + 300 * ax, a->center.y + 300 * ay), Scalar(0, 255, 0));
                line(resized, b->center, Point(b->center.x + 300 * bx, b->center.y + 300 * by), Scalar(0, 255, 0));
#endif

                float t = -(a->center.x * by - a->center.y * bx - b->center.x * by + b->center.y * bx) / (ax * by - ay * bx);
                cout << "t " << t << endl;
                float centerX = a->center.x + ax * t;
                // if the solution lies between the two centers, they form a match
                cout << "cx " << centerX << endl;
                float maxX = max(a->center.x, b->center.x);
                float minX = min(a->center.x, b->center.x);
                if (minX < centerX && centerX < maxX && !isnan(centerX))
                {
#ifdef DEBUG
                    line(resized, Point(centerX, 0), Point(centerX, 200), Scalar(0, 255, 0));
#endif
                    if (a->center.x < centerX)
                    { // a is on the left
                        matched.push_back(make_pair(*a, *b));
                    }
                    else
                    { // a is on the right
                        matched.push_back(make_pair(*b, *a));
                    }
                }
            }
        }
        SHOW("targeted", resized);
        // push all the targets out in a message
        for (auto &pair : matched)
        {
            // find the bottom inside point of each rotated rect
            auto &lr = pair.first;
            double _angle = lr.angle * CV_PI / 180.;
            float b = (float)sin(_angle) * -0.5f;
            float a = (float)cos(_angle) * 0.5f;
            Point left;
            left.x = lr.center.x - a * lr.size.height + b * lr.size.width;
            left.y = lr.center.y + b * lr.size.height + a * lr.size.width;

            auto &rr = pair.second;
            _angle = rr.angle * CV_PI / 180.;
            b = (float)sin(_angle) * -0.5f;
            a = (float)cos(_angle) * 0.5f;
            Point right;
            right.x = rr.center.x - a * rr.size.height - b * rr.size.width;
            right.y = rr.center.y + b * rr.size.height - a * rr.size.width;

            // take the mean of the points
            Point mean;
            mean.x = (left.x + right.x) / 2;
            mean.y = (left.y + right.y) / 2;
#ifdef DEBUG
            circle(resized, mean, 2, Scalar(255, 255, 0), -1);
#endif

            // TODO send it out
        }
        SHOW("spoints", resized);

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
        case 's':
            stringstream fname;
            fname << "../c2018/vision/test-img/out/" << k << ".jpg";
            imwrite(fname.str(), resized);
            break;
#endif
        }
    };
    return 0;
}
