LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := LiquidFun-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libliquidfun.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include
TARGET_PLATFORM = android-19
include $(PREBUILT_STATIC_LIBRARY)
