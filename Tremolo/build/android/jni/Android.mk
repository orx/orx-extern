LOCAL_PATH := $(call my-dir)/../../..

include $(CLEAR_VARS)

LOCAL_MODULE = Tremolo
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

LOCAL_CFLAGS := -DANDROID -DUSE_MEMORY_H -DHAVE_ALLOCA_H -D_LOW_ACCURACY_

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_SRC_FILES += \
           bitwiseARM.S   \
           dpen.S         \
           floor1LARM.S   \
           mdctLARM.S

    LOCAL_CFLAGS += -mhard-float -D_ARM_ASSEM_
endif

ifeq ($(TARGET_ARCH_ABI),armeabi)
    LOCAL_SRC_FILES += \
           bitwiseARM.S   \
           dpen.S         \
           floor1LARM.S   \
           mdctLARM.S

    LOCAL_CFLAGS += -D_ARM_ASSEM_
endif

ifeq ($(TARGET_ARCH_ABI),x86)
    LOCAL_CFLAGS += -DONLY_C
endif


LOCAL_ARM_MODE := arm
TARGET_PLATFORM = android-9

include $(BUILD_STATIC_LIBRARY)
