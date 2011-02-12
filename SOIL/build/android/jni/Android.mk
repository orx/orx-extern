LOCAL_PATH := $(call my-dir)

SRC_FILES := 
include $(CLEAR_VARS)

LOCAL_MODULE    := SOIL
LOCAL_CFLAGS := -DANDROID
LOCAL_CFLAGS += -I$(LOCAL_PATH)/../../../include
LOCAL_CFLAGS += -I$(LOCAL_PATH)/../../../src

LOCAL_CPPFLAGS := -DANDROID
LOCAL_CPPFLAGS += -I$(LOCAL_PATH)/../../../include
LOCAL_CPPFLAGS += -I$(LOCAL_PATH)/../../../src
LOCAL_ARM_MODE := arm
LOCAL_DEFAULT_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := \
../../../src/image_DXT.c \
../../../src/image_helper.c \
../../../src/SOIL.c \
../../../src/stb_image_aug.c


LOCAL_LDLIBS := -ldl -llog -lm -lgcc

include $(BUILD_STATIC_LIBRARY)

