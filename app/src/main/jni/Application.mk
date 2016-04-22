APP_PLATFORM := android-9
APP_CFLAGS += -fexceptions
APP_STL := gnustl_static

# C++11 and threading enabling features.
#  Otherwise c++11, pthread, rtti and exceptions are not enabled by default
LOCAL_CPPFLAGS := -std=c++11 -pthread -frtti -fexceptions