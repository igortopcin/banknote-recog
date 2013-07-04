#include <stdio.h>
#include <iostream>
#include <set>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

using namespace std;
using namespace cv;

void matchImage(const Mat& queryImage_, string& tag, const bool saveResults=false);
int extract(const string& dir);
void initTrainSamples(const string& dir);

int main(int argc, char** argv_) {
	vector<string> argv;for (int i = 0; i < argc; i++) {argv.push_back(argv_[i]);}

	if (argv.size() < 3) {
		cout << "USAGE: matcher <training_xml_dir> <queryimage.jpg>"<<endl;
		return 1;
	}

	Mat img;
	char c;
	string tag;

	//extract(argv[1]);
	initTrainSamples(argv[1]);

	//img = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);
	img = imread(argv[2], CV_LOAD_IMAGE_COLOR);
	matchImage(img, tag, true);
	cout << "Result: " << tag << endl;

	c = cvWaitKey(0);
	if (c == 27)
		return 0;
}

//int main(int argc, char *argv[]) {
//	printf("Press ESC to end capture");
//
//	Mat frame;
//	VideoCapture cap;
//	if (argc > 1) {
//		cap = VideoCapture(argv[1]);
//	} else {
//		cap = VideoCapture(0);
//	}
//
//	if (!cap.isOpened()) {
//		return -1;
//	}
//
//	namedWindow("Video", 1);
//	while (1) {
//		cap >> frame;
//		ColorSegment(frame);
//		imshow("Video", frame);
//		char c = cvWaitKey(33);
//		if (c == 27)
//			break;
//	}
//
//	return 0;
//}

