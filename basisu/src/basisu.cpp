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

#include "basisu/basisu_transcoder.h"
#include "basisu.h"
#include "zstd/zstddeclib.c"

using namespace basist;

static etc1_global_selector_codebook *spoCodeBook       = nullptr;
static ktx2_transcoder               *spoKTX2Transcoder = nullptr;

static inline transcoder_texture_format GetFormat(BasisUFormat _eFormat)
{
  transcoder_texture_format eResult;

  // Depending on format
  switch(_eFormat)
  {
    case BasisUFormat_ASTC:
      eResult = transcoder_texture_format::cTFASTC_4x4_RGBA;
      break;
    case BasisUFormat_BC7:
      eResult = transcoder_texture_format::cTFBC7_RGBA;
      break;
    case BasisUFormat_BC3:
      eResult = transcoder_texture_format::cTFBC3_RGBA;
      break;

    default:
      // Warn?

      // Fall through
    case BasisUFormat_Uncompressed:
      eResult = transcoder_texture_format::cTFRGBA32;
      break;
  }

  // Done!
  return eResult;
}

extern "C" void BasisU_Init()
{
  // No code book?
  if(spoCodeBook == nullptr)
  {
    // Init Basis Universal
    basisu_transcoder_init();

    // Create code book
    spoCodeBook = new etc1_global_selector_codebook(g_global_selector_cb_size, g_global_selector_cb);
  }

  // Valid?
  if(spoCodeBook)
  {
    // No KTX2 transcoder?
    if(spoKTX2Transcoder == nullptr)
    {
      // Create KTX2 transcoder
      spoKTX2Transcoder = new ktx2_transcoder(spoCodeBook);
    }
  }

  // Done!
  return;
}

extern "C" void BasisU_Exit()
{
  if(spoKTX2Transcoder)
  {
    delete spoKTX2Transcoder;
    spoKTX2Transcoder = nullptr;
  }
  if(spoCodeBook)
  {
    delete spoCodeBook;
    spoCodeBook = nullptr;
  }

  // Done!
  return;
}

extern "C" unsigned int BasisU_GetHeaderSize()
{
  return (unsigned int)sizeof(ktx2_header);
}

extern "C" int BasisU_GetInfo(void *_pInput, unsigned int _uiInputSize, BasisUFormat _eFormat, unsigned int *_puiWidth, unsigned int *_puiHeight, unsigned int *_puiSize)
{
  ktx2_header  *pstHeader = (ktx2_header *)_pInput;
  unsigned int  uiResult = 0;

  // Valid?
  if(_uiInputSize >= sizeof(ktx2_header))
  {
    // Valid?
    if((memcmp(pstHeader, g_ktx2_file_identifier, sizeof(g_ktx2_file_identifier)) == 0)
    && (pstHeader->m_vk_format == KTX2_VK_FORMAT_UNDEFINED)
    && (pstHeader->m_type_size == 1))
    {
      // Store width, height and size
      *_puiWidth   = pstHeader->m_pixel_width;
      *_puiHeight  = pstHeader->m_pixel_height;
      *_puiSize    = basis_get_bytes_per_block_or_pixel(GetFormat(_eFormat)) * ((basis_transcoder_format_is_uncompressed(GetFormat(_eFormat))) ? pstHeader->m_pixel_width * pstHeader->m_pixel_height : ((pstHeader->m_pixel_width + 3) >> 2) * ((pstHeader->m_pixel_height + 3) >> 2));

      // Update result
      uiResult = *_puiSize;
    }
  }

  // Done!
  return uiResult;
}

extern "C" int BasisU_Transcode(void *_pInput, unsigned int _uiInputSize, BasisUFormat _eFormat, void *_pOutput, unsigned int _uiOutputSize)
{
  unsigned uiResult = 0;

  // Valid transcoder?
  if(spoKTX2Transcoder)
  {
    // Init transcode
    if(spoKTX2Transcoder->init(_pInput, _uiInputSize))
    {
      ktx2_image_level_info sstInfo;

      // Retrieve image info
      if(spoKTX2Transcoder->get_image_level_info(sstInfo, 0, 0, 0))
      {
        unsigned int uiRealSize;

        // Get real size
        uiRealSize = basis_get_bytes_per_block_or_pixel(GetFormat(_eFormat)) * ((basis_transcoder_format_is_uncompressed(GetFormat(_eFormat))) ? sstInfo.m_orig_width * sstInfo.m_orig_height : sstInfo.m_total_blocks);

        // Has enough room?
        if(_uiOutputSize >= uiRealSize)
        {
          // Start transcoding
          if(spoKTX2Transcoder->start_transcoding())
          {
            // Transcode image
            if(spoKTX2Transcoder->transcode_image_level(0, 0, 0, _pOutput, uiRealSize / basis_get_bytes_per_block_or_pixel(GetFormat(_eFormat)), GetFormat(_eFormat)))
            {
              // Update result
              uiResult = uiRealSize;
            }
          }
        }
      }
    }
  }

  // Done!
  return uiResult;
}
