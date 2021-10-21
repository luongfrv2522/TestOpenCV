#pragma once
class TestFindCircle2
{
	public:
		static void Run() {
			try {
				Mat img = imread("Resources/50_cau_sau_khi_crop_lan_2.jpg");
				Mat img2 = img.clone();
				Mat imgReSize, imgGray, imgThreshHold, imgDilate;

				cvtColor(img, imgGray, COLOR_BGR2GRAY);
				//imshow("Image1", imgGray);

				adaptiveThreshold(imgGray, imgThreshHold, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 45, 5);
				imshow("Image2.1", imgThreshHold);

				Mat imgThreshHold2;
				adaptiveThreshold(imgGray, imgThreshHold2, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 45, 70);
				imshow("Image2.2", imgThreshHold2);


				//Co nền
				Mat elementErode = getStructuringElement(MORPH_ELLIPSE, Size(5, 5), Point(2, 2));
				erode(imgThreshHold2, imgThreshHold2, elementErode, Point(-1, -1), 3);
				imshow("Image3.2", imgThreshHold2);

				/*GaussianBlur(imgThreshHold2, imgThreshHold2, Size(5, 5), 2);
				Canny(imgThreshHold2, imgThreshHold2, 30, 200, 3, false);
				imshow("Image2.2", imgReSize);*/

				vector<vector<Point>> contours;
				findContours(imgThreshHold2, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
				vector<vector<Point>> contoursForSort;
				for (size_t i = 0; i < contours.size(); i++) {
					Rect r = boundingRect(contours[i]);
					if (r.area() > 500 && r.area() < 5000) {
						contoursForSort.push_back(contours[i]);
					}
				}
				cout << "------------------------------------------" << endl;
				cout << "countours.size: " << contoursForSort.size() << endl;
				cout << "------------------------------------------" << endl;

				sort(contoursForSort.begin(), contoursForSort.end(), Top_Bottom_contour_sorter());
				for (size_t i = 0; i < contoursForSort.size(); i++) {
					if ((i+1) % 4==0) {
						vector<vector<Point>> contours4;
						for (size_t j = 0; j < 4; j++) {
							contours4.push_back(contoursForSort[i-j]);
						}
						sort(contours4.begin(), contours4.end(), Left_Right_contour_sorter());
						for (size_t j = 0; j < contours4.size(); j++) {
							Rect r = boundingRect(contours4[j]);
							rectangle(img2, r, Scalar(0, 255, 0), 2, 8, 0);
							putText(img2, to_string(j), Point(r.x, r.y), FONT_HERSHEY_SIMPLEX, 1, 255, 3);
						}
					}
				}

				imshow("Image4", img2);
			}
			catch (Exception ex) {
				cout << "Exception: " << ex.msg << endl;
			}
		}
};

