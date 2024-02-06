#define MSDFGEN_PUBLIC
#include "msdfgen.h"

#include "ftoutln.h"

using namespace msdfgen;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

struct FtContext
{
  Point2 position;
  Shape* shape;
  Contour* contour;
};

#define F26DOT6_TO_DOUBLE(x) (1/64.*double(x))

static Point2 ftPoint2(const FT_Vector& vector)
{
  return Point2(F26DOT6_TO_DOUBLE(vector.x), F26DOT6_TO_DOUBLE(vector.y));
}

static int ftMoveTo(const FT_Vector* to, void* user)
{
  FtContext* context = reinterpret_cast<FtContext*>(user);
  if(!(context->contour && context->contour->edges.empty()))
  {
    context->contour = &context->shape->addContour();
  }
  context->position = ftPoint2(*to);
  return 0;
}

static int ftLineTo(const FT_Vector* to, void* user)
{
  FtContext* context = reinterpret_cast<FtContext*>(user);
  Point2 endpoint = ftPoint2(*to);
  if(endpoint != context->position)
  {
    context->contour->addEdge(new LinearSegment(context->position, endpoint));
    context->position = endpoint;
  }
  return 0;
}

static int ftConicTo(const FT_Vector* control, const FT_Vector* to, void* user)
{
  FtContext* context = reinterpret_cast<FtContext*>(user);
  context->contour->addEdge(new QuadraticSegment(context->position, ftPoint2(*control), ftPoint2(*to)));
  context->position = ftPoint2(*to);
  return 0;
}

static int ftCubicTo(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user)
{
  FtContext* context = reinterpret_cast<FtContext*>(user);
  context->contour->addEdge(new CubicSegment(context->position, ftPoint2(*control1), ftPoint2(*control2), ftPoint2(*to)));
  context->position = ftPoint2(*to);
  return 0;
}

static FT_Error readFreetypeOutline(Shape& output, FT_Outline* outline)
{
  output.contours.clear();
  output.inverseYAxis = false;
  FtContext context = { };
  context.shape = &output;
  FT_Outline_Funcs ftFunctions;
  ftFunctions.move_to = &ftMoveTo;
  ftFunctions.line_to = &ftLineTo;
  ftFunctions.conic_to = &ftConicTo;
  ftFunctions.cubic_to = &ftCubicTo;
  ftFunctions.shift = 0;
  ftFunctions.delta = 0;
  FT_Error error = FT_Outline_Decompose(outline, &ftFunctions, &context);
  if(!output.contours.empty() && output.contours.back().edges.empty())
  {
    output.contours.pop_back();
  }
  
  return error;
}

extern "C" void generateMTSDF(void* pPixelBuffer, int s32Stride, FT_GlyphSlot pGlyph, float fRange, double dAngleThreshold)
{
  int s32Width, s32Height;
  double left, right, top, bottom;
  Shape shape;

  readFreetypeOutline(shape, &pGlyph->outline);
  shape.inverseYAxis = true;

  shape.normalize();
  edgeColoringInkTrap(shape, dAngleThreshold);

  Shape::Bounds bounds = shape.getBounds();

  s32Width = (int)ceil(pGlyph->bitmap.width + fRange) + 1;
  s32Height = (int)ceil(pGlyph->bitmap.rows + fRange) + 1;

  left = bounds.l - 0.5 * fRange;
  right = bounds.r + 0.5 * fRange;
  bottom = bounds.b - 0.5 * fRange;
  top = bounds.t + 0.5 * fRange;

  Vector2 dims(s32Width, s32Height);
  Vector2 translate(0.5 * (dims.x - right - left), 0.5 * (dims.y - top - bottom));

  Bitmap<float, 4> mtsdf(s32Width, s32Height);
  generateMTSDF(mtsdf, shape, fRange, 1.0, translate);

  unsigned char* pDst = (unsigned char*)pPixelBuffer;

  for (int i = 0; i < mtsdf.height(); i++)
  {
    for (int j = 0; j < mtsdf.width(); j++)
    {
      *pDst++ = pixelFloatToByte(mtsdf(j, i)[0]);
      *pDst++ = pixelFloatToByte(mtsdf(j, i)[1]);
      *pDst++ = pixelFloatToByte(mtsdf(j, i)[2]);
      *pDst++ = pixelFloatToByte(mtsdf(j, i)[3]);
    }

    /* Advance to next row in target buffer, taking into account our written data. */
    pDst += (s32Stride - mtsdf.width() * 4);
  }
}
