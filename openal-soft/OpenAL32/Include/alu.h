#ifndef _ALU_H_
#define _ALU_H_

#include <limits.h>
#include <math.h>
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif

#include "alMain.h"
#include "alBuffer.h"
#include "alFilter.h"
#include "alAuxEffectSlot.h"

#include "hrtf.h"
#include "align.h"
#include "math_defs.h"


#define MAX_PITCH  (255)

/* Maximum number of buffer samples before the current pos needed for resampling. */
#define MAX_PRE_SAMPLES 12

/* Maximum number of buffer samples after the current pos needed for resampling. */
#define MAX_POST_SAMPLES 12


#ifdef __cplusplus
extern "C" {
#endif

struct ALsource;
struct ALvoice;
struct ALeffectslot;


/* The number of distinct scale and phase intervals within the filter table. */
#define BSINC_SCALE_BITS  4
#define BSINC_SCALE_COUNT (1<<BSINC_SCALE_BITS)
#define BSINC_PHASE_BITS  4
#define BSINC_PHASE_COUNT (1<<BSINC_PHASE_BITS)

/* Interpolator state.  Kind of a misnomer since the interpolator itself is
 * stateless.  This just keeps it from having to recompute scale-related
 * mappings for every sample.
 */
typedef struct BsincState {
    ALfloat sf; /* Scale interpolation factor. */
    ALuint m;   /* Coefficient count. */
    ALint l;    /* Left coefficient offset. */
    struct {
        const ALfloat *filter;   /* Filter coefficients. */
        const ALfloat *scDelta;  /* Scale deltas. */
        const ALfloat *phDelta;  /* Phase deltas. */
        const ALfloat *spDelta;  /* Scale-phase deltas. */
    } coeffs[BSINC_PHASE_COUNT];
} BsincState;


typedef union aluVector {
    alignas(16) ALfloat v[4];
} aluVector;

inline void aluVectorSet(aluVector *vector, ALfloat x, ALfloat y, ALfloat z, ALfloat w)
{
    vector->v[0] = x;
    vector->v[1] = y;
    vector->v[2] = z;
    vector->v[3] = w;
}


typedef union aluMatrixf {
    alignas(16) ALfloat m[4][4];
} aluMatrixf;

inline void aluMatrixfSetRow(aluMatrixf *matrix, ALuint row,
                             ALfloat m0, ALfloat m1, ALfloat m2, ALfloat m3)
{
    matrix->m[row][0] = m0;
    matrix->m[row][1] = m1;
    matrix->m[row][2] = m2;
    matrix->m[row][3] = m3;
}

inline void aluMatrixfSet(aluMatrixf *matrix, ALfloat m00, ALfloat m01, ALfloat m02, ALfloat m03,
                                              ALfloat m10, ALfloat m11, ALfloat m12, ALfloat m13,
                                              ALfloat m20, ALfloat m21, ALfloat m22, ALfloat m23,
                                              ALfloat m30, ALfloat m31, ALfloat m32, ALfloat m33)
{
    aluMatrixfSetRow(matrix, 0, m00, m01, m02, m03);
    aluMatrixfSetRow(matrix, 1, m10, m11, m12, m13);
    aluMatrixfSetRow(matrix, 2, m20, m21, m22, m23);
    aluMatrixfSetRow(matrix, 3, m30, m31, m32, m33);
}


typedef union aluMatrixd {
    alignas(16) ALdouble m[4][4];
} aluMatrixd;

inline void aluMatrixdSetRow(aluMatrixd *matrix, ALuint row,
                             ALdouble m0, ALdouble m1, ALdouble m2, ALdouble m3)
{
    matrix->m[row][0] = m0;
    matrix->m[row][1] = m1;
    matrix->m[row][2] = m2;
    matrix->m[row][3] = m3;
}

inline void aluMatrixdSet(aluMatrixd *matrix, ALdouble m00, ALdouble m01, ALdouble m02, ALdouble m03,
                                              ALdouble m10, ALdouble m11, ALdouble m12, ALdouble m13,
                                              ALdouble m20, ALdouble m21, ALdouble m22, ALdouble m23,
                                              ALdouble m30, ALdouble m31, ALdouble m32, ALdouble m33)
{
    aluMatrixdSetRow(matrix, 0, m00, m01, m02, m03);
    aluMatrixdSetRow(matrix, 1, m10, m11, m12, m13);
    aluMatrixdSetRow(matrix, 2, m20, m21, m22, m23);
    aluMatrixdSetRow(matrix, 3, m30, m31, m32, m33);
}


enum ActiveFilters {
    AF_None = 0,
    AF_LowPass = 1,
    AF_HighPass = 2,
    AF_BandPass = AF_LowPass | AF_HighPass
};


typedef struct MixGains {
    ALfloat Current;
    ALfloat Step;
    ALfloat Target;
} MixGains;


typedef struct DirectParams {
    ALfloat (*OutBuffer)[BUFFERSIZE];
    ALuint OutChannels;

    /* If not 'moving', gain/coefficients are set directly without fading. */
    ALboolean Moving;
    /* Stepping counter for gain/coefficient fading. */
    ALuint Counter;
    /* Last direction (relative to listener) and gain of a moving source. */
    aluVector LastDir;
    ALfloat LastGain;

    struct {
        enum ActiveFilters ActiveType;
        ALfilterState LowPass;
        ALfilterState HighPass;
    } Filters[MAX_INPUT_CHANNELS];

    struct {
        HrtfParams Params;
        HrtfState State;
    } Hrtf[MAX_INPUT_CHANNELS];
    MixGains Gains[MAX_INPUT_CHANNELS][MAX_OUTPUT_CHANNELS];
} DirectParams;

typedef struct SendParams {
    ALfloat (*OutBuffer)[BUFFERSIZE];
    ALuint OutChannels;

    ALboolean Moving;
    ALuint Counter;

    struct {
        enum ActiveFilters ActiveType;
        ALfilterState LowPass;
        ALfilterState HighPass;
    } Filters[MAX_INPUT_CHANNELS];
    MixGains Gains[MAX_INPUT_CHANNELS][MAX_EFFECT_CHANNELS];
} SendParams;


typedef const ALfloat* (*ResamplerFunc)(const BsincState *state,
    const ALfloat *src, ALuint frac, ALuint increment, ALfloat *restrict dst, ALuint dstlen
);

typedef void (*MixerFunc)(const ALfloat *data, ALuint OutChans,
                          ALfloat (*restrict OutBuffer)[BUFFERSIZE], struct MixGains *Gains,
                          ALuint Counter, ALuint OutPos, ALuint BufferSize);
typedef void (*HrtfMixerFunc)(ALfloat (*restrict OutBuffer)[BUFFERSIZE], const ALfloat *data,
                              ALuint Counter, ALuint Offset, ALuint OutPos,
                              const ALuint IrSize, const HrtfParams *hrtfparams,
                              HrtfState *hrtfstate, ALuint BufferSize);


#define GAIN_SILENCE_THRESHOLD  (0.00001f) /* -100dB */

#define SPEEDOFSOUNDMETRESPERSEC  (343.3f)
#define AIRABSORBGAINHF           (0.99426f) /* -0.05dB */

#define FRACTIONBITS (12)
#define FRACTIONONE  (1<<FRACTIONBITS)
#define FRACTIONMASK (FRACTIONONE-1)


static inline ALfloat minf(ALfloat a, ALfloat b)
{ return ((a > b) ? b : a); }
static inline ALfloat maxf(ALfloat a, ALfloat b)
{ return ((a > b) ? a : b); }
static inline ALfloat clampf(ALfloat val, ALfloat min, ALfloat max)
{ return minf(max, maxf(min, val)); }

static inline ALdouble mind(ALdouble a, ALdouble b)
{ return ((a > b) ? b : a); }
static inline ALdouble maxd(ALdouble a, ALdouble b)
{ return ((a > b) ? a : b); }
static inline ALdouble clampd(ALdouble val, ALdouble min, ALdouble max)
{ return mind(max, maxd(min, val)); }

static inline ALuint minu(ALuint a, ALuint b)
{ return ((a > b) ? b : a); }
static inline ALuint maxu(ALuint a, ALuint b)
{ return ((a > b) ? a : b); }
static inline ALuint clampu(ALuint val, ALuint min, ALuint max)
{ return minu(max, maxu(min, val)); }

static inline ALint mini(ALint a, ALint b)
{ return ((a > b) ? b : a); }
static inline ALint maxi(ALint a, ALint b)
{ return ((a > b) ? a : b); }
static inline ALint clampi(ALint val, ALint min, ALint max)
{ return mini(max, maxi(min, val)); }

static inline ALint64 mini64(ALint64 a, ALint64 b)
{ return ((a > b) ? b : a); }
static inline ALint64 maxi64(ALint64 a, ALint64 b)
{ return ((a > b) ? a : b); }
static inline ALint64 clampi64(ALint64 val, ALint64 min, ALint64 max)
{ return mini64(max, maxi64(min, val)); }

static inline ALuint64 minu64(ALuint64 a, ALuint64 b)
{ return ((a > b) ? b : a); }
static inline ALuint64 maxu64(ALuint64 a, ALuint64 b)
{ return ((a > b) ? a : b); }
static inline ALuint64 clampu64(ALuint64 val, ALuint64 min, ALuint64 max)
{ return minu64(max, maxu64(min, val)); }


union ResamplerCoeffs {
    ALfloat FIR4[FRACTIONONE][4];
    ALfloat FIR8[FRACTIONONE][8];
};
extern alignas(16) union ResamplerCoeffs ResampleCoeffs;

extern alignas(16) const ALfloat bsincTab[18840];


static inline ALfloat lerp(ALfloat val1, ALfloat val2, ALfloat mu)
{
    return val1 + (val2-val1)*mu;
}
static inline ALfloat resample_fir4(ALfloat val0, ALfloat val1, ALfloat val2, ALfloat val3, ALuint frac)
{
    const ALfloat *k = ResampleCoeffs.FIR4[frac];
    return k[0]*val0 + k[1]*val1 + k[2]*val2 + k[3]*val3;
}
inline ALfloat resample_fir8(ALfloat val0, ALfloat val1, ALfloat val2, ALfloat val3, ALfloat val4, ALfloat val5, ALfloat val6, ALfloat val7, ALuint frac)
{
    const ALfloat *k = ResampleCoeffs.FIR8[frac];
    return k[0]*val0 + k[1]*val1 + k[2]*val2 + k[3]*val3 +
           k[4]*val4 + k[5]*val5 + k[6]*val6 + k[7]*val7;
}


void aluInitMixer(void);

ALvoid aluInitPanning(ALCdevice *Device);

void aluInitEffectPanning(struct ALeffectslot *slot);

/**
 * CalcDirectionCoeffs
 *
 * Calculates ambisonic coefficients based on a direction vector. The vector
 * must not be longer than 1 unit.
 */
void CalcDirectionCoeffs(const ALfloat dir[3], ALfloat coeffs[MAX_AMBI_COEFFS]);

/**
 * CalcXYZCoeffs
 *
 * Same as CalcDirectionCoeffs except the direction is specified as separate x,
 * y, and z parameters instead of an array.
 */
static inline void CalcXYZCoeffs(ALfloat x, ALfloat y, ALfloat z, ALfloat coeffs[MAX_AMBI_COEFFS])
{
    ALfloat dir[3] = { x, y, z };
    CalcDirectionCoeffs(dir, coeffs);
}

/**
 * CalcAngleCoeffs
 *
 * Calculates ambisonic coefficients based on angle and elevation. The angle
 * and elevation parameters are in radians, going right and up respectively.
 */
void CalcAngleCoeffs(ALfloat angle, ALfloat elevation, ALfloat coeffs[MAX_AMBI_COEFFS]);

/**
 * ComputeAmbientGains
 *
 * Computes channel gains for ambient, omni-directional sounds.
 */
void ComputeAmbientGains(const ChannelConfig *chancoeffs, ALuint numchans, ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS]);

