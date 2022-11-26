#pragma once
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <string> 

using namespace cv;
using namespace std;


namespace grd {

    const Scalar BEP_SCALAR_GREEN = Scalar(0, 255, 0, 255);
    const Scalar BEP_SCALAR_BLUE = Scalar(255, 0, 0, 255);
    const Scalar BEP_SCALAR_RED = Scalar(0, 0, 255, 255);
    const Scalar BEP_SCALAR_YELLOW = Scalar(0, 255, 255, 255);

    enum Bep_DetectType {
        Top2Bottom_Left2Right,
        Left2Right_Top2Bottom
    };

    struct Bep_Left2Right_ContourSorter
    {
        bool operator ()(const vector<Point>& a, const vector<Point>& b)
        {
            Rect ra(boundingRect(a));
            Rect rb(boundingRect(b));
            return ra.x < rb.x;
        }
    };
    struct Bep_Top2Bottom_ContourSorter
    {
        bool operator ()(const vector<Point>& a, const vector<Point>& b)
        {
            Rect ra(boundingRect(a));
            Rect rb(boundingRect(b));
            return ra.y < rb.y;
        }
    };
    struct Bep_AreaDesc_ContourSorter
    {
        bool operator ()(const vector<Point>& a, const vector<Point>& b)
        {
            Rect ra(boundingRect(a));
            Rect rb(boundingRect(b));
            return ra.area() > rb.area();
        }
    };
    /*----------------------------------*/

    void bep_Resize(Mat& img, Mat& imgReSize) {
        resize(img, imgReSize, Size(), 1, 1);
    }

    void bep_PerspectiveTransform(vector<vector<Point>>& inputMat, vector<vector<Point>>& dst, Mat& matrix) {
        for (int i = 0; i < inputMat.size(); i++) {
            vector<Point2f> contour0;
            Mat(inputMat[i]).convertTo(contour0, Mat(contour0).type());
            vector<Point2f> contour1;
            perspectiveTransform(contour0, contour1, matrix);
            vector<Point> contour2;
            Mat(contour1).convertTo(contour2, Mat(contour2).type());
            dst[i] = contour2;
        }
    }

    void bep_DrawContours(Mat& src, vector<vector<Point>>& contours, const Scalar& color, bool hasLabel) {
        for (size_t i = 0; i < contours.size(); i++) {
            Rect r = boundingRect(contours[i]);
            //
            Point point4 = Point((r.br() + r.tl()) * .5);
            //circle(src, point4, 1, 255, 3);
            rectangle(src, r, color, 2, 8, 0);
            String text = to_string(i + 1);
            if (hasLabel) {
                putText(src, text, point4, FONT_HERSHEY_SIMPLEX, 0.5, BEP_SCALAR_BLUE, 2);
            }
        }
    }
    void bep_DrawContours(Mat& src, vector<vector<Point>>& contours, bool hasLabel) {
        bep_DrawContours(src, contours, BEP_SCALAR_GREEN, hasLabel);
    }
    void bep_DrawContours(Mat& src, vector<vector<Point>>& contours) {
        bep_DrawContours(src, contours, BEP_SCALAR_GREEN, false);
    }

    void bep_DrawCircleOfContour(Mat& src, vector<Point>& contour, const Scalar& color, int thickness = 1) {
        Rect r = boundingRect(contour);
        //
        Point point4 = Point((r.br() + r.tl()) * .5);
        circle(src, point4, (r.width + r.height) / 4, color, thickness, 0);
    }

    void bep_DrawPointContours(Mat& src, vector<vector<Point>>& contours, const Scalar& color, bool hasLabel) {
        int count = 0;
        for (size_t i = 0; i < contours.size(); i++) {
            count++;
            bep_DrawCircleOfContour(src, contours[i], color);
            String text = to_string(count);
            if (hasLabel) {
                putText(src, text, contours[i][0], FONT_HERSHEY_SIMPLEX, 0.5, BEP_SCALAR_BLUE, 2);
            }
        }
    }

