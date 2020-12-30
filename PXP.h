/*****************************************************************
*  PXP Library Header File  12/21/2020  MJB
*  
*  Set up and use the Pixel Pipeline hardware for Color-Space
*  conversion,  scaling, and overlays.
*****************************************************************/


#ifndef  PXP_LIB_H
#define PXP_LIB_H

#include <stdint.h>
#include <stdio.h>
#include <TimeLib.h>


#ifdef __cplusplus
extern "C" {
#endif

// define four pointers to uninitialized buffers of 1MB in EXTMEM
extern const void *exptr1;
extern const void *exptr2;
extern const void *exptr3;
extern const void *exptr4;

enum tRotval {ROT0,ROT90,ROT180,ROT270};
class clPXP
{
  protected:
  private:


	public:

	uint32_t PXPNext0[32];
	uint32_t PXPNext1[32];
	uint32_t PXPNext2[32];
	uint32_t PXPNext3[32];
  
	void begin(uint16_t *inbuff, uint16_t *outbuff, uint16_t inh, uint16_t inv,
                        uint16_t outh, uint16_t outv);
						
	void SetPS(	uint16_t *psbuff,uint16_t inh, uint16_t inv, uint16_t cspace);
	void SetOutput(	uint16_t *outbuff, uint16_t outh, uint16_t outv, uint16_t cspace);
	void SetScale(float fscale);
	void SetStream(Stream *psptr);
	void Rotate(tRotval);
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
