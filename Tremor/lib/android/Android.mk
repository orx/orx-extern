LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := Tremor-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libTremor.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../..
TARGET_PLATFORM = android-9
include $(PREBUILT_STATIC_LIBRARY)
