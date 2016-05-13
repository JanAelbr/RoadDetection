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


const int OUR_SPEED_90 = 90;
const int OUR_SPEED_70 = 70;
const int INITIAL_THRESHOLD_90 = 75;
const int INITIAL_THRESHOLD_70 = 150;
static int false_positive_90 = 0;
static int true_positive_90 = 0;
static int false_negative_90 = 0;
static int true_negative_90 = 0;
static int false_positive_70 = 0;
static int true_positive_70 = 0;
static int false_negative_70 = 0;
static int true_negative_70 = 0;
const bool USING_THREADS = true;
static int marge = 0;
const int MIN_CONTOURS = 1;
ofstream file;
ofstream results;
vector<string> oplossingen;

int geef_minimum_van_vvv(const vector<vector<Point>> &vvv, string maskname, string ext) {
	int min_intensiteit=90;
	int max_i = 0, max_j = 0;
	Mat mask = imread(maskname + ext, CV_LOAD_IMAGE_GRAYSCALE);
	for (int i = 0; i < vvv.size(); i++) {
		if (vvv[i].size() > MIN_CONTOURS) {
			for (int j = 0; j < vvv[i].size(); j++) {
				if (mask.at<uchar>(vvv[i][j]) < min_intensiteit) {
					min_intensiteit = (int)mask.at<uchar>(vvv[i][j]);
					max_i = i;
					max_j = j;
				}
			}
		}	
	}
	return min_intensiteit;
}

int geef_maximum_intensiteit(string maskname, string ext) {
	Mat mask = imread(maskname + ext, CV_LOAD_IMAGE_GRAYSCALE);
	//imshow("Masker", mask);
	//cout << mask.type() << endl;
	//waitKey();
	int max_intensiteit=0;
	int max_i=0, max_j=0;
	//cout << "Intensiteit: " << (int)mask.at<uchar>(403, 567) << endl;
	for (int i = 0; i < mask.rows-1 ; i++) {
		for (int j = 0; j < mask.cols-1; j++) {
			if (mask.at<uchar>(i, j) > max_intensiteit) {
				max_intensiteit = (int)mask.at<uchar>(i, j);
				max_i = i;
				max_j = j;
				//cout << "i: " << i << "\tj: " << j << "\t Intensiteit: " << (int)mask.at<uchar>(i, j) << endl;
			}
		}
	}
	return max_intensiteit;
}

Mat no_white_planes(String filename, String ext, String maskname) {

	Mat mask = imread(maskname + ext, CV_LOAD_IMAGE_UNCHANGED);
	threshold(mask, mask, 1, 255, 0);

	Mat color, image2, blurred, gray;
	color = imread(filename + ext, CV_LOAD_IMAGE_ANYCOLOR);
	gray = imread(filename + ext, CV_LOAD_IMAGE_GRAYSCALE);
	//imshow("gray", gray);

	medianBlur(color, blurred, 221);
	//threshold(color, image2, 130, 255, THRESH_BINARY);
	threshold(gray, image2, 140, 255, THRESH_BINARY);
	//imshow("image2", image2);

	Mat grayC3;
	Mat input[] = { image2, image2, image2 };
	merge(input, 3, grayC3);

	Mat top = color - grayC3;

	grayC3 = grayC3 / 255;

	Mat mult;
	multiply(grayC3, blurred, mult);

	Mat end = mult + top;

	Mat final;
	GaussianBlur(end, final, Size(7, 7), 1);

	Mat canny;
	Canny(final, canny, 20, 140);
	//imshow("canny", canny);
	Mat cutout_image;
	//cout << endl << canny.type() << endl << mask.type();
	bitwise_and(canny, mask, cutout_image);
	return cutout_image;
}

bool goes_overboard(string maskname) {
	Mat mask = imread(maskname, CV_LOAD_IMAGE_UNCHANGED);
	mask.convertTo(mask, CV_32S);
	int aantal_rijen = mask.rows;
	bool goes_overboard = false;
	int i = 0;
	while (!goes_overboard && i < aantal_rijen - 1) {
		if (mask.at<int>(i, 0) != 0 || mask.at<int>(i, mask.cols - 1) != 0) {
			goes_overboard = true;
		}
		i++;
	}
	return goes_overboard;
};

