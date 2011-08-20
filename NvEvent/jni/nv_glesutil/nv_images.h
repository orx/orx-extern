//----------------------------------------------------------------------------------
// File:            libs\jni\nv_glesutil\nv_images.h
// Samples Version: NVIDIA Android Lifecycle samples 1_0beta 
// Email:           tegradev@nvidia.com
// Web:             http://developer.nvidia.com/category/zone/mobile-development
//
// Copyright 2009-2011 NVIDIA® Corporation 
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//----------------------------------------------------------------------------------
#ifndef _NV_IMAGES_H
#define _NV_IMAGES_H

#include "../nv_global.h"

#include "../nv_hhdds/nv_hhdds.h"
#include <GLES2/gl2.h>

GLuint NvCreateTextureFromDDSEx(const char* filename, NvBool flipVertical,
                                NvBool useMipmaps, NvS32* width, NvS32* height, NvBool* alpha, NvBool *isCubeMap);

#define NvCreateTextureFromDDS(a,b,c) NvCreateTextureFromDDSEx(a,b,c,0,0,0,0)

GLuint NvCreateTextureFromHHDDSImage(const NVHHDDSImage& image,
                                NvBool useMipmaps, NvS32* width, NvS32* height, NvBool* alpha, NvBool *isCubeMap);

#endif