/**
 * ComputePanningGains
 *
 * Computes panning gains using the given channel decoder coefficients and the
 * pre-calculated direction or angle coefficients.
 */
void ComputePanningGains(const ChannelConfig *chancoeffs, ALuint numchans, const ALfloat coeffs[MAX_AMBI_COEFFS], ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS]);

/**
 * ComputeFirstOrderGains
 *
 * Sets channel gains for a first-order ambisonics input channel. The matrix is
 * a 1x4 'slice' of a transform matrix for the input channel, used to scale and
 * orient the sound samples.
 */
void ComputeFirstOrderGains(const ChannelConfig *chancoeffs, ALuint numchans, const ALfloat mtx[4], ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS]);


ALvoid UpdateContextSources(ALCcontext *context);

ALvoid CalcSourceParams(struct ALvoice *voice, const struct ALsource *source, const ALCcontext *ALContext);
ALvoid CalcNonAttnSourceParams(struct ALvoice *voice, const struct ALsource *source, const ALCcontext *ALContext);

ALvoid MixSource(struct ALvoice *voice, struct ALsource *source, ALCdevice *Device, ALuint SamplesToDo);

ALvoid aluMixData(ALCdevice *device, ALvoid *buffer, ALsizei size);
/* Caller must lock the device. */
ALvoid aluHandleDisconnect(ALCdevice *device);

extern ALfloat ConeScale;
extern ALfloat ZScale;

#ifdef __cplusplus
}
#endif

#endif