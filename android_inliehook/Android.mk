LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := hello
LOCAL_SRC_FILES := hello.cpp Trampline.S Hook.cpp
include $(BUILD_EXECUTABLE)