double bepaal_rico_weg(Mat m, Point & p1, Point & p2) {
	bool rand;
	Point y1, y2, x1, x2;
	rand = false;
	bool y1found = false, y2found = false, x1found = false, x2found = false;
	m.convertTo(m, CV_32S);
	for (int i = 0; i < m.rows; i++) {
		if (m.at<int>(i, m.cols - 1) != 0 && !y1found) {//laatste kolom overlopen eerste coordinaat;
			y1found = true;
			y1 = Point(m.cols - 1, i);
			rand = true;
		}
		int teller = i;
		if (y1found) {
			while (!y2found && teller < m.rows) {

				if (m.at<int>(teller, m.cols - 1) == 0 && !y2found && y1found) { //laatste kolom overlopen tweede coordinaat;
					y2found = true;
					y2 = Point(m.cols - 1, teller);
				}
				teller++;
			}
			i = m.rows;
		}
	}

	if (!y1found && !y2found)
		for (int i = 0; i < m.rows; i++) {
			if (m.at<int>(i, 0) != 0 && !y1found) {//laatste kolom overlopen eerste coordinaat;
				y1found = true;
				y1 = Point(0, i);
				rand = true;
			}
			int teller = i;
			if (y1found) {
				while (!y2found && teller < m.rows) {

					if (m.at<int>(teller, 0) == 0 && !y2found && y1found) { //laatste kolom overlopen tweede coordinaat;
						y2found = true;
						y2 = Point(0, teller);
					}
					teller++;
				}
				i = m.rows;
			}
		}

	for (int i = 0; i<m.cols; i++) {
		if (m.at<int>(m.rows - 1, i) != 0 && !x1found) {//laatste rij overlopen eerste coordinaat;
			x1found = true;
			x1 = Point(i, m.rows - 1);
		}
		int teller = i;
		if (x1found) {
			while (!x2found) {

				if (m.at<int>(m.rows - 1, teller) == 0 && !x2found && x1found) { //laatste rij overlopen tweede coordinaat;
					x2found = true;
					x2 = Point(teller, m.rows - 1);
				}
				teller++;
			}
			i = m.cols;
		}
	}

	if (y1found && y2found && x1found && x2found) {
		Point bocht = Point(m.cols - 1, (y2.y + y1.y) / 2);
		Point begin = Point((x2.x + x1.x) / 2, m.rows - 1);
		double x = (double)(bocht.y - begin.y) / (double)(bocht.x - begin.x);
		return 1 / x;
	}
	else {
		int _col = 0, _row = 0;
		while (m.at<int>(_row, _col) == 0) {
			if (_col == m.cols) {
				_col = 0;
				_row++;
			}
			_col++;
		}
		p1 = Point(_col, _row);
		while (m.at<int>(_row, _col) != 0) {
			_col++;
		}
		if (_col == m.cols) { //return thesame pixel when only one on a line
			p2 = Point(p1);
			Point bocht = Point(p2.x, p2.y);
			Point begin = Point(x2.x - x1.x, m.rows - 1);
			return -(double)(bocht.y - begin.y) / (double)(bocht.x - begin.x);
		}
		else { // return other pixel on the row where the road ends

			p2 = Point(_col, _row);
			Point bocht = Point((p2.x + p1.x) / 2, p2.y);
			Point begin = Point(x2.x - x1.x, m.rows - 1);
			double ric = (double)(bocht.y - begin.y) / (double)(bocht.x - begin.x);
			return 1 / ric;

		}
	}
};

