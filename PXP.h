/*****************************************************************
*  PXPV2 Library Header File  1/24/21  MJB
*  
*  Set up and use the Pixel Pipeline hardware for Color-Space
*  conversion,  scaling, and overlays.
*
*****************************************************************/


#ifndef  PXP_LIB_H
#define PXP_LIB_H

#include <stdint.h>
#include <stdio.h>
#include <TimeLib.h>


#ifdef __cplusplus
extern "C" {
#endif


#define SetCorner( reg, h,v)  reg = ((h<<16) | v)

// define four pointers to uninitialized buffers of 1MB in EXTMEM
extern const void *exptr1;
extern const void *exptr2;
extern const void *exptr3;
extern const void *exptr4;

enum tRotval {ROT0,ROT90,ROT180,ROT270};

typedef struct tBitmap{
  void *pbits;
  uint32_t pixelbytes;
  uint16_t width;
  uint16_t height;
  uint16_t maptype;   // RGB888, RGB565, etc.
  uint16_t spare;  // to make structure 16 bytes length
} Bitmap;

class clPXP
{
  protected:
  private:

	public:

	uint32_t PXPNext0[32];
	uint32_t PXPNext1[32];
	uint32_t PXPNext2[32];
	uint32_t PXPNext3[32];
  
						
	void begin(void);	
	uint16_t BytesPerPixel(uint16_t mpt);	
	void SetOutput(	void *outbuff, uint16_t outh, uint16_t outv,  uint16_t cspace);
	void SetPS(	void *psbuff,uint16_t inh, uint16_t inv,  uint16_t cspace);
	void SetAS(	void *asbuff,uint16_t inh, uint16_t inv,  uint16_t cspace);

	void SetScale(float fscale);
	void SetOVRAlpha(uint8_t alpha);
	void SetStream(Stream *psptr);
	void Rotate(tRotval);
	void SetFlip(uint16_t flipval);
	bool Done(void);
	void Start(void);
	void Stop(void);
	void StartNext(uint32_t pxnptr[]);
	void SaveNext(uint32_t pxnptr[]);
	void Show(void);
	void PrintNext(const char *arrayname, uint32_t pxnptr[]) ;

};

extern clPXP pxp;

#ifdef __cplusplus
}
#endif


#endif // PXP_Lib.h
