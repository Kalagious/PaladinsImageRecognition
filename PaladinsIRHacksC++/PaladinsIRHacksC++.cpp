
#include "Header.h"


const int max_value_H = 180;
const int max_value = 255;
int low_H = 0, low_S = 142, low_V = 80;
int high_H = 0, high_S = 201, high_V = 255;
int blurSize = 17;
float aimThreshold = 5;
float minAimSpeed = 1;
float maxAimSpeed = 15;
float maxDistance = 30;
float strength = 0.06;

RNG rng(12345);

bool display = true;

atomic<int> targetX;
atomic<int> targetY;
int lastX, lastY;
int yOffset = 7;
int down_width = 150;
int down_height = 100;

const String window_detection_name = "Object Detection";

void manageMouse()
{
	while (true)
	{
		if (targetX != -1)
		{
			float xDist = (targetX - down_width / 2) * strength;
			float yDist = (targetY + yOffset - down_height / 2) * strength;
			if (xDist < minAimSpeed && xDist > 0 && abs(targetX - down_width / 2) > aimThreshold)
				xDist = minAimSpeed;
			else if (xDist > -1 * minAimSpeed && xDist < 0 && abs(targetX - down_width / 2) > aimThreshold)
				xDist = minAimSpeed * -1;
			else if (abs(xDist) > maxAimSpeed)
				xDist = maxAimSpeed;


			if (abs(xDist) < maxDistance && (GetKeyState(VK_LBUTTON) & 0x8000) != 0)
				mouseMove((int)(xDist), 0);
		}
	}
}



int main()
{	

	HWND hwnd = FindWindowA(NULL, "Paladins (32-bit, DX9)");
	RECT rect = { 0 };
	GetWindowRect(hwnd, &rect);


	
	if (display)
		namedWindow(window_detection_name);

	Mat frame, frame_HSV, frame_filtered;
	int FPS = 0;

	

	/*createTrackbar("Low H", window_detection_name, &low_H, max_value_H, NULL);
	createTrackbar("High H", window_detection_name, &high_H, max_value_H, NULL);
	createTrackbar("Low S", window_detection_name, &low_S, max_value, NULL);
	createTrackbar("High S", window_detection_name, &high_S, max_value, NULL);
	createTrackbar("Low V", window_detection_name, &low_V, max_value, NULL);
	createTrackbar("High V", window_detection_name, &high_V, max_value, NULL);
	createTrackbar("Blur", window_detection_name, &blurSize, 50, NULL);*/
	

	SimpleBlobDetector::Params params;
	/*params.filterByArea = true;
	params.minArea = 10;*/

	Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	std::thread mouseThread(manageMouse);
	Mat killfeedMask = imread("./mask.png", 0);
	bitwise_not(killfeedMask, killfeedMask);
	resize(killfeedMask, killfeedMask, Size(down_width, down_height), INTER_LINEAR);

	while (true)
	{
		auto start = std::chrono::system_clock::now();
		// capture image
		frame = captureScreenMat(hwnd);
		resize(frame, frame, Size(down_width, down_height), INTER_LINEAR);
		if (frame.empty())
		{
			std::cout << "Could not read the image" << std::endl;
			return 1;
		}

		// Convert from BGR to HSV colorspace
		cvtColor(frame, frame_HSV, COLOR_BGR2HSV);
			
		// Detect the object based on HSV Range Values
		inRange(frame_HSV, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), frame_filtered);

		blur(frame_filtered, frame_filtered, Size(blurSize, blurSize));
		inRange(frame_filtered, Scalar(1, 1, 1), Scalar(180, 255, 255), frame_filtered);


		/*std::vector<KeyPoint> keypoints;
		detector->detect(frame_filtered, keypoints);
		drawKeypoints(frame_filtered, keypoints, frame_filtered, Scalar(0, 255, 0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);*/

		bitwise_and(killfeedMask, frame_filtered, frame_filtered);
		findContours(frame_filtered, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		vector<Point2f>centers(contours.size());
		vector<float>radius(contours.size());
		vector<vector<Point> > contours_poly(contours.size());

		for (size_t i = 0; i < contours.size(); i++)
		{
			approxPolyDP(contours[i], contours_poly[i], 3, true);
			minEnclosingCircle(contours_poly[i], centers[i], radius[i]);
		}
		int closestI = -1, closestDist = 1000;

		for (int i = 0; i < contours.size(); i++)
		{
			Scalar color = Scalar(0, 255, 0);
			int dist = pow((int)centers[i].x-(int)frame.cols/2,2.0) + pow((int)centers[i].y-yOffset-(int)frame.rows/2,2);
			dist = sqrt(dist);
			if (display)
			{
				circle(frame, centers[i], (int)radius[i], color, 2);
				cv::putText(frame, std::to_string(dist), centers[i], cv::FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(118, 185, 0), 2);
			}
			if (dist<closestDist)
			{
				closestI = i;
				closestDist = dist;
			}
		}
		//cout << centers[closestI] << '\n';
		if (closestI != -1)
		{
			line(frame,
				Point(frame.cols / 2, frame.rows / 2 + 50),
				centers[closestI],
				Scalar(0, 255, 0),
				2,
				LINE_8);
			RECT rect = { 0 };


			targetX = centers[closestI].x+(centers[closestI].x-lastX)*5;
			targetY = centers[closestI].y+(centers[closestI].y-lastY)*5;
			lastX = centers[closestI].x;
			lastY = centers[closestI].y;
			
		}
		else {
			targetX = -1;
			targetY = -1;
		}
		if (display)
		{
			cv::putText(frame, std::to_string(FPS), cv::Point(10, frame.rows / 2), cv::FONT_HERSHEY_DUPLEX, 1.0, CV_RGB(118, 185, 0), 2);
			imshow(window_detection_name, frame);
		}

		char key = (char)waitKey(1);

		
		auto end = std::chrono::system_clock::now();
		//end - start is the time taken to process 1 frame, output it:
		std::chrono::duration<double> diff = end - start;
		FPS = 1.0 / diff.count();
		//cout << FPS << "\n";
		
	}

	return 0;
}

