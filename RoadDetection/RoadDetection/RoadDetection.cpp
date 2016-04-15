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

void print_enkel_mask(string filename, string maskname, string ext, string number) {
	Mat image, mask;	//read image
	image = imread(filename + ext, CV_LOAD_IMAGE_COLOR);
	mask = imread(maskname + ext, CV_LOAD_IMAGE_COLOR);
	//cout << mask << endl << endl;
	threshold(mask, mask, 1, 255, 0);
	//imshow("window", mask);
	//waitKey();
	Mat contour_mask;
	Canny(mask, contour_mask, 50, 150);
	//imshow("window", contour_mask);
	//waitKey();
	bitwise_and(image, mask, image);
	Mat contour;
	Canny(image, contour, 50, 150);
	namedWindow("window", 1);
	//imshow("window", image);
	imshow("window", contour);
	waitKey();
	
}

string format(int num) {
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(5) << num;
	return oss.str();
};

int main()
{
	for (int i = 10; i < 106; i++) {
		string map = "01";
		String filename = "../../../images/" + map + "/frame" + format(i); 
		String maskname = "../../../images/" + map + "/mask" + format(i);
		String ext = ".png";
		print_enkel_mask(filename, maskname, ext, format(i));
		std::cout << "EINDE" << std::endl;
	}
	
    return 0;
}



