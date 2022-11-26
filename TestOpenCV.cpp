﻿#include "Header.h"

using namespace grd;

void bepMainDetect(Mat& img, string identity) {
	try {
		float w = 768, h = 1024;
		float wShow = 576, hShow = 768;
		//
		float hCodeVsAnswer = h / 3;
		Mat imgReSize, imgGray, imgThreshHold, imgDilate, imgErode, outMat;
		//
		resize(img, img, Size(w, h));

		//bep_ImResizeAndShow("Image1.1", img, wShow, hShow);

		cvtColor(img, imgGray, COLOR_BGR2GRAY);
		adaptiveThreshold(imgGray, imgThreshHold, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 45, 5);
		//bep_ImResizeAndShow("imgThreshHold", imgThreshHold, wShow, hShow);
		//Giãn nền
		Mat elementDilate = getStructuringElement(MORPH_CROSS, Size(3, 3), Point(1, 1));
		dilate(imgThreshHold, imgDilate, elementDilate, Point(-1, -1), 3);
		//bep_ImResizeAndShow("imgDilate", imgDilate, wShow, hShow);

		Mat elementErode = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
		erode(imgDilate, imgErode, elementErode, Point(-1, -1), 3);
		//bep_ImResizeAndShow("imgErode", imgErode, wShow, hShow);

		//Tìm viền các hình tô đậm
		vector<vector<Point>> contours;
		findContours(imgErode, contours, RETR_TREE, CHAIN_APPROX_NONE);

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
/**/	#pragma region Find contours
		vector<vector<Point>> contours4BasePoint;
		vector<vector<Point>> contoursSubBasePoint;
		if (contoursToSort.size() >= 14) {
			sort(contoursToSort.begin(), contoursToSort.end(), Bep_AreaDesc_ContourSorter());
			for (size_t i = 0; i < contoursToSort.size(); i++) {
				if (i < 4) {
					contours4BasePoint.push_back(contoursToSort[i]);
				}
				else {
					contoursSubBasePoint.push_back(contoursToSort[i]);
				}
			}

			sort(contours4BasePoint.begin(), contours4BasePoint.end(), Bep_Top2Bottom_ContourSorter());
			sort(contours4BasePoint.begin(), contours4BasePoint.begin() + 2, Bep_Left2Right_ContourSorter());
			sort(contours4BasePoint.begin() + 2, contours4BasePoint.end(), Bep_Left2Right_ContourSorter());


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

			bep_PerspectiveTransform(contours4BasePoint, contours4BasePoint, matrix);
			bep_PerspectiveTransform(contoursSubBasePoint, contoursSubBasePoint, matrix);
			r1 = boundingRect(contours4BasePoint[0]);
			r2 = boundingRect(contours4BasePoint[1]);
			r3 = boundingRect(contours4BasePoint[2]);
			r4 = boundingRect(contours4BasePoint[3]);

			//drawToContour(outMat, contours4BasePoint);

			/*BEGIN*Chia phần câu hỏi và mã đề*/
			sort(contoursSubBasePoint.begin(), contoursSubBasePoint.end(), Bep_Top2Bottom_ContourSorter());
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
			/*END*Chia phần câu hỏi và mã đề*/

			/*BEGIN*Tìm hình chữ nhật cơ sở của số báo danh và mã đề*/
			vector<vector<Point>> subCodeBase4PointContours = {
				contoursSubCodeBasePoint[0],
				contoursSubCodeBasePoint[1],
				contoursSubCodeBasePoint[contoursSubCodeBasePoint.size() - 2],
				contoursSubCodeBasePoint[contoursSubCodeBasePoint.size() - 1]
			};
			bep_Sort_Lef2Right_Top2Bottom(subCodeBase4PointContours, 2);
			//
			bep_DrawContours(outMat, subCodeBase4PointContours, BEP_SCALAR_YELLOW, true);
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
			bep_DetectCircleMatrix(identityMat, identityNum, 10, Top2Bottom_Left2Right);
			bep_DetectCircleMatrix(codeMat, codeNum, 10, Top2Bottom_Left2Right);
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
			/*END*Tìm hình chữ nhật cơ sở của số báo danh và mã đề*/

/**/		#pragma region Tìm các hình cơ sở của câu trả lời
			sort(contoursSubAnswerBasePoint.begin(), contoursSubAnswerBasePoint.end(), Bep_Top2Bottom_ContourSorter());
			vector<vector<Point>> subCodeBase6PointContours_Top = {
				contoursSubAnswerBasePoint[0],
				contoursSubAnswerBasePoint[1],
				contoursSubAnswerBasePoint[2]
			};
			sort(subCodeBase6PointContours_Top.begin(), subCodeBase6PointContours_Top.end(), Bep_Left2Right_ContourSorter());
			vector<Rect> subCodeBase6PointContours_Top_Bound;
			for (rsize_t i = 0; i < subCodeBase6PointContours_Top.size(); i++) {
				subCodeBase6PointContours_Top_Bound.push_back(boundingRect(subCodeBase6PointContours_Top[i]));
			}
			vector<vector<Point>> subCodeBase6PointContours_Bottom = {
				contoursSubAnswerBasePoint[contoursSubAnswerBasePoint.size() - 3],
				contoursSubAnswerBasePoint[contoursSubAnswerBasePoint.size() - 2],
				contoursSubAnswerBasePoint[contoursSubAnswerBasePoint.size() - 1]
			};
			sort(subCodeBase6PointContours_Bottom.begin(), subCodeBase6PointContours_Bottom.end(), Bep_Left2Right_ContourSorter());
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
				bep_DetectCircleMatrix(asw, valuers, 4, Left2Right_Top2Bottom);
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
				//putText(outMat, strValuers, boundSubCodeBase6_Top.br(), FONT_HERSHEY_SIMPLEX, .5, BEP_SCALAR_BLUE, 2);
			}
			bep_DrawContours(outMat, subCodeBase6PointContours_Top, BEP_SCALAR_YELLOW, false);
			bep_DrawContours(outMat, subCodeBase6PointContours_Bottom, BEP_SCALAR_YELLOW, false);
/**/		#pragma endregion
		}
		else if (contoursToSort.size() < 4) {
			cout << "---------------------------------" << endl;
			cout << "WARNING NO FOUND 4Point " << contours.size() << endl;
		}
/**/	#pragma endregion

/**/	#pragma region show outMat
		//line(outMat, Point(0, hCodeVsAnswer), Point(w, hCodeVsAnswer), BEP_SCALAR_BLUE, 5, LINE_8);
		resize(outMat, imgReSize, Size(wShow, hShow));
		imshow("Image_" + identity, imgReSize);
/**/	#pragma endregion show outMat
	}
	catch (Exception e) {
		cout << e.msg << endl;
	}
}

