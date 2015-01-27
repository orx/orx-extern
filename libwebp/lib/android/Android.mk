LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := WebP-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libwebpdecoder.a
TARGET_PLATFORM = android-9

LOCAL_STATIC_LIBRARIES += cpufeatures

include $(PREBUILT_STATIC_LIBRARY)

$(call import-module,android/cpufeatures)
