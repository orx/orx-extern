LOCAL_PATH := $(call my-dir)/../../..

include $(CLEAR_VARS)

LOCAL_MODULE = Tremor
LOCAL_SRC_FILES = \
	bitwise.c      \
        codebook.c     \
        dsp.c          \
        floor0.c       \
        floor1.c       \
        floor_lookup.c \
        framing.c      \
        info.c         \
        mapping0.c     \
        mdct.c         \
        misc.c         \
        res012.c       \
        vorbisfile.c

LOCAL_CFLAGS := -DANDROID -D_ARM_ASSEM_ -DUSE_MEMORY_H -DHAVE_ALLOCA_H

LOCAL_ARM_MODE := arm
TARGET_PLATFORM = android-9

include $(BUILD_STATIC_LIBRARY)
