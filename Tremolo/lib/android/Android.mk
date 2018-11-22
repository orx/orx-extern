LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := Tremolo-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libTremolo.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../..
TARGET_PLATFORM = android-19
include $(PREBUILT_STATIC_LIBRARY)
