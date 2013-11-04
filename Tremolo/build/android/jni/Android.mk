LOCAL_PATH := $(call my-dir)/../../..

include $(CLEAR_VARS)

LOCAL_MODULE = Tremolo
LOCAL_SRC_FILES = \
	bitwise.c      \
        bitwiseARM.S   \
        codebook.c     \
        dpen.S         \
        dsp.c          \
        floor0.c       \
        floor1.c       \
        floor1LARM.S   \
        floor_lookup.c \
        framing.c      \
        info.c         \
        mapping0.c     \
        mdct.c         \
        mdctLARM.S     \
        misc.c         \
        res012.c       \
        vorbisfile.c   \
        speed.S

LOCAL_CFLAGS := -DANDROID -DUSE_MEMORY_H -DHAVE_ALLOCA_H -D_LOW_ACCURACY_ -D_ARM_ASSEM_

LOCAL_ARM_MODE := arm
TARGET_PLATFORM = android-9

include $(BUILD_STATIC_LIBRARY)
