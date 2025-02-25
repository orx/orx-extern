/* Orx - Simple MSDFGen+STB_TrueType C Wrapper
 * Copyright (c) 2025- Orx-Project
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

/* Vertex structure, lifted from stb_truetype/stbtt_vertex */
typedef struct
{
  short         x, y, cx, cy, cx1, cy1;
  unsigned char type, padding;
} MSDFGen_Vertex;

enum
{
  MSDFGen_Move = 1,
  MSDFGen_Line,
  MSDFGen_Curve,
  MSDFGen_Cubic
};

// Render MSDF glyph
void MSDFGen_RenderGlyph(MSDFGen_Vertex *_astVertices, int _iVertexCount, int _iWidth, int _iHeight, float _fRange, float _fScaleX, float _fScaleY, float _fOffsetX, float _fOffsetY, unsigned char *_pu8Output, int _iStride);

#ifdef __cplusplus
}
#endif /* __cplusplus */
