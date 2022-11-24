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

static enum BepDetect_Type {
	Top2Bottom_Left2Right,
	Left2Right_Top2Bottom
};

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
	for (size_t i = 0; i < contours.size(); i++) {
		Rect r = boundingRect(contours[i]);
		//
		Point point4 = Point((r.br() + r.tl()) * .5);
		//circle(src, point4, 1, 255, 3);
		rectangle(src, r, color, 2, 8, 0);
		String text = to_string(i+1);
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

void bepDrawCircleOfContour(Mat& src, vector<Point> contour, Scalar color, int thickness = 1) {
	Rect r = boundingRect(contour);
	//
	Point point4 = Point((r.br() + r.tl()) * .5);
	circle(src, point4, (r.width + r.height) / 4, color, thickness, 0);
}

void bepDrawPointContours(Mat& src, vector<vector<Point>> contours, Scalar color, bool hasLabel) {
	int count = 0;
	for (size_t i = 0; i < contours.size(); i++) {
		count++;
		bepDrawCircleOfContour(src, contours[i], color);
		String text = to_string(count);
		cout << text << endl;
		if (hasLabel) {
			putText(src, text, contours[i][0], FONT_HERSHEY_SIMPLEX, 0.5, BEP_SCALAR_BLUE, 2);
		}
	}
}

void bepDetectCircleMatrix(Mat& identityMat, vector<int>& identityNum, int roundLine, BepDetect_Type type) {
	function<int(vector<Point>&, vector<Point>&)> firstPred, secondPred;
	if (type == Top2Bottom_Left2Right) {
		firstPred = Left_Right_contour_sorter();
		secondPred = Top_Bottom_contour_sorter();
	}
	else if(Left2Right_Top2Bottom) {
		firstPred = Top_Bottom_contour_sorter();
		secondPred = Left_Right_contour_sorter();
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
				bepDrawCircleOfContour(identityMat, codeContoursToSort[j], BEP_SCALAR_GREEN, 2);
				break;
			}
		}
		/*if (tempVal < 0) {
			bepDrawCircleOfContour(identityMat, codeContoursToSort[i], BEP_SCALAR_RED, 2);
		}*/
		identityNum.push_back(tempVal);
	}
}


void bepSort_Top2Bottom_Lef2Right(vector<vector<Point>>& input, int roundLine) {
	sort(input.begin(), input.end(), Left_Right_contour_sorter());
	for (size_t i = 0; i < input.size(); i = i + roundLine) {
		sort(input.begin() + i, input.begin() + i + roundLine, Top_Bottom_contour_sorter());
	}
}
void bepSort_Lef2Right_Top2Bottom(vector<vector<Point>>& input, int roundLine) {
	sort(input.begin(), input.end(), Top_Bottom_contour_sorter());
	for (size_t i = 0; i < input.size(); i = i + roundLine) {
		sort(input.begin() + i, input.begin() + i + roundLine, Left_Right_contour_sorter());
	}
}