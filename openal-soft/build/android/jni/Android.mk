LOCAL_PATH := $(call my-dir)/../../..

include $(CLEAR_VARS)

LOCAL_CFLAGS :=-fvisibility=internal -Wno-psabi -fPIC -DAL_BUILD_LIBRARY -D_GNU_SOURCE=1 -DAL_ALEXT_PROTOTYPES
LOCAL_MODULE = openal-soft
LOCAL_SRC_FILES = \
        OpenAL32/alAuxEffectSlot.c \
        OpenAL32/alBuffer.c        \
        OpenAL32/alEffect.c        \
        OpenAL32/alError.c         \
        OpenAL32/alExtension.c     \
        OpenAL32/alFilter.c        \
        OpenAL32/alListener.c      \
        OpenAL32/alSource.c        \
        OpenAL32/alState.c         \
        OpenAL32/alThunk.c         \
        Alc/ALc.c                  \
        Alc/alcConfig.c            \
        Alc/alcDedicated.c         \
        Alc/alcEcho.c              \
        Alc/alcModulator.c         \
        Alc/alcReverb.c            \
        Alc/alcRing.c              \
        Alc/alcThread.c            \
        Alc/ALu.c                  \
        Alc/bs2b.c                 \
        Alc/helpers.c              \
        Alc/panning.c              \
        Alc/hrtf.c                 \
        Alc/mixer.c                \
        Alc/mixer_c.c              \
        Alc/backends/loopback.c    \
        Alc/backends/null.c        \
	Alc/backends/opensl.c      \
        Alc/backends/wave.c

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_CFLAGS += -mhard-float -DHAVE_NEON -DHAVE_ARM_NEON_H
    LOCAL_SRC_FILES += Alc/mixer_neon.c.neon
endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)/build/android/jni $(LOCAL_PATH)/include $(LOCAL_PATH)/OpenAL32/Include

LOCAL_ARM_MODE := arm
TARGET_PLATFORM = android-9
LOCAL_STATIC_LIBRARIES += cpufeatures

include $(BUILD_STATIC_LIBRARY)

$(call import-module,android/cpufeatures)
