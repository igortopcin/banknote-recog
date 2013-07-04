/**
 * @file NoteMatcher
 * @brief Matches banknotes by using Sift detector + descriptor + FLANN Matcher
 * @author Igor Topcin
 * @author Larissa Sartori
 */
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

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

vector<Mat> TrainImages;
vector<vector<KeyPoint> > TrainKeypoints;
vector<Mat> TrainDescriptors;
vector<string> Tags;

SiftFeatureDetector Detector(MIN_HESSIAN);
//SiftDescriptorExtractor Extractor;
Ptr<DescriptorExtractor> Extractor(
		new OpponentColorDescriptorExtractor(
				Ptr<DescriptorExtractor>(new SiftDescriptorExtractor())));

void loadFile(const string& filename, vector<KeyPoint>& keypoints, Mat& descriptors, Mat& image, string& tag) {
	FileStorage fs(filename, FileStorage::READ);
	fs["tag"] >> tag;
	read(fs["image"], image);
	read(fs["keypoints"], keypoints);
	read(fs["descriptors"], descriptors);
	fs.release();
}

void highlightMatchedObject(const Mat& img_1,
		const vector<Point2f>& img_points_1, Mat& img_2,
		const vector<Point2f>& img_points_2, const Mat& Homography) {
	//-- Get the corners from the image_1 ( the object to be "detected" )
	vector<Point2f> img_corners_1(4);
	img_corners_1[0] = cvPoint(0, 0);
	img_corners_1[1] = cvPoint(img_1.cols, 0);
	img_corners_1[2] = cvPoint(img_1.cols, img_1.rows);
	img_corners_1[3] = cvPoint(0, img_1.rows);

	vector<Point2f> img_corners_2(4);
	perspectiveTransform(img_corners_1, img_corners_2, Homography);

	printf("----\n");
	for (int i = 0; i < (int) img_corners_2.size(); i++) {
		printf("-- Corner [%d] x: %f, y: %f  \n", i, img_corners_2[i].x,
				img_corners_2[i].y);
	}

	double diagonal = sqrt(
			pow(img_corners_2[1].x - img_corners_2[3].x, 2)
					+ pow(img_corners_2[1].y - img_corners_2[3].y, 2));
	printf("Diagonal do retângulo: %f\n", diagonal);

	//-- Draw lines between the corners (the mapped object in the scene - image_2 )
	line(img_2, img_corners_2[0], img_corners_2[1], Scalar(0, 255, 0), 4);
	line(img_2, img_corners_2[1], img_corners_2[2], Scalar(0, 255, 0), 4);
	line(img_2, img_corners_2[2], img_corners_2[3], Scalar(0, 255, 0), 4);
	line(img_2, img_corners_2[3], img_corners_2[0], Scalar(0, 255, 0), 4);
}

void initTrainSamples(const string& dir) {
	TickMeter tm;
	tm.start();

	string filepath;
	string tag;
	Mat image;
	vector<KeyPoint> keypoints;
	Mat descriptors;

	DIR *dp;
	struct dirent *dirp;
	struct stat filestat;

	cout << "load train samples from yml" << endl;
	dp = opendir(dir.c_str());

	while ((dirp = readdir(dp))) {
		filepath = dir + "/" + dirp->d_name;

		// If the file is a directory (or is in some way invalid) we'll skip it
		if (stat(filepath.c_str(), &filestat))
			continue;
		if (S_ISDIR( filestat.st_mode ))
			continue;
		if (filepath.find("trained_sample_") != string::npos) {
			cout << "..load " << filepath << endl;
			loadFile(filepath, keypoints, descriptors, image, tag);
			TrainKeypoints.push_back(keypoints);
			TrainDescriptors.push_back(descriptors);
			TrainImages.push_back(image);
			Tags.push_back(tag);
		}
	}
	closedir(dp);

	tm.stop();
	cout << "..load time: " << tm.getTimeMilli() << " ms" << endl;
}

void matchDescriptors(const Mat& queryDescriptors, vector<DMatch>& matches, string& tag) {
	FlannBasedMatcher descriptorMatcher;
	TickMeter tm;

	tm.start();
	cout << "train descriptors" << endl;
	descriptorMatcher.add(TrainDescriptors);
	descriptorMatcher.train();
	tm.stop();
	double buildTime = tm.getTimeMilli();

	tm.start();
	cout << "train descriptors" << endl;
	descriptorMatcher.match(queryDescriptors, matches);
	CV_Assert(queryDescriptors.rows == (int)matches.size() || matches.empty());

	vector<int> total_matches(Tags.size(), 0);
	vector<double> max_dist(Tags.size(), 0);
	vector<double> min_dist(Tags.size(), 1000);
	vector<double> scores(Tags.size(), 0);
	double min_dist_global = 1000;

	for (int i = 0; i < matches.size(); i++) {
		int imgIdx = matches[i].imgIdx;
		total_matches[imgIdx]++;

		double dist = matches[i].distance;
		if (dist < min_dist[imgIdx])
			min_dist[imgIdx] = dist;
		if (dist > max_dist[imgIdx])
			max_dist[imgIdx] = dist;
		if (dist < min_dist_global)
			min_dist_global = dist;
	}

	int max_score_idx = 0;
	for (int i = 0; i < scores.size(); i++) {
		scores[i] = (min_dist_global / min_dist[i]) * total_matches[i];
		if (scores[i] > scores[max_score_idx])
			max_score_idx = i;
	}

	tag = Tags[max_score_idx];

	for (int i = 0; i < Tags.size(); i++) {
		cout << "..image " << Tags[i]
		        << " matches:" << total_matches[i]
				<< " min dist:" << min_dist[i]
				<< " max dist:" << max_dist[i]
				<< " score:" << scores[i] << endl;
	}

	tm.stop();
	double matchTime = tm.getTimeMilli();

	cout << "..number of matches: " << matches.size() << endl;
	cout << "..build time: " << buildTime << " ms; match time: " << matchTime << " ms" << endl;
}

