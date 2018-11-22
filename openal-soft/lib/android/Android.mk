LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := OpenAL-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libopenal-soft.a
LOCAL_EXPORT_LDLIBS = -lOpenSLES -llog
TARGET_PLATFORM = android-19

LOCAL_STATIC_LIBRARIES += cpufeatures

include $(PREBUILT_STATIC_LIBRARY)

$(call import-module,android/cpufeatures)
