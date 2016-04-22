LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= substrate-dvm
LOCAL_SRC_FILES := libsubstrate-dvm.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE:= substrate
LOCAL_SRC_FILES := libsubstrate.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)


LOCAL_MODULE    := FileHook.cy
LOCAL_SRC_FILES := file/Hxms.cpp
LOCAL_LDLIBS := -landroid -latomic -llog
LOCAL_LDLIBS += -L$(LOCAL_PATH) -lsubstrate-dvm -lsubstrate
LOCAL_DISABLE_FORMAT_STRING_CHECKS := true

APP_STL := gnustl_static
LOCAL_CPPFLAGS := -std=c++11 -pthread -frtti -fexceptions

include $(BUILD_SHARED_LIBRARY)

