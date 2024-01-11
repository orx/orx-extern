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

static bool autoFrame(Projection* pProjection, Vector2 frame, Shape::Bounds bounds, float fRange)
{
  Vector2 translate, scale;
  double left = bounds.l, bottom = bounds.b, right = bounds.r, top = bounds.t;
  double m = 0.5;

  frame -= 2.0 * m * fRange;

  if(left >= right || bottom >= top)
  {
    left = bottom = 0, right = top = 1;
  }

  if(frame.x <= 0 || frame.y <= 0)
  {
    return false;
  }

  Vector2 dims(right - left, top - bottom);
  if(dims.x * frame.y < dims.y * frame.x)
  {
    translate.set(0.5 * (frame.x / frame.y * dims.y - dims.x) - left, -bottom);
    scale = frame.y / dims.y;
  }
  else
  {
    translate.set(-left, 0.5 * (frame.y / frame.x * dims.x - dims.y) - bottom);
    scale = frame.x / dims.x;
  }

  translate += m * fRange / scale;

  *pProjection = Projection(scale, translate);
  return true;
}

extern "C" int generateMTSDF(void* pPixelBuffer, int s32Stride, FT_GlyphSlot pGlyph, float fRange, double dAngleThreshold)
{
  Shape shape;
  readFreetypeOutline(shape, &pGlyph->outline);

  shape.normalize();
  edgeColoringInkTrap(shape, dAngleThreshold);

  Vector2 frame(pGlyph->bitmap.width, pGlyph->bitmap.rows);
  Projection projection;
  if(!autoFrame(&projection, frame, shape.getBounds(), fRange))
  {
    return 0;
  }

  Bitmap<float, 4> mtsdf(pGlyph->bitmap.width, pGlyph->bitmap.rows);
  generateMTSDF(mtsdf, shape, projection, fRange);

  unsigned char* pDst = (unsigned char*)pPixelBuffer;

  for (int i = mtsdf.height() - 1; i >= 0; i--)
  {
    for (int j = 0; j < mtsdf.width(); j++)
    {
      *pDst++ = pixelFloatToByte(mtsdf(j, i)[0]);
      *pDst++ = pixelFloatToByte(mtsdf(j, i)[1]);
      *pDst++ = pixelFloatToByte(mtsdf(j, i)[2]);
      *pDst++ = pixelFloatToByte(mtsdf(j, i)[3]);
    }

    // Advance to next row in target buffer, taking into account our written data.
    pDst += (s32Stride - mtsdf.width() * 4);
  }

  return 1;
}
