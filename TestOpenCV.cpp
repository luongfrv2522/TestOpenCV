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
    
    //
    float w = 768, h = 1024;
    float wShow = 576, hShow = 768;
    Mat img = imread("Resources/50_cau.jpg");
    //
    float hCodeVsAnswer = h/3;
    Mat imgReSize, imgGray, imgThreshHold, imgDilate, outMat;
    //
    resize(img, img, Size(w, h));

    #pragma region show Image2.1
    /*resize(img, imgReSize, Size(wShow, hShow));
    imshow("Image1.1", imgReSize);*/
    #pragma endregion

    cvtColor(img, imgGray, COLOR_BGR2GRAY);
    adaptiveThreshold(imgGray, imgThreshHold, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 45, 5);
    #pragma region show Image2.1
    /*resize(imgThreshHold, imgReSize, Size(wShow, hShow));
    imshow("Image2.1", imgReSize);*/
    #pragma endregion
    //Giãn nền
    Mat elementDilate = getStructuringElement(MORPH_CROSS, Size(3, 3), Point(1, 1));
    dilate(imgThreshHold, imgDilate, elementDilate, Point(-1, -1), 3);
    #pragma region show Image2.1
    /*resize(imgDilate, imgReSize, Size(wShow, hShow));
    imshow("Image3.1", imgReSize);*/
    #pragma endregion
    Mat elementErode = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
    erode(imgDilate, imgDilate, elementErode, Point(-1, -1), 3);
    #pragma region show Image2.1
    /*resize(imgDilate, imgReSize, Size(wShow, hShow));
    imshow("Image3.2", imgReSize);*/
    #pragma endregion

    //Tìm viền các hình tô đậm
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
    #pragma region Find contours
    vector<vector<Point>> contours4BasePoint;
    vector<vector<Point>> contoursSubBasePoint;
    if (contoursToSort.size() >= 14) {
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
        #pragma region Chia phần câu hỏi và mã đề
        vector<vector<Point>> contoursSubCodeBasePoint;
        vector<vector<Point>> contoursSubAnswerBasePoint;
        for (size_t i = 0; i < contoursSubBasePoint.size(); i++) {
			if (contoursSubBasePoint[i][0].y < hCodeVsAnswer) {
                contoursSubCodeBasePoint.push_back(contoursSubBasePoint[i]);
            }
            else {
                contoursSubAnswerBasePoint.push_back(contoursSubBasePoint[i]);
            }
        }
        myDrawContours(outMat, contoursSubCodeBasePoint, BEP_SCALAR_YELLOW, false);
        myDrawContours(outMat, contoursSubAnswerBasePoint, BEP_SCALAR_GREEN, false);
        #pragma endregion

        #pragma region Tìm hình chữ nhật cơ sở của số báo danh và mã đề
        vector<Rect> subCodeBase4Point;
        subCodeBase4Point.push_back(boundingRect(contoursSubCodeBasePoint[0]));
        subCodeBase4Point.push_back(boundingRect(contoursSubCodeBasePoint[1]));
        subCodeBase4Point.push_back(boundingRect(contoursSubCodeBasePoint[contoursSubCodeBasePoint.size()-2]));
        subCodeBase4Point.push_back(boundingRect(contoursSubCodeBasePoint[contoursSubCodeBasePoint.size()-1]));
        //
        Mat identityMat = outMat(Rect(
            subCodeBase4Point[0].br().x,
            subCodeBase4Point[0].br().y,
            subCodeBase4Point[0].width * 9,
            subCodeBase4Point[2].tl().y - subCodeBase4Point[0].br().y
        ));
        /*Mat codeMat = outMat(Rect(
            subCodeBase4Point[1].br().x,
            subCodeBase4Point[1].br().y,
            subCodeBase4Point[1].width * 5,
            subCodeBase4Point[3].br().y - subCodeBase4Point[1].br().y
        ));*/

        #pragma region show Image3.5
        /*for (int i = 0; i < 10; i++) {
            line(identityMat,
                Point(0, identityMat.size().height / 10 * (i+1)),
                Point(identityMat.size().height, identityMat.size().height / 10 * (i + 1)),
                BEP_SCALAR_BLUE,
                1,
                LINE_8
            );
        }*/
        //Tìm viền các hình tô đậm
        Mat identityMatTempThesh;
        cvtColor(identityMat, identityMatTempThesh, COLOR_BGR2GRAY);
        threshold(identityMatTempThesh, identityMatTempThesh, 0, 255,THRESH_BINARY_INV | THRESH_OTSU);
        //
        vector<vector<Point>> codeContours;
        vector<vector<Point>> codeContoursToSort;
        findContours(identityMatTempThesh, codeContours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        imshow("Image3.5", identityMatTempThesh);
        for (size_t i = 0; i < codeContours.size(); i++) {
            Rect r = boundingRect(codeContours[i]);
            bool flagRect = r.width / (double)r.height < 1.5 && r.width / (double)r.height > 0.75 && r.area() < 1000 && r.area() > 50;
            if (flagRect) {
                codeContoursToSort.push_back(codeContours[i]);
            }
        }
        sort(codeContoursToSort.begin(), codeContoursToSort.end(), Left_Right_contour_sorter());
        int roundLine = 10;
        String identityNum;
        for (size_t i = 0; i < codeContoursToSort.size(); i = i + roundLine) {
            int tempVal = -1;
            int indexIdentityNum = 0;
            sort(codeContoursToSort.begin() + i, codeContoursToSort.begin() + i + roundLine, Top_Bottom_contour_sorter());
            for (size_t j = i; j < i + roundLine; j++) {
                Rect boundCodeContours = boundingRect(codeContoursToSort[j]);
                Mat mask = Mat(boundCodeContours.height, boundCodeContours.width, uchar(0));
                fillPoly(mask, codeContoursToSort, Scalar(255), 8, 0, -boundCodeContours.tl());
                Mat cropped = identityMatTempThesh(boundCodeContours) & mask;
                int countNonZe = countNonZero(cropped);
                if (countNonZe > (boundCodeContours.width * boundCodeContours.height) / 2) {
                    tempVal = indexIdentityNum;
                }
                indexIdentityNum++;
            }
            identityNum = identityNum +"|" + to_string(tempVal);
        }

        //myDrawContours(identityMat, codeContoursToSort);

        putText(outMat, identityNum, subCodeBase4Point[0].br(), FONT_HERSHEY_SIMPLEX, 0.5, BEP_SCALAR_BLUE, 2);
        imshow("Image3.6", identityMat);
        //imshow("Image3.6", codeMat);
        #pragma endregion show Image3.5
        #pragma endregion
    }
    else if (contoursToSort.size() < 4) {
        cout << "---------------------------------" << endl;
        cout << "WARNING NO FOUND 4Point " << contours.size() << endl;
    }
    #pragma endregion

    #pragma region show Image2.1
    line(outMat, Point(0, hCodeVsAnswer), Point(w, hCodeVsAnswer), BEP_SCALAR_BLUE, 5, LINE_8);
    resize(outMat, imgReSize, Size(wShow, hShow));
    imshow("Image4", imgReSize);
    #pragma endregion
    waitKey(0);
}