//----------------------------------------------------------------------------------
// File:            libs\jni\nv_sound\nv_sound.h
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
#ifndef _NV_SOUND_H
#define _NV_SOUND_H

void NvSoundInit();
void NVSoundShutdown();

int SoundPoolLoadSFX(const char *FileName, int Priority);
int SoundPoolLoadSFXAsset(const char *FileName, int Priority);
void SoundPoolResume(int StreamID);
void SoundPoolStop(int StreamID);
int SoundPoolPlaySound(int SoundID, float LeftVolume, float RightVolume, int Priority, int Loop, float Rate);
void SoundPoolSetVolume(int StreamID, float LeftVolume, float RightVolume);
bool SoundPoolUnloadSample(int SoundID);

void MediaPlayerSetDataSource(const char* FileName);
void MediaPlayerStart();
void MediaPlayerSetVolume(float LeftVolume, float RightVolume);
void MediaPlayerStop();
void MediaSetMaxVolume();

#endif