void print_enkel_mask(string filename, string maskname, string ext, string number, int oplossing) {
	/*if (!goes_overboard(maskname + ext)) {*/
		int THRESHOLD_90 = INITIAL_THRESHOLD_90;
		Mat image, mask;	//read image
		image = imread(filename + ext, CV_LOAD_IMAGE_COLOR);
		mask = imread(maskname + ext, CV_LOAD_IMAGE_COLOR);
		//imshow("original", image + mask);
		//Mat M = Mat::ones(30, 60, CV_32F);
		//dilate(mask, mask, M);
		/*if (number == "00033") {
			imshow("new mask", image + mask);
			waitKey();
		}*/

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
		//imshow("cutout", cutout_image);
		//waitKey();
		Mat contour;
		Canny(cutout_image, contour, 50, 150);
		//imshow("window", image);
		Mat cannied_masked_image = contour - contour_mask;
		//imshow("final image", cannied_masked_image);
		//waitKey();

		//double rico = bepaal_rico_weg(original_mask, Point(), Point());
		//rico = abs(1 / rico);
		//rico /= 0.35;
		//cout << "Gewicht: " << rico << endl;
		//imshow("rico", original_mask);
		//if (rico > 1) {
			//THRESHOLD_90 = THRESHOLD_90 * rico;
			//cout << "Gewicht: " << rico << endl;
		//}
		//cout << "Rico: " << THRESHOLD_90 << endl;
		//waitKey();
		//imshow("mask", mask);
		//waitKey();

		cannied_masked_image = no_white_planes(filename, ext, maskname);

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		//imshow("Final Image before", cannied_masked_image);
		findContours(cannied_masked_image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		Mat image_contours = Mat::zeros(cannied_masked_image.rows, cannied_masked_image.cols, CV_32F);
		Scalar color(255,255,255);
		bool found = false;
		for (int i = 0; i < contours.size();i++) {
			if (contours[i].size() > MIN_CONTOURS) {
				drawContours(image_contours, contours, i, color, 1, 8, hierarchy);
				found = true;
			}
		}
		int snelheid = 0;
		//imshow("original mask", original_mask);
		//waitKey();
		if (found) {
			//imshow("Ewouds canny", cannied_masked_image);
			//imshow("Ewouds canny contoured", image_contours);
			//imshow("Original", image + original_mask);
			//waitKey();
			snelheid = geef_minimum_van_vvv(contours, maskname, ext);
			//snelheid = snelheid - 5;
		}
		else {
			snelheid = geef_maximum_intensiteit(maskname, ext);
		}
		if (snelheid != 90) {
			snelheid -= 8;
		}
		/*if (snelheid < 30) {
			snelheid = 30;
		}*/
		//cout << filename << ":" << snelheid << "\t" << oplossing << endl;
		//cin.get();
		if (snelheid > oplossing) {
			//cout << "FOUT";
			if (marge < snelheid - oplossing) {
				marge = snelheid - oplossing;
			}
		}
		file << filename << ";" << snelheid - oplossing << ";" << snelheid << ";" << oplossing << endl;
		String hulp = number;
		hulp += "\t";
		hulp += to_string(snelheid);
		oplossingen.push_back(hulp);
		//Point minimum = geef_minimum_van_vvv(contours);
		//cout << "Type:" << original_mask.type();
		//cout << "x: " << minimum.x << "\ty: " << endl;
		//cout << filename << ":" << original_mask.at<uchar>(minimum) << endl;





		vector<Mat> channels;
		split(cannied_masked_image, channels);
		Scalar m = mean(channels[0]);
		double aantal_pixels = m[0] * cannied_masked_image.rows * cannied_masked_image.cols / 255;

		//imshow("Original", image + original_mask);
		//imshow("Final Image after", image_contours);
		//cout << "Aantal pixels: " << aantal_pixels << endl;
		//waitKey();

		//file << aantal_pixels << ";" << oplossing << ";";

		if (aantal_pixels <= THRESHOLD_90) { // WE SAY 90
			if (oplossing < OUR_SPEED_90) { // SOLUTION SAYS NOT 90
											//cout << "Oplossing : " << oplossing << " Verkeerd toegelaten\n";
											//cout << "Oplossing: " << oplossing << "\tAantal pixels: " << aantal_pixels << " File: " << filename << "Threshold: " << THRESHOLD_90 << endl;
				string windowname = filename;
				//imshow(filename, cannied_masked_image);
				false_positive_90++; // SHOULD BE ZERO OR ELSE WE CRASH

				waitKey();
			}
			else { //SOLUTION SAYS 90
				true_positive_90++;
			}
		}
		else { // WE SAY NOT 90
			if (oplossing >= OUR_SPEED_90) { // SOLUTION SAYS 90
											 //cout << "Oplossing: " << oplossing << " Verkeerd rejected\n";
				false_negative_90++;
				//waitKey();
			}
			else { //SOLUTION SAYS NOT 90
				true_negative_90++;
			}

			if (aantal_pixels < INITIAL_THRESHOLD_70) { // WE SAY 70
				if (oplossing >= 70) { // SOLUTION SAYS MORE THAN 70
					true_positive_70++;
				}
				else { // SOLUTION SAYS LESS THAN 70
					false_positive_70++;
				}
			}
			else { //WE SAY LESS THAN 70
				if (oplossing >= 70) { //SOLUTION SAYS MORE THAN 70
					false_negative_70++;
				}
				else { // SOLUTION SAYS LESS THEN 70
					true_negative_70++;
				}

			}

			// NEED TO BE EXAMINED FURTHER
		}
		//file << is90;
		//file << endl;
		//cout << "Rico: " << bepaal_rico_weg(mask) << endl;
	/*}
	else {
		cout << "went overboard" << endl;
	}*/
	
	
	
}

string format(int num, int length) {
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(length) << num;
	return oss.str();
};


int main()
{
	file.open("../../../numbers.csv");
	//file << "aantal_pixels;oplossing" << endl;
	file << "filename;marge;snelheid;oplossing" << endl;
	results.open("../../../results.txt");
	vector<thread> draad;
	for (int j = 3; j <= 3;j++) {
		string map_number = format(j,2);
		cout << "MAP: " << j << endl;
		string oplossing_file = "../../../images/";
		oplossing_file += map_number;
		oplossing_file += "/01";
		oplossing_file += "/gtdistances.txt";
		ifstream in;
		in.open(oplossing_file);

		string map = "../../../images/";
		map += map_number;
		map += "/01";
		int i = 0;
		string filename = map + "/frame" + format(i,5);
		Mat image = imread(filename + ".png");
		while (!image.empty()) {
			cout << i << " ";
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

	sort(oplossingen.begin(), oplossingen.end());
	for (int i = 0; i < oplossingen.size();i++) {
		results << oplossingen[i] << endl;
	}
	
	cout << endl;
	//cout << "True positive 90: " << true_positive_90 << " WE ARE CLASSIFYING THOSE WELL" << endl;
	//cout << "False positive 90: " << false_positive_90 << " SHOULD BE ZERO OR ELSE WE CRASH" << endl;
	//cout << "True negative 90: " << true_negative_90 << " WE WILL FURTHER EXAMINE THOSE" << endl;
	//cout << "False negative 90: " << false_negative_90 << " WE WILL FURTHER EXAMINE THOSE" << endl;
	//cout << "Total: " << true_positive_90 + true_negative_90 + false_negative_90 + false_positive_90 << endl;
	//cout << "True positive 70: " << true_positive_70 << " WE ARE CLASSIFYING THOSE WELL" << endl;
	//cout << "False positive 70: " << false_positive_70 << " SHOULD BE ZERO OR ELSE WE CRASH" << endl;
	//cout << "True negative 70: " << true_negative_70 << " WE WILL FURTHER EXAMINE THOSE" << endl;
	//cout << "False negative 70: " << false_negative_70 << " WE WILL FURTHER EXAMINE THOSE" << endl;
	//cout << "Total: " << true_positive_90 + true_negative_90 + false_negative_90 + false_positive_90 << endl;
	cout << "Marge: " << marge << endl;
	cin.get();
	
    return 0;
}



