LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := WebP-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libwebpdecoder.a
TARGET_PLATFORM = android-19

include $(PREBUILT_STATIC_LIBRARY)
