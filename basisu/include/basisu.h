// Orx - Simple Basis Universal C Wrapper

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
