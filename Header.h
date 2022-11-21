#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <string> 

using namespace cv;
using namespace std;

Scalar BEP_SCALAR_GREEN = Scalar(0, 255, 0, 255);
Scalar BEP_SCALAR_BLUE = Scalar(255, 0, 0, 255);
Scalar BEP_SCALAR_RED = Scalar(0, 0, 255, 255);
Scalar BEP_SCALAR_YELLOW = Scalar(0, 255, 255, 255);

struct Left_Right_contour_sorter // 'less' for contours
{
	bool operator ()(const vector<Point>& a, const vector<Point>& b)
	{
		Rect ra(boundingRect(a));
		Rect rb(boundingRect(b));
		return ra.x < rb.x;
	}
};
struct Top_Bottom_contour_sorter // 'less' for contours
{
	bool operator ()(const vector<Point>& a, const vector<Point>& b)
	{
		Rect ra(boundingRect(a));
		Rect rb(boundingRect(b));
		return ra.y < rb.y;
	}
};
struct area_desc_contour_sorter // 'less' for contours
{
	bool operator ()(const vector<Point>& a, const vector<Point>& b)
	{
		Rect ra(boundingRect(a));
		Rect rb(boundingRect(b));
		return ra.area() > rb.area();
	}
};

void myResize(Mat& img, Mat& imgReSize) {
	resize(img, imgReSize, Size(), 1, 1);
}

void persTrans(vector<vector<Point>> &inputMat, vector<vector<Point>> &dst, Mat& matrix) {
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

void myDrawContours(Mat& src, vector<vector<Point>> contours, Scalar color, bool hasLabel) {
	int count = 0;
	for (size_t i = 0; i < contours.size(); i++) {
		count++;
		Rect r = boundingRect(contours[i]);
		//
		Point point4 = Point((r.br() + r.tl()) * .5);
		circle(src, point4, 1, 255, 3);
		rectangle(src, r, color, 2, 8, 0);
		String text = to_string(count);
		cout << text << endl;
		if (hasLabel) {
			putText(src, text, point4, FONT_HERSHEY_SIMPLEX, 0.5, BEP_SCALAR_BLUE, 2);
		}
	}
}
void myDrawContours(Mat& src, vector<vector<Point>> contours, bool hasLabel) {
	myDrawContours(src, contours, BEP_SCALAR_GREEN, hasLabel);
}
void myDrawContours(Mat& src, vector<vector<Point>> contours) {
	myDrawContours(src, contours, BEP_SCALAR_GREEN, false);
}