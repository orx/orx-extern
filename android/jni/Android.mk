LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := OrxLib
LOCAL_SRC_FILES = apk_file.c file.c thread.c

LOCAL_ARM_MODE   := arm
TARGET_PLATFORM = android-8

include $(BUILD_STATIC_LIBRARY)

