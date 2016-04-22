// RoadDetection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv\highgui.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
using namespace std;
using namespace cv;


const int OUR_MAX_SPEED = 88;
const int THRESHOLD_90 = 30;
static int false_positive_90 = 0;
static int true_positive_90 = 0;
static int false_negative_90 = 0;
static int true_negative_90 = 0;
const bool USING_THREADS = false;

double bepaal_rico_weg(Mat & m, Point & p1, Point & p2, bool & goes_overboard) { // GEEF IMAGE MEE EN 2 POINTS, GEWOON LEGE POINTS
	Point y1, y2, x1, x2;
	goes_overboard = false;
	bool y1found = false, y2found = false, x1found = false, x2found = false;
	for (int i = 0; i<m.rows; i++) {
		if (m.at<float>(m.cols - 1, i) != 0 && !y1found) {//laatste colom overlopen eerste coordinaat;
			y1found = true;
			y1.y = i;
			y1.x = m.rows;
		}
		else {
			if (m.at<float>(m.cols - 1, i) == 0 && !y2found) { //laatste colom overlopen tweede coordinaat;
				y2found = true;
				y2.y = i;
				y2.x = m.rows;
			}
		}
		if (m.at<float>(0, i) != 0 && !y1found) { //eerste colom overlopen eerste coordinaat;
			y1found = true;
			y1.y = i;
			y1.x = m.rows;
		}
		else {
			if (m.at<float>(0, i) == 0 && !y2found) { //eerste colom overlopen tweede coordinaat;
				y2found = true;
				y2.y = i;
				y2.x = m.rows;
			}
		}
	}

	for (int i = 0; i<m.cols; i++) { //laatste rij overlopen
		if (m.at<float>(i, m.rows - 1) != 0 && !x1found) {
			x1found = true;
			x1.x = i;
			x1.y = m.rows;
		}
		else {
			if (m.at<float>(i, m.rows - 1) == 0 && !x2found) {
				x2found = true;
				x2.x = i;
				x2.y = m.rows;
			}
		}
	}
	if (y1found && y2found && x1found && x2found) {
		Point bocht = Point(m.rows - 1, y2.y - y1.y);
		Point begin = Point(x2.x - x1.x, m.rows - 1);
		goes_overboard = true;

		return (double)(bocht.y - begin.y) / (double)(bocht.x - begin.x);
	}
	else {
		int _col = 0;
		int _row = 0;
		while (m.at<float>(_row, _col) == 0) {
			_col++;
			if (_col == m.cols) {
				_col = 0;
				_row++;
			}
		}
		p1 = Point(_row, _col);
		while (m.at<float>(_row, _col) != 0) {
			_col++;
		}
		if (_col == m.cols) { //return thesame pixel when only one on a line
			p2 = Point(p1);
			return -1;
		}
		else { // return other pixel on the row where the road ends
			p2 = Point(_row, _col);
			Point bocht = Point(p2.x - p1.x, p2.y);
			Point begin = Point(x2.x - x1.x, m.rows - 1);

			return (double)(bocht.y - begin.y) / (double)(bocht.x - begin.x);

		}
	}
};