void bepMainDetect2Test(Mat& img, string identity) {
	try {
		float w = 768, h = 1024;
		float wShow = 576, hShow = 768;
		//
		float hCodeVsAnswer = h / 3;
		Mat imgReSize, imgGray, imgThreshHold, imgDilate, outMat;
		//
		resize(img, img, Size(w, h));
		//bep_ImResizeAndShow("Image1.1", img, wShow, hShow);

		cvtColor(img, imgGray, COLOR_BGR2GRAY);
		//adaptiveThreshold(imgGray, imgThreshHold, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 45, 5);
		GaussianBlur(imgGray, imgThreshHold, Size(5, 5), 0);
		//
		//bep_ImResizeAndShow("Image adaptiveThreshold", img, wShow, hShow);
		//
		Mat canny;
		Canny(imgThreshHold, canny, 75, 200);
		//bep_ImResizeAndShow("Image candy", img, wShow, hShow);
	}
	catch (Exception e) {
		cout << e.msg << endl;
	}
}


int main()
{
	//TestFindRect::Run();

	//TestFindCircle::Run();

	//TestFindCircle2::Run();

	//
	Mat img = imread("Resources/50_cau.jpg");
	Mat img1 = imread("Resources/50_cau_1.jpg");
	Mat img2 = imread("Resources/50_cau_2.jpg");
	bepMainDetect(img, "50_cau");
	bepMainDetect(img1, "50_cau_1");
	bepMainDetect(img2, "50_cau_2");

	/*Mat img5 = imread("Resources/50_cau_5.jpg");
	bepMainDetect2Test(img5, "50_cau_5");*/
#pragma endregion
	waitKey(0);
}