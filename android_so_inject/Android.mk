LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := hello
LOCAL_SRC_FILES := hello.cpp 
LOCAL_LDFLAGS += -Wl,-rpath=/data/local/tmp
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_MODULE := inject
LOCAL_SRC_FILES := inject.cpp 
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := test
LOCAL_SRC_FILES := test.cpp 
include $(BUILD_SHARED_LIBRARY)