void print_enkel_mask(string filename, string maskname, string ext, string number, int oplossing) {
	Mat image, mask;	//read image
	image = imread(filename + ext, CV_LOAD_IMAGE_COLOR);
	mask = imread(maskname + ext, CV_LOAD_IMAGE_COLOR);
	//imshow("original", image + mask);
	//cout << mask << endl << endl;
	Mat original_mask = mask.clone();
	threshold(mask, mask, 1, 255, 0);
	//imshow("window", mask);
	//waitKey();
	Mat contour_mask;
	Canny(mask, contour_mask, 50, 150);
	//imshow("window", contour_mask);
	//waitKey();
	Mat cutout_image;
	bitwise_and(image, mask, cutout_image);
	Mat contour;
	Canny(cutout_image, contour, 50, 150);
	//imshow("window", image);
	Mat cannied_masked_image = contour - contour_mask;
	

	vector<Mat> channels;
	split(cannied_masked_image, channels);
	Scalar m = mean(channels[0]);
	double aantal_pixels = m[0] * cannied_masked_image.rows * cannied_masked_image.cols / 255;
	
	//cout << aantal_pixels << endl;
	//imshow("output", cannied_masked_image);
	

	if (aantal_pixels <= THRESHOLD_90) { // WE SAY 90
		if (oplossing < OUR_MAX_SPEED) { // SOLUTION SAYS NOT 90
			//cout << "Oplossing : " << oplossing << " Verkeerd toegelaten\n";
			cout << "Oplossing: " << oplossing << "\tAantal pixels: " << aantal_pixels << " File: " << filename << endl;
			string windowname = filename;
			windowname += " ";
			windowname += oplossing;
			windowname += " ";
			windowname += aantal_pixels;
			imshow(windowname, image + original_mask);
			false_positive_90++; // SHOULD BE ZERO OR ELSE WE CRASH
			waitKey();
		}
		else { //SOLUTION SAYS 90
			true_positive_90++;
		}
	}
	else { // WE SAY NOT 90
		if (oplossing >= OUR_MAX_SPEED) { // SOLUTION SAYS 90
			//cout << "Oplossing: " << oplossing << " Verkeerd rejected\n";
			false_negative_90++;
			//waitKey();
		}
		else { //SOLUTION SAYS NOT 90
			true_negative_90++;
		}

		// NEED TO BE EXAMINED FURTHER
		bool goes_overboard = false;
		Point x, y;
		//cout << "Rico: " << bepaal_rico_weg(original_mask, x,y, goes_overboard);
		//cin.get();


	}
	//cout << "Rico: " << bepaal_rico_weg(mask) << endl;
	
	
}

string format(int num, int length) {
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(length) << num;
	return oss.str();
};


int main()
{
	vector<thread> draad;
	for (int j = 1; j <= 4;j++) {
		string map_number = format(j,2);
		cout << "MAP: " << j << endl;
		string oplossing_file = "C:/Project_Computervisie/images/" + map_number + "/gtdistances.txt";
		ifstream in;
		in.open(oplossing_file);

		string map = "../../../images/";
		map += map_number;
		int i = 0;
		string filename = map + "/frame" + format(i,5);
		Mat image = imread(filename + ".png");
		while (!image.empty()) {
			int oplossing;
			in >> oplossing;
			in >> oplossing;
			string weg;
			getline(in, weg);
			string maskname = map + "/mask" + format(i,5);
			string ext = ".png";
			if (USING_THREADS) {
				draad.push_back(thread(print_enkel_mask, filename, maskname, ext, format(i, 5), oplossing));
			}
			else {
				print_enkel_mask(filename, maskname, ext, format(i, 5), oplossing);
			}
			i++;
			filename = map + "/frame" + format(i,5);
			image = imread(filename + ext);
			//cout << i << " ";
		}
		cout << endl;
	}

	if (USING_THREADS) {
		for (int i = 0; i < draad.size(); i++)
		{
			draad[i].join();
		}
	}

	
	cout << endl;
	cout << "True positive 90: " << true_positive_90 << " WE ARE CLASSIFYING THOSE WELL" << endl;
	cout << "False positive 90: " << false_positive_90 << " SHOULD BE ZERO OR ELSE WE CRASH" << endl;
	cout << "True negative 90: " << true_negative_90 << " WE WILL FURTHER EXAMINE THOSE" << endl;
	cout << "False negative 90: " << false_negative_90 << " WE WILL FURTHER EXAMINE THOSE" << endl;
	cout << "Total: " << true_positive_90 + true_negative_90 + false_negative_90 + false_positive_90 << endl;
	cin.get();
	
    return 0;
}



