LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
OPENCV_CAMERA_MODULES	:= on
OPENCV_INSTALL_MODULES	:= on
include $(OPENCV_ANDROID_SDK_HOME)/sdk/native/jni/OpenCV.mk

LOCAL_MODULE		:= banknote
LOCAL_SRC_FILES		:= nonfree_init.cpp precomp.cpp sift.cpp surf.cpp matcher.cpp matcher_jni.cpp
LOCAL_C_INCLUDES	+= $(LOCAL_PATH)
LOCAL_LDFLAGS		+= -llog -ldl 

include $(BUILD_SHARED_LIBRARY)
