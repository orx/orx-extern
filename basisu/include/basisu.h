/* Orx - Simple Basis Universal C Wrapper
 * Copyright (c) 2021- Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum __BasisUFormat_t
{
  BasisUFormat_ASTC,
  BasisUFormat_BC7,
  BasisUFormat_BC3,
  BasisUFormat_Uncompressed,

  BasisUFormat_None = 0xFFFFFFFFU

} BasisUFormat;

typedef unsigned int (*BasisURead) (void *_pData, void *_pOutput, unsigned int _uiSize);
typedef void (*BasisUSkip) (void *_pData, int _iOffset);

void BasisU_Init(BasisURead _pfnRead, BasisUSkip _pfnSkip, BasisUFormat _eFormat);
void BasisU_Exit();

int BasisU_GetInfo(void *_pData, unsigned int *_puiWidth, unsigned int *_puiHeight, unsigned int *_puiSize);
int BasisU_Transcode(void *_pInput, unsigned int _uiInputSize, void *_pOutput, unsigned int _uiOutputSize);

#ifdef __cplusplus
}
#endif /* __cplusplus */
