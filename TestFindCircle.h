#pragma once
class TestFindCircle
{
	public:
		static void Run() {
			try {
				Mat img = imread("Resources/50_cau_sau_khi_crop.jpg");
				Mat img2 = img.clone();
				Mat imgReSize, imgGray, imgThreshHold, imgDilate;

				cvtColor(img, imgGray, COLOR_BGR2GRAY);
				//imshow("Image1", imgGray);

				adaptiveThreshold(imgGray, imgThreshHold, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 45, 5);
				myResize(imgThreshHold, imgReSize);
				imshow("Image2.1", imgReSize);

				Mat imgThreshHold2;
				adaptiveThreshold(imgGray, imgThreshHold2, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 45, 70);
				myResize(imgThreshHold2, imgReSize);
				imshow("Image2.2", imgReSize);

				//Giãn nền
				Mat elementDilate = getStructuringElement(MORPH_CROSS, Size(3, 3), Point(1, 1));
				dilate(imgThreshHold2, imgDilate, elementDilate, Point(-1, -1), 3);
				myResize(imgDilate, imgReSize);
				imshow("Image3.1", imgReSize);
				//Co nền
				Mat elementErode = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
				erode(imgDilate, imgDilate, elementErode, Point(-1, -1), 3);
				myResize(imgDilate, imgReSize);
				imshow("Image3.2", imgReSize);

				vector<vector<Point>> contours;
				findContours(imgDilate, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
				cout << "------------------------------------------" << endl;
				cout << "countours.size: " << contours.size() << endl;
				cout << "------------------------------------------" << endl;
				for (size_t i = 0; i < contours.size(); i++) {
					Rect r = boundingRect(contours[i]);
					if (r.area() < 5000) {
						rectangle(img2, r, Scalar(0, 255, 0), 2, 8, 0);
					}
				}

				myResize(img2, imgReSize);
				imshow("Image4", imgReSize);
			}
			catch (Exception ex) {
				cout << "Exception: " << ex.msg << endl;
			}
		}
};

