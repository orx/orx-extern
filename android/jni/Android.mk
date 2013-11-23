LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE = orxAndroidSupport
LOCAL_SRC_FILES = orxAndroidSupport.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../code/include
LOCAL_CFLAGS := -D__orxANDROID__
LOCAL_ARM_MODE := arm
TARGET_PLATFORM = android-9

include $(BUILD_STATIC_LIBRARY)

