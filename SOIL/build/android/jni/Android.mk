LOCAL_PATH := $(call my-dir)/../../../src

include $(CLEAR_VARS)

LOCAL_MODULE = SOIL
LOCAL_SRC_FILES = \
	image_DXT.c \
	image_helper.c \
	SOIL.c \
	stb_image_aug.c

LOCAL_CFLAGS := -DANDROID

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_CFLAGS += -mhard-float
endif

LOCAL_ARM_MODE := arm
TARGET_PLATFORM = android-9

include $(BUILD_STATIC_LIBRARY)