    void bep_DetectCircleMatrix(Mat& identityMat, vector<int>& identityNum, int roundLine, Bep_DetectType type) {
        function<int(vector<Point>&, vector<Point>&)> firstPred, secondPred;
        if (type == Top2Bottom_Left2Right) {
            firstPred = Bep_Left2Right_ContourSorter();
            secondPred = Bep_Top2Bottom_ContourSorter();
        }
        else if (Left2Right_Top2Bottom) {
            firstPred = Bep_Top2Bottom_ContourSorter();
            secondPred = Bep_Left2Right_ContourSorter();
        }
        Mat identityMatTempThesh;
        cvtColor(identityMat, identityMatTempThesh, COLOR_BGR2GRAY);
        threshold(identityMatTempThesh, identityMatTempThesh, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
        //
        vector<vector<Point>> codeContours;
        vector<vector<Point>> codeContoursToSort;
        findContours(identityMatTempThesh, codeContours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        //imshow("Image3.5", identityMatTempThesh);
        for (size_t i = 0; i < codeContours.size(); i++) {
            Rect r = boundingRect(codeContours[i]);
            bool flagRect = r.width / (double)r.height < 1.5 && r.width / (double)r.height > 0.75 && r.area() < 1000 && r.area() > 50;
            if (flagRect) {
                codeContoursToSort.push_back(codeContours[i]);
            }
        }
        sort(codeContoursToSort.begin(), codeContoursToSort.end(), firstPred);
        for (size_t i = 0; i < codeContoursToSort.size(); i = i + roundLine) {
            int tempVal = -1;
            sort(codeContoursToSort.begin() + i, codeContoursToSort.begin() + i + roundLine, secondPred);
            for (size_t j = i; j < i + roundLine; j++) {
                Rect boundCodeContours = boundingRect(codeContoursToSort[j]);
                Mat mask = Mat(boundCodeContours.height, boundCodeContours.width, uchar(0));
                fillPoly(mask, codeContoursToSort, Scalar(255), 8, 0, -boundCodeContours.tl());
                Mat cropped = identityMatTempThesh(boundCodeContours) & mask;
                int countNonZe = countNonZero(cropped);
                if (countNonZe > (boundCodeContours.width * boundCodeContours.height) / 2) {
                    tempVal = j - i;
                    bep_DrawCircleOfContour(identityMat, codeContoursToSort[j], BEP_SCALAR_GREEN, 2);
                    break;
                }
            }
            /*if (tempVal < 0) {
                bepDrawCircleOfContour(identityMat, codeContoursToSort[i], BEP_SCALAR_RED, 2);
            }*/
            identityNum.push_back(tempVal);
        }
    }

    void bep_Sort_Top2Bottom_Lef2Right(vector<vector<Point>>& input, int roundLine) {
        sort(input.begin(), input.end(), Bep_Left2Right_ContourSorter());
        for (size_t i = 0; i < input.size(); i = i + roundLine) {
            sort(input.begin() + i, input.begin() + i + roundLine, Bep_Top2Bottom_ContourSorter());
        }
    }
    void bep_Sort_Lef2Right_Top2Bottom(vector<vector<Point>>& input, int roundLine) {
        sort(input.begin(), input.end(), Bep_Top2Bottom_ContourSorter());
        for (size_t i = 0; i < input.size(); i = i + roundLine) {
            sort(input.begin() + i, input.begin() + i + roundLine, Bep_Left2Right_ContourSorter());
        }
    }

    void bep_ImResizeAndShow(string& title, Mat& img, int w, int h) {
        Mat imgReSize;
        resize(img, imgReSize, Size(w, h));
        imshow(title, imgReSize);
    }

    int bep_RandomIntInRange(int start, int end) {
        return rand() % end + start;
    }

    double bep_Round(double num, int afterPoint = 0) {
        int deter = 1;
        while (afterPoint > 0) {
            deter += 10;
            afterPoint--;
        }
        return round(num * deter) / deter;
    }
}