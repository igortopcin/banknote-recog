#include <jni.h>
#include <stdio.h>
#include <android/log.h>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

void matchImage(const Mat& queryImage_, string& tag, const bool saveResults=false);
void initTrainSamples(const string& dir);

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

// convert a string to a jstring
void getJStringContent(JNIEnv *aEnv, jstring aStr, std::string &aRes) {
	if (!aStr) {
		aRes.clear();
		return;
	}

	const char *s = aEnv->GetStringUTFChars(aStr, NULL);
	aRes = s;
	aEnv->ReleaseStringUTFChars(aStr, s);
}

extern "C" JNIEXPORT void JNICALL Java_br_usp_ime_banknote_MainActivity_nInitTrainSamples(
		JNIEnv * env, jobject, jstring descriptors_dir) {
	string descriptors_dir_str;
	getJStringContent(env, descriptors_dir, descriptors_dir_str);
	__android_log_write(ANDROID_LOG_INFO, "banknote_recog.jni", "Initializing dir::");
	__android_log_write(ANDROID_LOG_INFO, "banknote_recog.jni", descriptors_dir_str.c_str());

	initTrainSamples(descriptors_dir_str);
}

extern "C" JNIEXPORT jstring JNICALL Java_br_usp_ime_banknote_MainActivity_nMatchImage(
		JNIEnv * env, jobject, jlong addr) {
	Mat& img = *(Mat *) addr;
	__android_log_write(ANDROID_LOG_INFO, "banknote_recog.jni", "Test image type::");
	__android_log_write(ANDROID_LOG_INFO, "banknote_recog.jni", getImgType(img.type()).c_str());

	string tag;
	matchImage(img, tag);
	__android_log_write(ANDROID_LOG_INFO, "banknote_recog.jni", "Returned::");
	__android_log_write(ANDROID_LOG_INFO, "banknote_recog.jni", tag.c_str());
	//return tag;
	return env->NewStringUTF(tag.c_str());
}
