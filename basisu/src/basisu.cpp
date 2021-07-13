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
static transcoder_texture_format      seFormat          = transcoder_texture_format::cTFRGBA32;
static ktx2_image_level_info          sstInfo           = {};
static BasisURead                     spfnRead          = nullptr;
static BasisUSkip                     spfnSkip          = nullptr;

extern "C" void BasisU_Init(BasisURead _pfnRead, BasisUSkip _pfnSkip, BasisUFormat _eFormat)
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

    // Valid?
    if(spoKTX2Transcoder)
    {
      // Store callbacks
      spfnRead = _pfnRead;
      spfnSkip = _pfnSkip;

      // Store format
      switch(_eFormat)
      {
        case BasisUFormat_ASTC:
          seFormat = transcoder_texture_format::cTFASTC_4x4_RGBA;
          break;
        case BasisUFormat_BC7:
          seFormat = transcoder_texture_format::cTFBC7_RGBA;
          break;
        case BasisUFormat_BC3:
          seFormat = transcoder_texture_format::cTFBC3_RGBA;
          break;

        default:
          // Warn?

          // Fall through
        case BasisUFormat_Uncompressed:
          seFormat = transcoder_texture_format::cTFRGBA32;
          break;
      }
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

extern "C" int BasisU_GetInfo(void *_pData, unsigned int *_puiWidth, unsigned int *_puiHeight, unsigned int *_puiSize)
{
  ktx2_header   stHeader;
  unsigned int  uiResult = 0;

  // Read header
  if(spfnRead(_pData, &stHeader, sizeof(ktx2_header)) == sizeof(ktx2_header))
  {
    // Valid?
    if((memcmp(&stHeader, g_ktx2_file_identifier, sizeof(g_ktx2_file_identifier)) == 0)
    && (stHeader.m_vk_format == KTX2_VK_FORMAT_UNDEFINED)
    && (stHeader.m_type_size == 1))
    {
      // Store width, height and size
      *_puiWidth   = stHeader.m_pixel_width;
      *_puiHeight  = stHeader.m_pixel_height;
      *_puiSize    = basis_get_bytes_per_block_or_pixel(seFormat) * ((basis_transcoder_format_is_uncompressed(seFormat)) ? stHeader.m_pixel_width * stHeader.m_pixel_height : ((stHeader.m_pixel_width + 3) >> 2) * ((stHeader.m_pixel_height + 3) >> 2));

      // Update result
      uiResult = *_puiSize;
    }
  }

  // Resets data
  spfnSkip(_pData, -(int)sizeof(ktx2_header));

  // Done!
  return uiResult;
}

extern "C" int BasisU_Transcode(void *_pInput, unsigned int _uiInputSize, void *_pOutput, unsigned int _uiOutputSize)
{
  unsigned uiResult = 0;

  // Valid transcoder?
  if(spoKTX2Transcoder)
  {
    // Init transcode
    if(spoKTX2Transcoder->init(_pInput, _uiInputSize))
    {
      // Retrieve image info
      if(spoKTX2Transcoder->get_image_level_info(sstInfo, 0, 0, 0))
      {
        unsigned int uiRealSize;

        // Get real size
        uiRealSize = basis_get_bytes_per_block_or_pixel(seFormat) * ((basis_transcoder_format_is_uncompressed(seFormat)) ? sstInfo.m_orig_width * sstInfo.m_orig_height : sstInfo.m_total_blocks);

        // Has enough room?
        if(_uiOutputSize >= uiRealSize)
        {
          // Start transcoding
          if(spoKTX2Transcoder->start_transcoding())
          {
            // Transcode image
            if(spoKTX2Transcoder->transcode_image_level(0, 0, 0, _pOutput, uiRealSize / basis_get_bytes_per_block_or_pixel(seFormat), seFormat))
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
