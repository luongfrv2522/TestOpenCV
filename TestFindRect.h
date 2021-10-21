class TestFindRect
{
public:
	static void Run() {
		try {
			Mat img = imread("Resources/50_cau.jpg");
			Mat imgReSize, imgGray, imgThreshHold, imgDilate;

			cvtColor(img, imgGray, COLOR_BGR2GRAY);
			//imshow("Image1", imgGray);

			adaptiveThreshold(imgGray, imgThreshHold, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 45, 5);
			myResize(imgThreshHold, imgReSize);
			//imshow("Image2", imgReSize);

			//Giãn nền
			Mat elementDilate = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
			dilate(imgThreshHold, imgDilate, elementDilate, Point(-1, -1), 3);
			myResize(imgDilate, imgReSize);
			imshow("Image3.1", imgReSize);
			Mat elementErode = getStructuringElement(MORPH_RECT, Size(7, 7), Point(3, 3));
			erode(imgDilate, imgDilate, elementErode, Point(-1, -1), 3);
			myResize(imgDilate, imgReSize);
			imshow("Image3.2", imgReSize);

			//Tìm viền các hình chữ nhật neo góc
			vector<vector<Point>> contours;
			findContours(imgDilate, contours, RETR_LIST, CHAIN_APPROX_NONE);
			Mat img2 = img.clone();
			cout << "countours.size: " << contours.size() << endl;
			int count = 0;
			vector<vector<Point>> contoursToSort;
			for (size_t i = 0; i < contours.size(); i++) {
				Rect r = boundingRect(contours[i]);
				if (r.width / (double)r.height < 1.5 && r.width / (double)r.height > 0.75 && r.area() > 600) {
					contoursToSort.push_back(contours[i]);
				}
			}

			sort(contoursToSort.begin(), contoursToSort.end(), Top_Bottom_contour_sorter());

			for (size_t i = 0; i < contoursToSort.size(); i++) {
				count++;
				Rect r = boundingRect(contoursToSort[i]);
				Scalar scalar = r.area() > 20 ? Scalar(0, 255, 0) : Scalar(0, 0, 255);
				rectangle(img2, r, scalar, 2, 8, 0);
				String text = to_string(count) + "-" + to_string(r.area());
				putText(img2, text, Point(r.x, r.y), FONT_HERSHEY_SIMPLEX, 1, 255, 3);
				//
				Point point4 = Point((r.br() + r.tl()) * .5);
				circle(img2, point4, 1, 255, 3);
			}

			myResize(img2, imgReSize);
			imshow("Image4", imgReSize);
		}
		catch (Exception ex) {
			cout << "Exception: " << ex.msg << endl;
		}
	}

};

