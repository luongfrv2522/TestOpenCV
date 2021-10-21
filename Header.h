#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <string> 

using namespace cv;
using namespace std;

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


void myResize(Mat& img, Mat& imgReSize) {
	resize(img, imgReSize, Size(), 0.5, 0.5);
}