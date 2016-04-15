// RoadDetection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv\highgui.h>
#include <iostream>
#include <iomanip>
using namespace std;
using namespace cv;

void print_enkel_mask(string filename, string maskname, string ext) {
	Mat image, mask;	//read image
	image = imread(filename + ext, CV_LOAD_IMAGE_COLOR);
	mask = imread(maskname + ext, CV_LOAD_IMAGE_COLOR);
	threshold(mask, mask, 1, 255, 0);
	bitwise_and(image, mask, image);
	namedWindow("window", 1);
	imshow("window", image);
	waitKey();
}

string format(int num) {
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(5) << num;
	return oss.str();
};

int main()
{
	for (int i = 0; i < 96; i++) {
		string map = "02";
		String filename = "../../../images/" + map + "/frame" + format(i); 
		String maskname = "../../../images/" + map + "/mask" + format(i);
		String ext = ".png";

		print_enkel_mask(filename, maskname, ext);
		std::cout << "EINDE" << std::endl;
	}
	
    return 0;
}



