#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

using namespace cv;
using namespace std;

int main()
{
    Mat img = imread("Resources/50_cau.jpg");
    Mat imgReSize, imgGray, imgThreshHold, imgDilate;
    resize(img, imgReSize, Size(), 0.5, 0.5);
    //imshow("Image", imgReSize);

    cvtColor(imgReSize, imgGray, COLOR_BGR2GRAY);
    //imshow("Image1", imgGray);
    
    adaptiveThreshold(imgGray, imgThreshHold, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 35, 5);
    imshow("Image2", imgThreshHold);
    
    //Giãn nền
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5), Point(2, 2));
    dilate(imgThreshHold, imgDilate, element, Point(-1, -1), 3);
    erode(imgDilate, imgDilate, element, Point(-1, -1), 3);
    imshow("Image3", imgDilate);

    //Tìm viền các hình chữ nhật neo góc
    vector<vector<Point>> countours;
    findContours(imgDilate, countours, RETR_LIST, CHAIN_APPROX_NONE);
    Mat img2 = imgReSize.clone();
    for (size_t i = 0; i < countours.size(); i++) {
        Rect r = boundingRect(countours[i]);
        if (r.width / (double)r.height < 1.1 && r.width / (double)r.height > 0.75) {
            rectangle(img2, r, Scalar(0, 255, 0), 1, 8, 0);
        }
    }
    imshow("Image4", img2);

    waitKey(0);
}