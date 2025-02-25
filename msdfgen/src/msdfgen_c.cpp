#include "msdfgen.h"
#ifndef MSDFGEN_NO_FREETYPE
#define MSDFGEN_NO_FREETYPE
#endif /* MSDFGEN_NO_FREETYPE */
#include "msdfgen/msdfgen.h"
#undef MSDFGEN_NO_FREETYPE

extern "C" void MSDFGen_RenderGlyph(MSDFGen_Vertex *_astVertices, int _iVertexCount, int _iWidth, int _iHeight, float _fRange, float _fScaleX, float _fScaleY, float _fOffsetX, float _fOffsetY, unsigned char *_pu8Output, int _iStride)
{
  msdfgen::Shape stShape;
  
  /* Inverses Y axis */
  stShape.inverseYAxis = true;
  
  /* For all vertices */
  for(int i = 0; i < _iVertexCount; ++i)
  {
    /* Depending on type */
    switch(_astVertices[i].type)
    {
      default:
      case MSDFGen_Move:
      {
        stShape.addContour();
        break;
      }
      case MSDFGen_Line:
      {
        stShape.contours.back().addEdge({{(double)(_astVertices[i - 1].x), (double)(_astVertices[i - 1].y)}, {(double)(_astVertices[i].x), (double)(_astVertices[i].y)}});
        break;
      }
      case MSDFGen_Curve:
      {
        stShape.contours.back().addEdge({{(double)(_astVertices[i - 1].x), (double)(_astVertices[i - 1].y)}, {(double)(_astVertices[i].cx), (double)(_astVertices[i].cy)}, {(double)(_astVertices[i].x), (double)(_astVertices[i].y)}});
        break;
      }
      case MSDFGen_Cubic:
      {
        stShape.contours.back().addEdge({{(double)(_astVertices[i - 1].x), (double)(_astVertices[i - 1].y)}, {(double)(_astVertices[i].cx), (double)(_astVertices[i].cy)}, {(double)(_astVertices[i].cx1), (double)(_astVertices[i].cy1)}, {(double)(_astVertices[i].x), (double)(_astVertices[i].y)}});
        break;
      }
    }
  }
  
  /* Normalizes the shape */
  stShape.normalize();
  
  /* Orients its contours */
  stShape.orientContours();
  
  /* Colors its edges */
  msdfgen::edgeColoringByDistance(stShape, 3.0);
  
  /* Allocates temp bitmap */
  msdfgen::Bitmap<float, 4> oBitmap(_iWidth, _iHeight);
  
  /* Inits transformation */
  msdfgen::SDFTransformation stTransformation(msdfgen::Projection({_fScaleX, _fScaleY},
                                                                  {_fOffsetX, _fOffsetY}),
                                              msdfgen::Range(_fRange));
  
  /* Renders the MTSDF Glyph */
  msdfgen::generateMTSDF(oBitmap, stShape, stTransformation);
  
  /* Copies it to output */
  for(int y = 0; y < oBitmap.height(); y++)
  {
    for(int x = 0; x < oBitmap.width(); x++)
    {
      _pu8Output[x * 4 + y * _iStride + 0] = msdfgen::pixelFloatToByte(oBitmap(x, y)[0]);
      _pu8Output[x * 4 + y * _iStride + 1] = msdfgen::pixelFloatToByte(oBitmap(x, y)[1]);
      _pu8Output[x * 4 + y * _iStride + 2] = msdfgen::pixelFloatToByte(oBitmap(x, y)[2]);
      _pu8Output[x * 4 + y * _iStride + 3] = msdfgen::pixelFloatToByte(oBitmap(x, y)[3]);
    }
  }
}
