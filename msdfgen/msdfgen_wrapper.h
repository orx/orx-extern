#include "ft2build.h"
#include FT_FREETYPE_H

#if defined(__cplusplus)
extern "C"
{
#endif

int generateMTSDF(void* pPixelBuffer, int s32Stride, FT_GlyphSlot pGlyph, float fRange, double dAngleThreshold);

#if defined(__cplusplus)
}
#endif
