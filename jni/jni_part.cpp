#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>
#include <unistd.h>

#include <android/log.h>
#include <android/bitmap.h>

#include "ec-plates/algorithm.h"

using namespace std;
using namespace cv;

#define LOG_TAG "co.mwater.opencvactivity"
#ifdef ANDROID
#include <android/log.h>
#  define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#  define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#include <stdio.h>
#  define QUOTEME_(x) #x
#  define QUOTEME(x) QUOTEME_(x)
#  define LOGI(...) {printf(__VA_ARGS__); printf("\r\n");}
#  define LOGE(...) printf("E/" LOG_TAG "(" ")" __VA_ARGS__)
#endif

/*
 * Finds the affine transform to display the image within the screen
 */
static Mat getScreenTransform(Size image, Size screen) {
	Point2f srcTri[3];
	Point2f dstTri[3];

	// Set 3 points to calculate the Affine Transform
	srcTri[0] = Point2f(0, 0);
	srcTri[1] = Point2f(image.width - 1, 0);
	srcTri[2] = Point2f(0, image.height - 1);

	// Calculate scale
	double scaleSame = min(screen.width * 1.0 / image.width, screen.height * 1.0 / image.height);
	double scaleRt90 = min(screen.height * 1.0 / image.width, screen.width * 1.0 / image.height);

	if (scaleSame > scaleRt90) {
		dstTri[0] = Point2f(0, 0);
		dstTri[1] = srcTri[1] * scaleSame;
		dstTri[2] = srcTri[2] * scaleSame;
	}
	else {
		dstTri[0] = Point2f(image.height - 1, 0) * scaleRt90;
		dstTri[1] = Point2f(image.height - 1, image.width - 1) * scaleRt90;
		dstTri[2] = Point2f(0, 0);
	}

	return getAffineTransform(srcTri, dstTri);
}

/*
 * Context to run the open CV process within an android activity
 */
class AndroidOpenCVActivityContext : public OpenCVActivityContext {
public:
	AndroidOpenCVActivityContext(JNIEnv* env, jobject activity, jobjectArray params, jobject screen_bitmap) :
		screen_bitmap(screen_bitmap), params(params), activity(activity) {
		this->env = env;

		int ret;

		if ((ret = AndroidBitmap_getInfo(env, screen_bitmap, &this->screen_bitmap_info)) < 0) {
			LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		    return;
		}

		screenBGR = new Mat(Size(screen_bitmap_info.width, screen_bitmap_info.height), CV_8UC3, Scalar(0,0,0));

	}

	~AndroidOpenCVActivityContext() {
	}

	string getParam(int n) {
		jobject elem = env->GetObjectArrayElement(params, n);
		const char *s = env->GetStringUTFChars((jstring)elem, NULL);

		string str = s;
		env->ReleaseStringUTFChars((jstring)elem, s);

		return str;
	}

	int getParamCount() {
		return env->GetArrayLength(params);
	}

	void setReturnValue(string val) {
		returnValue = val;
	}

	void updateScreen(Mat& screen) {
		// Fit to screen

		// Create affine transform for screen
		Mat screenMat = getScreenTransform(screen.size(), screenBGR->size());

		// Create BGR screen
		warpAffine(screen, *screenBGR, screenMat, screenBGR->size());

		// Lock pixels
		void*              screen_bitmap_pixels;
		int ret;
		if ((ret = AndroidBitmap_lockPixels(env, screen_bitmap, &screen_bitmap_pixels)) < 0) {
			LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
		}

		// Create Mat of actual screen (ARGB)
		Mat screenARGB = Mat(
				Size(this->screen_bitmap_info.width, this->screen_bitmap_info.height),
				CV_8UC4, screen_bitmap_pixels);// , screen_bitmap_info.stride);

		// Convert format
		cvtColor(*screenBGR, screenARGB, CV_BGR2RGBA);

		// Unlock pixels
		LOGI("unlocking pixels");
		AndroidBitmap_unlockPixels(env, screen_bitmap);

		// Notify
		jclass cls = env->FindClass("co/mwater/opencvactivity/OpenCVActivity");
		jmethodID mth = env->GetMethodID(cls, "updateScreen", "()V");
		env->CallVoidMethod(activity, mth);
	}

	void log(string msg) {
		LOGI("%s", msg.c_str());
	}


	bool isAborted() {
		jclass cls = env->FindClass("co/mwater/opencvactivity/OpenCVActivity");
		jfieldID field = env->GetFieldID(cls, "aborted", "Z");
		return env->GetBooleanField(activity, field);
	}

	string returnValue;

private:
	Ptr<Mat> screenBGR;
	jobject screen_bitmap;
	jobjectArray params;
	JNIEnv* env;
	jobject activity;
	AndroidBitmapInfo  screen_bitmap_info;
};

void demoAlgo(OpenCVActivityContext& context) {
	Mat screen(300, 300, CV_8UC3);

	putText(screen, "Processing...", Point(30,30), FONT_HERSHEY_PLAIN,
			2, Scalar(0, 255, 0, 255));

	// Animate circle
	for (int i=0;i<100;i++)
	{
		circle(screen, Point(100+i, 100), 20, Scalar(255, 0, 0, 255), 5);
		context.updateScreen(screen);
		usleep(100000);
		LOGI("Iteration %d", i);
	}
	LOGI("Called with %s", context.getParam(0).c_str());

	context.setReturnValue("{\"test\":5}");
}

extern "C" {

JNIEXPORT jstring JNICALL Java_co_mwater_opencvactivity_OpenCVActivity_runProcess(JNIEnv* env, jobject activity, jstring id, jobjectArray params, jobject screen_bitmap) {
	AndroidOpenCVActivityContext context(env, activity, params, screen_bitmap);

	const char* utf_id = env->GetStringUTFChars(id, NULL);

	if (strcmp(utf_id, "demo") == 0)
		demoAlgo(context);
	if (strcmp(utf_id, "ec-plate") == 0)
		analyseECPlate(context);

	env->ReleaseStringUTFChars(id, utf_id);

	return env->NewStringUTF(context.returnValue.c_str());
}
}
