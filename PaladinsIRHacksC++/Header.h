#pragma once
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include<thread>

using namespace cv;
using namespace std;





BITMAPINFOHEADER createBitmapHeader(int width, int height);

Mat captureScreenMat(HWND hwnd);

void mouseMove(int x, int y);

