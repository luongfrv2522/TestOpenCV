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
        r1 = boundingRect(contours4BasePoint[0]);
        r2 = boundingRect(contours4BasePoint[1]);
        r3 = boundingRect(contours4BasePoint[2]);
        r4 = boundingRect(contours4BasePoint[3]);

        //drawToContour(outMat, contours4BasePoint);

        #pragma region Chia phần câu hỏi và mã đề
        sort(contoursSubBasePoint.begin(), contoursSubBasePoint.end(), Top_Bottom_contour_sorter());
        vector<vector<Point>> contoursSubCodeBasePoint;
        vector<vector<Point>> contoursSubAnswerBasePoint;
        for (size_t i = 0; i < contoursSubBasePoint.size(); i++) {
            Rect boundIt = boundingRect(contoursSubBasePoint[i]);
            if (boundIt.y < r1.y || boundIt.y > r4.br().y || boundIt.x < r1.x || boundIt.x > r4.br().x) continue;
			if (boundIt.y < hCodeVsAnswer) {
                contoursSubCodeBasePoint.push_back(contoursSubBasePoint[i]);
            }
            else {
                contoursSubAnswerBasePoint.push_back(contoursSubBasePoint[i]);
            }
        }
        //myDrawContours(outMat, contoursSubCodeBasePoint, BEP_SCALAR_YELLOW, false);
        //myDrawContours(outMat, contoursSubAnswerBasePoint, BEP_SCALAR_GREEN, true);
        #pragma endregion

        #pragma region Tìm hình chữ nhật cơ sở của số báo danh và mã đề
        vector<vector<Point>> subCodeBase4PointContours = {
            contoursSubCodeBasePoint[0],
            contoursSubCodeBasePoint[1],
            contoursSubCodeBasePoint[contoursSubCodeBasePoint.size() - 2],
            contoursSubCodeBasePoint[contoursSubCodeBasePoint.size() - 1]
        };
        bepSort_Lef2Right_Top2Bottom(subCodeBase4PointContours, 2);
        //
        myDrawContours(outMat, subCodeBase4PointContours, BEP_SCALAR_YELLOW, true);
        //
        vector<Rect> subCodeBase4Point;
        for (rsize_t i = 0; i < subCodeBase4PointContours.size(); i++) {
            subCodeBase4Point.push_back(boundingRect(subCodeBase4PointContours[i]));
        }
        //
        Mat identityMat = outMat(Rect(
            subCodeBase4Point[0].br().x,
            subCodeBase4Point[0].br().y,
            subCodeBase4Point[3].tl().x - subCodeBase4Point[0].br().x,
            subCodeBase4Point[3].tl().y - subCodeBase4Point[0].br().y
        ));
        Mat codeMat = outMat(Rect(
            subCodeBase4Point[1].br().x,
            subCodeBase4Point[1].br().y,
            subCodeBase4Point[1].width * 5,
            subCodeBase4Point[3].tl().y - subCodeBase4Point[1].br().y
        ));

        //Tìm viền các hình tô đậm
        vector<int> identityNum;
        vector<int> codeNum;
        bepDetect(identityMat, identityNum, 10, Top2Bottom_Left2Right);
        bepDetect(codeMat, codeNum, 10, Top2Bottom_Left2Right);
        //myDrawContours(identityMat, codeContoursToSort);
        string strIdentityNum;
        for (int it : identityNum) {
            strIdentityNum += "|" + to_string(it);
        }
        putText(outMat, strIdentityNum, subCodeBase4Point[0].br(), FONT_HERSHEY_SIMPLEX, .5, BEP_SCALAR_BLUE, 2);
        string strCodeNum;
        for (int it : codeNum) {
            strCodeNum += "|" + to_string(it);
        }
        putText(outMat, strCodeNum, subCodeBase4Point[1].br(), FONT_HERSHEY_SIMPLEX, .5, BEP_SCALAR_BLUE, 2);
        #pragma endregion

        #pragma region Tìm các hình cơ sở của câu trả lời
        sort(contoursSubAnswerBasePoint.begin(), contoursSubAnswerBasePoint.end(), Top_Bottom_contour_sorter());
        vector<vector<Point>> subCodeBase6PointContours_Top = {
            contoursSubAnswerBasePoint[0],
            contoursSubAnswerBasePoint[1],
            contoursSubAnswerBasePoint[2]
        };
        sort(subCodeBase6PointContours_Top.begin(), subCodeBase6PointContours_Top.end(), Left_Right_contour_sorter());
        vector<Rect> subCodeBase6PointContours_Top_Bound;
        for (rsize_t i = 0; i < subCodeBase6PointContours_Top.size(); i++) {
            subCodeBase6PointContours_Top_Bound.push_back(boundingRect(subCodeBase6PointContours_Top[i]));
        }
        vector<vector<Point>> subCodeBase6PointContours_Bottom = {
            contoursSubAnswerBasePoint[contoursSubAnswerBasePoint.size() - 3],
            contoursSubAnswerBasePoint[contoursSubAnswerBasePoint.size() - 2],
            contoursSubAnswerBasePoint[contoursSubAnswerBasePoint.size() - 1]
        };
        sort(subCodeBase6PointContours_Bottom.begin(), subCodeBase6PointContours_Bottom.end(), Left_Right_contour_sorter());
        vector<Rect> subCodeBase6PointContours_Bottom_Bound;
        for (rsize_t i = 0; i < subCodeBase6PointContours_Bottom.size(); i++) {
            subCodeBase6PointContours_Bottom_Bound.push_back(boundingRect(subCodeBase6PointContours_Bottom[i]));
        }
        vector<Mat> matAnswerArr;
        for (rsize_t i = 0; i < subCodeBase6PointContours_Top_Bound.size(); i++) {
            Rect boundSubCodeBase6_Top = subCodeBase6PointContours_Top_Bound[i];
            //
            rsize_t fitSize = subCodeBase6PointContours_Top_Bound.size() > i + 1 ? i : i - 1;
            Rect boundSubCodeBase6_Fit_TL = subCodeBase6PointContours_Top_Bound[fitSize];
            Rect boundSubCodeBase6_Fit_BR = subCodeBase6PointContours_Bottom_Bound[fitSize + 1];
            
            Mat asw = outMat(Rect(
                boundSubCodeBase6_Top.br().x,
                boundSubCodeBase6_Top.br().y,
                boundSubCodeBase6_Fit_BR.tl().x - boundSubCodeBase6_Fit_TL.br().x,
                boundSubCodeBase6_Fit_BR.tl().y - boundSubCodeBase6_Fit_TL.br().y
            ));
            matAnswerArr.push_back(asw);
            //
            vector<int> valuers;
            bepDetect(asw, valuers, 4, Left2Right_Top2Bottom);
            string strValuers;
            for (int it : valuers) {
                strValuers += "|";
                switch (it)
                {
                case 0:
                    strValuers += "A";
                    break;
                case 1:
                    strValuers += "B";
                    break;
                case 2:
                    strValuers += "C";
                    break;
                case 3:
                    strValuers += "D";
                    break;
                default:
                    break;
                }
            }
            cout << "values: " << strValuers << endl;
            putText(outMat, strValuers, boundSubCodeBase6_Top.br(), FONT_HERSHEY_SIMPLEX, .5, BEP_SCALAR_BLUE, 2);
        }
        myDrawContours(outMat, subCodeBase6PointContours_Top, BEP_SCALAR_GREEN, true);
        myDrawContours(outMat, subCodeBase6PointContours_Bottom, BEP_SCALAR_GREEN, true);
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