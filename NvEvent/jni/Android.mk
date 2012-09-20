#----------------------------------------------------------------------------------
# File:            libs\jni\Android.mk
# Samples Version: NVIDIA Android Lifecycle samples 1_0beta 
# Email:           tegradev@nvidia.com
# Web:             http://developer.nvidia.com/category/zone/mobile-development
#
# Copyright 2009-2011 NVIDIA® Corporation 
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#----------------------------------------------------------------------------------
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := NvEvent
LOCAL_SRC_FILES = \
        nv_math/nv_math.cpp \
        nv_math/nv_matrix.cpp \
        nv_math/nv_quat.cpp \
        nv_shader/nv_shader.cpp \
	nv_apk_file/nv_apk_file.c \
        nv_util/nv_util.cpp \
        nv_time/nv_time.cpp \
        nv_event/nv_event.cpp \
        nv_event/nv_event_queue.cpp \
        nv_file/nv_file.c \
        nv_thread/nv_thread.c \
        nv_hhdds/nv_hhdds.cpp \
        nv_log/nv_log.cpp \
        nv_glesutil/nv_images.cpp \
        nv_sound/nv_sound.cpp

LOCAL_ARM_MODE   := arm
TARGET_PLATFORM = android-8

include $(BUILD_STATIC_LIBRARY)

