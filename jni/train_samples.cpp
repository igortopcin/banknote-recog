#include <stdio.h>
#include <fstream>
#include <iostream>
#include <set>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

const int MIN_HESSIAN = 400;

// take number image type number (from cv::Mat.type()), get OpenCV's enum string.
string getImgType(int imgTypeInt) {
	int numImgTypes = 35; // 7 base types, with five channel options each (none or C1, ..., C4)

	int enum_ints[] = { CV_8U, CV_8UC1, CV_8UC2, CV_8UC3, CV_8UC4, CV_8S,
			CV_8SC1, CV_8SC2, CV_8SC3, CV_8SC4, CV_16U, CV_16UC1, CV_16UC2,
			CV_16UC3, CV_16UC4, CV_16S, CV_16SC1, CV_16SC2, CV_16SC3, CV_16SC4,
			CV_32S, CV_32SC1, CV_32SC2, CV_32SC3, CV_32SC4, CV_32F, CV_32FC1,
			CV_32FC2, CV_32FC3, CV_32FC4, CV_64F, CV_64FC1, CV_64FC2, CV_64FC3,
			CV_64FC4 };

	string enum_strings[] = { "CV_8U", "CV_8UC1", "CV_8UC2", "CV_8UC3",
			"CV_8UC4", "CV_8S", "CV_8SC1", "CV_8SC2", "CV_8SC3", "CV_8SC4",
			"CV_16U", "CV_16UC1", "CV_16UC2", "CV_16UC3", "CV_16UC4", "CV_16S",
			"CV_16SC1", "CV_16SC2", "CV_16SC3", "CV_16SC4", "CV_32S",
			"CV_32SC1", "CV_32SC2", "CV_32SC3", "CV_32SC4", "CV_32F",
			"CV_32FC1", "CV_32FC2", "CV_32FC3", "CV_32FC4", "CV_64F",
			"CV_64FC1", "CV_64FC2", "CV_64FC3", "CV_64FC4" };

	for (int i = 0; i < numImgTypes; i++) {
		if (imgTypeInt == enum_ints[i])
			return enum_strings[i];
	}
	return "unknown image type";
}

void extractTrainSamples(const string& dir, vector<Mat>& trainImages, vector<string>& tags) {
	cout << "look in train data"<<endl;
	Ptr<ifstream> ifs(new ifstream((dir + "/classtraining.txt").c_str()));

	char buf[255]; int count = 0;
	vector<string> lines;
	while(!ifs->eof()) {
		ifs->getline(buf, 255);
		lines.push_back(buf);
	}

	for(int i=0;i<lines.size();i++) {
		string filepath;
		string tag;

		string line(lines[i]);
		istringstream iss(line);

		iss >> filepath;
		iss >> tag;

		if (tag.size() == 0) {
			cout << "train image " << filepath << " does not have a tag..." << endl;
			continue;
		}

		cout << "reading: " << filepath << endl;
		Mat img = imread(dir + "/" + filepath, CV_LOAD_IMAGE_COLOR);
		cout << "..image type: " << getImgType(img.type()) << endl;
		cout << "..image depth: " << img.depth() << endl;
		cout << "..image channels: " << img.channels() << endl;
		if (img.empty()) {
			cout << "train image " << filepath << " cannot be read." << endl;
		}

		trainImages.push_back(img);
		tags.push_back(tag);
	}
}

void computeSamples(const string& outputDir, const vector<Mat>& trainImages, const vector<string>& tags) {
	vector<vector<KeyPoint> > trainKeypoints;
	vector<Mat> trainDescriptors;

	cout << "extracting keypoints from images..." << endl;
	OrbFeatureDetector detector(800);
	//SiftFeatureDetector detector(MIN_HESSIAN, 3, 0.04, 10, 1.6);
	//FastFeatureDetector detector(1, true, FastFeatureDetector::TYPE_9_16);
	detector.detect(trainImages, trainKeypoints);

	cout << "computing descriptors for keypoints..." << endl;
	//SiftDescriptorExtractor extractor;
	//extractor.compute(trainImages, trainKeypoints, trainDescriptors);
	//	Ptr<DescriptorExtractor> extractor(
	//			new OpponentColorDescriptorExtractor(
	//					Ptr<DescriptorExtractor>(new SiftDescriptorExtractor(MIN_HESSIAN, 3, 0.04, 10, 1.6))));
		Ptr<DescriptorExtractor> extractor(
				new OpponentColorDescriptorExtractor(
						Ptr<DescriptorExtractor>(new OrbDescriptorExtractor(800))));

	extractor->compute(trainImages, trainKeypoints, trainDescriptors);

	int totalTrainDesc = 0;
	for (vector<Mat>::const_iterator tdIter = trainDescriptors.begin();
			tdIter != trainDescriptors.end(); tdIter++)
		totalTrainDesc += tdIter->rows;
	cout << "total train descriptors count: " << totalTrainDesc << endl;

	for (int i = 0; i < trainDescriptors.size(); i++) {
		FileStorage fs(outputDir + "/trained_sample_" + tags[i] + ".yml", FileStorage::WRITE);
		write(fs, "tag", tags[i]);
		write(fs, "image", trainImages[i]);
		write(fs, "keypoints", trainKeypoints[i]);
		write(fs, "descriptors", trainDescriptors[i]);
		fs.release();
	}
}

int main(int argc, char** argv_) {
	vector<string> argv;for (int i = 0; i < argc; i++) {argv.push_back(argv_[i]);}
	
	if (argv.size() < 2) {
		cout << "USAGE: train <output_dir>"<<endl;
		return 1;
	}
	
	vector<Mat> trainImages;
	vector<string> tags;

	cout << "-------- training images -----------" << endl;
	
	cout << "extract training samples..." << endl;
	extractTrainSamples(argv[1], trainImages, tags);
	cout << "got " << trainImages.size() << " images." <<endl;
	
	cout << "computing and saving samples data..." << endl;
	computeSamples(argv[1], trainImages, tags);

	return 0;
}
