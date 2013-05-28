/**
 * @file SURF_FlannMatcher
 * @brief SURF detector + descriptor + FLANN Matcher
 * @author A. Huaman
 */

#include <stdio.h>
#include <iostream>
#include <set>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"

using namespace std;
using namespace cv;

const int MIN_HESSIAN = 400;

void readme();

static void saveDescriptors(const string& filename, const vector<KeyPoint>& keypoints, const Mat& descriptors) {
	FileStorage fs(filename, FileStorage::WRITE);
	write(fs, "keypoints", keypoints);
	fs << "descriptors" << descriptors;
	fs.release();
}

static void loadDescriptors(const string& filename, vector<KeyPoint>& keypoints, Mat& descriptors) {
	FileStorage fs(filename, FileStorage::READ);
	read(fs["keypoints"], keypoints);
	fs["descriptors"] >> descriptors;
	fs.release();
}

static void computeDescriptors(
		const Mat& img,
		const FeatureDetector& detector,
		const DescriptorExtractor& extractor,
		vector<KeyPoint>& keypoints,
		Mat& descriptors) {
	detector.detect(img, keypoints);
	extractor.compute(img, keypoints, descriptors);
}

static void findGoodMatches(
		const vector<KeyPoint>& keypoints_1,
		const Mat& descriptors_1,
		const vector<KeyPoint>& keypoints_2,
		const Mat& descriptors_2,
		vector<DMatch>& good_matches) {
	//-- Matching descriptor vectors using FLANN matcher
	FlannBasedMatcher matcher;
	vector<DMatch> matches;

	matcher.match(descriptors_1, descriptors_2, matches);

	double max_dist = 0;
	double min_dist = 100;

	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < descriptors_1.rows; i++) {
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	printf("-- Max dist : %f \n", max_dist);
	printf("-- Min dist : %f \n", min_dist);

	//-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
	//-- PS.- radiusMatch can also be used here.
	for (int i = 0; i < descriptors_1.rows; i++) {
		if (matches[i].distance < 3 * min_dist) {
			good_matches.push_back(matches[i]);
		}
	}

	for (int i = 0; i < (int) good_matches.size(); i++) {
		printf("-- Good Match [%d] Keypoint 1: %d  -- Keypoint 2: %d  \n", i,
				good_matches[i].queryIdx, good_matches[i].trainIdx);
	}
}

static void convertMatchesToPoints(
		const vector<DMatch>& good_matches,
		const vector<KeyPoint>& keypoints_1,
		const vector<KeyPoint>& keypoints_2,
		vector<Point2f>& img_points_1,
		vector<Point2f>& img_points_2) {
	for (int i = 0; i < (int) good_matches.size(); i++) {
		//-- Get the keypoints from the good matches
		img_points_1.push_back(keypoints_1[good_matches[i].queryIdx].pt);
		img_points_2.push_back(keypoints_2[good_matches[i].trainIdx].pt);
	}
}

static void drawRelativeMatches(
		const Mat& img_1,
		const vector<Point2f>& img_points_1,
		const Mat& img_2,
		const vector<Point2f>& img_points_2,
		const Mat& Homography,
		const vector<unsigned char>& match_mask,
		Mat& img_matches) {
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
		printf("-- Corner [%d] x: %f, y: %f  \n", i,
				img_corners_2[i].x, img_corners_2[i].y);
	}

	//-- Draw lines between the corners (the mapped object in the scene - image_2 )
	line(img_matches, img_corners_2[0] + Point2f(img_1.cols, 0),
			img_corners_2[1] + Point2f(img_1.cols, 0), Scalar(0, 255, 0), 4);
	line(img_matches, img_corners_2[1] + Point2f(img_1.cols, 0),
			img_corners_2[2] + Point2f(img_1.cols, 0), Scalar(0, 255, 0), 4);
	line(img_matches, img_corners_2[2] + Point2f(img_1.cols, 0),
			img_corners_2[3] + Point2f(img_1.cols, 0), Scalar(0, 255, 0), 4);
	line(img_matches, img_corners_2[3] + Point2f(img_1.cols, 0),
			img_corners_2[0] + Point2f(img_1.cols, 0), Scalar(0, 255, 0), 4);

	//-- Draw points that matched the homography
	for (int i = 0; i < (int) img_points_1.size(); i++) {
		if (match_mask.empty() || match_mask[i]) {
			line(img_matches, img_points_1[i], img_points_2[i] + Point2f(img_1.cols, 0), Scalar(0, 0, 255), 4);
			circle(img_matches, img_points_2[i] + Point2f(img_1.cols, 0), 4, Scalar(0, 0, 255), 4);
		}
	}
}


/**
 * @function main
 * @brief Main function
 */
int main(int argc, char** argv) {
	if (argc != 3) {
		readme();
		return -1;
	}

	Mat img_1 = imread(argv[1], IMREAD_GRAYSCALE);
	Mat img_2 = imread(argv[2], IMREAD_GRAYSCALE);

	if (!img_1.data || !img_2.data) {
		std::cout << " --(!) Error reading images " << std::endl;
		return -1;
	}

	//-- Step 1: Detect the keypoints using SURF Detector and calculate descriptors (feature vectors)
	SurfFeatureDetector detector(MIN_HESSIAN);
	SurfDescriptorExtractor extractor;

	vector<KeyPoint> keypoints_1, keypoints_2;
	Mat descriptors_1, descriptors_2;

	computeDescriptors(img_1, detector, extractor, keypoints_1, descriptors_1);
	//saveDescriptors(argv[1] + string(".xml"), keypoints_1, descriptors_1);
	//loadDescriptors(argv[1] + string(".xml"), keypoints_1, descriptors_1);
	computeDescriptors(img_2, detector, extractor, keypoints_2, descriptors_2);

	//-- Step 2: Matching descriptor vectors using FLANN matcher
	vector<DMatch> good_matches;
	findGoodMatches(keypoints_1, descriptors_1, keypoints_2, descriptors_2, good_matches);

	//-- Localize the object
	vector<Point2f> img_points_1, img_points_2;
	convertMatchesToPoints(good_matches, keypoints_1, keypoints_2, img_points_1, img_points_2);

	//-- Find the homography and match mask
	vector<unsigned char> match_mask;
	Mat H = findHomography(img_points_1, img_points_2, match_mask, CV_RANSAC);

	std::vector<Point2f> img_unique_points_2;
	for (int i = 0; i < (int) img_points_2.size(); i++) {
		if (match_mask.empty() || match_mask[i]) {
			printf("-- Point x: %d, y: %d  \n", (int)img_points_2[i].x, (int)img_points_2[i].y);
			if (img_unique_points_2.empty() || img_unique_points_2.back() != img_points_2[i]) {
				img_unique_points_2.push_back(img_points_2[i]);
			}
		}
	}

	printf("-- Homography match mask count: %d\n", countNonZero(Mat(match_mask)));
	printf("-- Homography unique match points count: %d\n", img_unique_points_2.size());

	//-- Draw only "good" matches
	Mat img_matches;
	drawMatches(img_1, keypoints_1, img_2, keypoints_2, good_matches,
			img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(),
			DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	if (img_unique_points_2.size() > 15) {
		drawRelativeMatches(img_1, img_points_1, img_2, img_points_2, H, match_mask, img_matches);
	}

	//-- Show detected matches
	imshow("Good Matches & Object detection", img_matches);

	waitKey(0);

	return 0;
}

/**
 * @function readme
 */
void readme() {
	std::cout << " Usage: ./SURF_FlannMatcher <img1> <img2>" << std::endl;
}
