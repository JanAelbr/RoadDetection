// RoadDetection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv\highgui.h>
#include <iostream>
using namespace std;
using namespace cv;


int main()
{
	String filename = "C:\\School\\5e Jaar\\Computervisie\\images\\clouds";
	String ext = ".png";

	Mat image1;	//read image
	image1 = imread(filename + ext, CV_LOAD_IMAGE_COLOR);
	namedWindow("window", 1);
	imshow("window", image1);
	waitKey();
	std::cout << "EINDE" << std::endl;
    return 0;
}

