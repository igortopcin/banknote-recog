#include <jni.h>
#include <android/log.h>
#include "opencv2/opencv.hpp"

using namespace cv;

void matchImage(const Mat& queryImage_, string& tag, const bool saveResults=false);
void initTrainSamples(const string& dir);

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
	cvtColor(img , img , CV_RGBA2RGB);

	string tag;
	matchImage(img, tag);
	__android_log_write(ANDROID_LOG_INFO, "banknote_recog.jni", "Returned::");
	__android_log_write(ANDROID_LOG_INFO, "banknote_recog.jni", tag.c_str());
	//return tag;
	return env->NewStringUTF(tag.c_str());
}
