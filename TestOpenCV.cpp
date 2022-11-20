#include "Header.h"
#include "TestFindRect.h"
#include "TestFindCircle.h"
#include "TestFindCircle2.h"
#include "AutomaticGrading.h"

int main()
{
    //TestFindRect::Run();

    //TestFindCircle::Run();

    //TestFindCircle2::Run();

    float w = 768;
    float h = 1024;
    Mat img = imread("Resources/50_cau.jpg");
    Mat imgReSize, imgGray, imgThreshHold, imgDilate, outMat;
    //
    resize(img, img, Size(w, h));

    cvtColor(img, imgGray, COLOR_BGR2GRAY);
    adaptiveThreshold(imgGray, imgThreshHold, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 45, 5);
    //imshow("Image2.1", imgThreshHold);
    //Giãn nền
    Mat elementDilate = getStructuringElement(MORPH_CROSS, Size(3, 3), Point(1, 1));
    dilate(imgThreshHold, imgDilate, elementDilate, Point(-1, -1), 3);
    //imshow("Image3.1", imgDilate);
    Mat elementErode = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
    erode(imgDilate, imgDilate, elementErode, Point(-1, -1), 3);
    //imshow("Image3.2", imgDilate);

    //Tìm viền các hình chữ nhật neo góc
    vector<vector<Point>> contours;
    findContours(imgDilate, contours, RETR_TREE, CHAIN_APPROX_NONE);

    cout << "countours.size: " << contours.size() << endl;
    int count = 0;
    vector<vector<Point>> contoursToSort;
    for (size_t i = 0; i < contours.size(); i++) {
        Rect r = boundingRect(contours[i]);
        bool flagRect = r.width / (double)r.height < 1.5 && r.width / (double)r.height > 0.75 && r.area() < 1000 && r.area() > 100;
        if (flagRect) {
            contoursToSort.push_back(contours[i]);
        }
    }
    //drawToContour(img, contoursToSort);
    //imshow("Image3.3", img);

    if (contoursToSort.size() >= 14) {
        vector<vector<Point>> contours4BasePoint;
        vector<vector<Point>> contoursSubBasePoint;
        sort(contoursToSort.begin(), contoursToSort.end(), area_desc_contour_sorter());
        for (size_t i = 0; i < contoursToSort.size(); i++) {
            if (i < 4) {
                contours4BasePoint.push_back(contoursToSort[i]);
            }
            else {
                contoursSubBasePoint.push_back(contoursToSort[i]);
            }
        }

        sort(contours4BasePoint.begin(), contours4BasePoint.end(), Top_Bottom_contour_sorter());
        sort(contours4BasePoint.begin(), contours4BasePoint.begin()+2, Left_Right_contour_sorter());
        sort(contours4BasePoint.begin()+2, contours4BasePoint.end(), Left_Right_contour_sorter());
        
        
        Rect r1 = boundingRect(contours4BasePoint[0]);
        Rect r2 = boundingRect(contours4BasePoint[1]);
        Rect r3 = boundingRect(contours4BasePoint[2]);
        Rect r4 = boundingRect(contours4BasePoint[3]);

        Point2f src[4] = {
            r1.tl(),
            {(float)(r2.x + r2.width), (float)r2.y },
            {(float)(r3.x), (float)(r3.y + r3.height)},
            r4.br()
        };
        Point2f dst[4] = {
            {0.0f, 0.0f},
            {w, 0.0f},
            {0.0f, h},
            {w, h}
        };

        Mat matrix;
        matrix = getPerspectiveTransform(src, dst);
        warpPerspective(img, outMat, matrix, Point(w, h));

        persTrans(contours4BasePoint, contours4BasePoint, matrix);
        persTrans(contoursSubBasePoint, contoursSubBasePoint, matrix);

        //drawToContour(outMat, contours4BasePoint);

        sort(contoursSubBasePoint.begin(), contoursSubBasePoint.end(), Top_Bottom_contour_sorter());
        myDrawContours(outMat, contoursSubBasePoint, true);
    }
    else if (contoursToSort.size() < 4) {
        cout << "---------------------------------" << endl;
        cout << "WARNING NO FOUND 4Point " << contours.size() << endl;
    }

    imshow("Image4", outMat);
    waitKey(0);
}