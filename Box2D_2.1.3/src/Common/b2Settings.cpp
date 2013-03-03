/*
* Copyright (c) 2006-2009 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include <Box2D/Common/b2Settings.h>

//! Orx modification

#ifdef ANDROID

  #include <stdlib.h>

#else // ANDROID

  #include <cstdlib>

#endif // ANDROID

b2Version b2_version = {2, 2, 0};

void* (*b2_custom_alloc)(int32) = NULL;
void (*b2_custom_free)(void*) = NULL;

void b2SetCustomAllocFree(void*(*_alloc)(int32), void(*_free)(void*))
{
  b2_custom_alloc = _alloc;
  b2_custom_free  = _free;
}

void set_b2_custom_alloc(void*(*_alloc)(int32))
{
  b2_custom_alloc = _alloc;
}

// Memory allocators. Modify these to use your own allocator.
void* b2Alloc(int32 size)
{
  return b2_custom_alloc ? b2_custom_alloc(size) : malloc(size);
}

void b2Free(void* mem)
{
  b2_custom_free ? b2_custom_free(mem) : free(mem);
}

//! End of Orx modification