int maskMatchesByTrainImgIdx(const vector<DMatch>& matches,
		int trainImgIdx, vector<char>& mask) {
	int totalMatches = 0;
	mask.resize(matches.size());
	fill(mask.begin(), mask.end(), 0);
	for (size_t i = 0; i < matches.size(); i++) {
		if (matches[i].imgIdx == trainImgIdx) {
			mask[i] = 1;
			totalMatches++;
		}
	}
	return totalMatches;
}

double avgDistanceByTrainImgIdx(const vector<DMatch>& matches,
		int trainImgIdx) {
	double avg = 0;
	int totalMatches = 0;
	for (size_t i = 0; i < matches.size(); i++) {
		if (matches[i].imgIdx == trainImgIdx) {
			avg += matches[i].distance;
			totalMatches++;
		}
	}
	avg /= totalMatches;
	return avg;
}

void saveResultImages(const Mat& queryImage,
		const vector<KeyPoint>& queryKeypoints,
		const vector<DMatch>& matches,
		const string& resultDir) {
	cout << "< Save results..." << endl;
	Mat drawImg;
	vector<char> mask;
	int totalMatches = 0;
	for (size_t i = 0; i < TrainImages.size(); i++) {
		if (!TrainImages[i].empty()) {
			totalMatches = maskMatchesByTrainImgIdx(matches, (int) i, mask);
			cout << "Image " << Tags[i] << ": " << totalMatches << " matches. Avg: " << avgDistanceByTrainImgIdx(matches, (int) i) << endl;
			drawMatches(queryImage, queryKeypoints, TrainImages[i],
					TrainKeypoints[i], matches, drawImg, Scalar(255, 0, 0),
					Scalar(0, 255, 255), mask);
			string filename = resultDir + "/res_" + Tags[i] + ".jpg";
			if (!imwrite(filename, drawImg))
				cout << "Image " << filename
						<< " can not be saved (may be because directory "
						<< resultDir << " does not exist)." << endl;
		}
	}
	cout << ">" << endl;
}

void extractTrainSamples(const string& dir, const string& trainingFilename, vector<Mat>& trainImages, vector<string>& tags) {
	cout << "look in train data"<<endl;
	Ptr<ifstream> ifs(new ifstream(trainingFilename.c_str()));

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
			cout << "train image " << dir << "/" << filepath << " does not have a tag..." << endl;
			continue;
		}

		cout << "reading: " << dir << "/" << filepath << endl;
		Mat img = imread(dir + "/" + filepath, CV_LOAD_IMAGE_COLOR);
		//Mat img = imread(filepath, CV_LOAD_IMAGE_GRAYSCALE);
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
	SiftFeatureDetector detector(MIN_HESSIAN);
	detector.detect(trainImages, trainKeypoints);

	cout << "computing descriptors for keypoints..." << endl;
//	SiftDescriptorExtractor extractor;
//	extractor.compute(trainImages, trainKeypoints, trainDescriptors);
	Ptr<DescriptorExtractor> extractor(
			new OpponentColorDescriptorExtractor(
					Ptr<DescriptorExtractor>(new SiftDescriptorExtractor())));
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

int extract(const string& dir) {
	vector<Mat> trainImages;
	vector<string> tags;

	cout << "-------- training images -----------" << endl;

	cout << "extract training samples..." << endl;
	extractTrainSamples(dir, dir+"/classtraining.txt", trainImages, tags);
	cout << "got " << trainImages.size() << " images." <<endl;

	cout << "computing and saving samples data..." << endl;
	computeSamples(dir, trainImages, tags);

	return 0;
}

void matchImage(const Mat& queryImage_, string& tag, const bool saveResults=false) {
	if (!queryImage_.data) {
		std::cout << " --(!) Error reading image " << std::endl;
		return;
	}

	TickMeter tm;
	tm.start();

	Mat queryImage;
	double ratio = 600.0/queryImage_.cols;
	resize(queryImage_, queryImage, Size(), ratio, ratio);

	vector<KeyPoint> queryKeypoints;
	Detector.detect(queryImage, queryKeypoints);

	Mat queryDescriptors;
	Extractor->compute(queryImage, queryKeypoints, queryDescriptors);
	//Extractor.compute(queryImage, queryKeypoints, queryDescriptors);

	tm.stop();
	cout << "..queryImage compute time: " << tm.getTimeMilli() << " ms" << endl;

	vector<DMatch> matches;
	matchDescriptors(queryDescriptors, matches, tag);

	if (saveResults)
		saveResultImages(queryImage, queryKeypoints, matches, string("./results"));
}
