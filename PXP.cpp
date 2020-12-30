/****************************************
*  PXP Library  Functions
***************************************/
#include <Arduino.h>
#include "PXP.h"


const void *exptr1 = (void*)0x70100000;
const void *exptr2 = (void*)0x70200000;
const void *exptr3 = (void*)0x70300000;
const void *exptr4 = (void*)0x70400000;

// PXP_Next struct is 32 32-bit register settings, but we just save as an array
uint32_t PXPNext0[32];
uint32_t PXPNext1[32];
uint32_t PXPNext2[32];
uint32_t PXPNext3[32];


// instantiate one clPXP object
clPXP  pxp;  


  // Set the default output pointer to USB Serial
Stream *pxpstrm = &Serial;

void clPXP::begin(uint16_t *inbuff, uint16_t *outbuff, uint16_t inh, uint16_t inv,
                        uint16_t outh, uint16_t outv){
  // turn on the PXP Clock
  CCM_CCGR2 |= CCM_CCGR2_PXP(CCM_CCGR_ON);

  PXP_CTRL_SET = PXP_CTRL_SFTRST; //Reset the PXP
  PXP_CTRL_CLR = PXP_CTRL_SFTRST | PXP_CTRL_CLKGATE; //Clear reset and gate
  delay(10);

  PXP_CTRL_SET = PXP_CTRL_ROTATE(0) | PXP_CTRL_BLOCK_SIZE;  // Set Rotation 0 block size 16x16

  PXP_CSC1_COEF0 |= PXP_COEF0_BYPASS; 

  PXP_OUT_CTRL_SET = PXP_RGB565;  // specify RGB565 output
  PXP_OUT_BUF = (volatile void *)outbuff;
  PXP_OUT_PITCH = outh * 2; // Same as width * 2
  PXP_OUT_LRC = 0;
  PXP_OUT_LRC = (outh << 16 | outv);

  PXP_OUT_AS_ULC = 0xFFFFFFFF;  // not using the alpha surface
  PXP_OUT_AS_LRC = 0;

  PXP_OUT_PS_ULC = 0;  // start processing at upper left 0,0
  PXP_OUT_PS_LRC = ((outh) << 16) | (outv); // same as output

  PXP_PS_CTRL_SET = PXP_RGB565;  // PS buffer format is RGB565
  PXP_PS_BUF = (volatile void *)inbuff;
  PXP_PS_UBUF = 0;  // not using YUV
  PXP_PS_VBUF = 0;  // not using YUV
  PXP_PS_BACKGROUND_0 = 0x80; // dark blue
  PXP_PS_PITCH = inh*2; // input is width * 2 bytes 
  PXP_PS_SCALE = 0x10001000; // 1:1 scaling (0x1.000)
  PXP_PS_CLRKEYLOW_0 = 0xFFFFFF;  // this disables color keying
  PXP_PS_CLRKEYHIGH_0 = 0x0;  //  this disables color keying

  PXP_CTRL_SET = PXP_CTRL_IRQ_ENABLE;
  // we don't actually use the interrupt but need to enable the bits
  // in the PXP_STAT register
}

void clPXP::SetPS(	uint16_t *psbuff,uint16_t inh, uint16_t inv,  uint16_t cspace){
	PXP_PS_CTRL_CLR = 0x1F;
	PXP_PS_CTRL_SET = cspace;  // PS buffer format specification
	PXP_PS_BUF = (volatile void *)psbuff;
	PXP_PS_UBUF = 0;  // not using YUV planes
	PXP_PS_VBUF = 0;  // not using YUV planes
	PXP_PS_BACKGROUND_0 = 0x80; // dark blue
	PXP_PS_PITCH = inh*2; // input is width * 2 bytes 
	PXP_PS_SCALE = 0x10001000; // 1:1 scaling (0x1.000)
	PXP_PS_CLRKEYLOW_0 = 0xFFFFFF;  // this disables color keying
	PXP_PS_CLRKEYHIGH_0 = 0x0;  //  this disables color keying		
}
	
void clPXP::SetOutput(	uint16_t *outbuff, uint16_t outh, uint16_t outv,  uint16_t cspace){
	PXP_OUT_CTRL_CLR = 0x1Fl;
	PXP_OUT_CTRL_SET = cspace;  // specify  output color format
	PXP_OUT_BUF = (volatile void *)outbuff;
	PXP_OUT_PITCH = outh * 2; // assume Same as width * 2
	PXP_OUT_LRC = (outh << 16 | outv);		
}

// set both x and y scales to same value
void clPXP::SetScale(float fscale){
	uint16_t invscale;
	invscale = (1.0/fscale)* 4096;
	PXP_PS_SCALE = (invscale << 16) + invscale;
}
	
void clPXP::SetStream(Stream *psptr){
  pxpstrm = psptr;
}

// This member function prints a nicely formatted output of the PXP register settings
// The formatting does require using a monospaced font, like Courier
void clPXP::Show(void) {
  pxpstrm->printf("CTRL:         %08X       STAT:         %08X\n", PXP_CTRL, PXP_STAT);
  pxpstrm->printf("OUT_CTRL:     %08X       OUT_BUF:      %08X    OUT_BUF2: %08X\n", PXP_OUT_CTRL,PXP_OUT_BUF,PXP_OUT_BUF2);
  pxpstrm->printf("OUT_PITCH:    %8lu       OUT_LRC:       %3u,%3u\n", PXP_OUT_PITCH, PXP_OUT_LRC>>16, PXP_OUT_LRC&0xFFFF);

  pxpstrm->printf("OUT_PS_ULC:    %3u,%3u       OUT_PS_LRC:    %3u,%3u\n", PXP_OUT_PS_ULC>>16, PXP_OUT_PS_ULC&0xFFFF,
                                                               PXP_OUT_PS_LRC>>16, PXP_OUT_PS_LRC&0xFFFF);
  pxpstrm->printf("OUT_AS_ULC:   %3u,%3u    OUT_AS_LRC:    %3u,%3u\n", PXP_OUT_AS_ULC>>16, PXP_OUT_AS_ULC&0xFFFF,
                                                               PXP_OUT_AS_LRC>>16, PXP_OUT_AS_LRC&0xFFFF);
  pxpstrm->println();  // section separator
  pxpstrm->printf("PS_CTRL:      %08X       PS_BUF:       %08X\n", PXP_PS_CTRL,PXP_PS_BUF);
  pxpstrm->printf("PS_UBUF:      %08X       PS_VBUF:      %08X\n", PXP_PS_UBUF, PXP_PS_VBUF);
  pxpstrm->printf("PS_PITCH:     %8lu       PS_BKGND:     %08X\n", PXP_PS_PITCH, PXP_PS_BACKGROUND_0);
  pxpstrm->printf("PS_SCALE:     %08X       PS_OFFSET:    %08X\n", PXP_PS_SCALE,PXP_PS_OFFSET);
  pxpstrm->printf("PS_CLRKEYLOW: %08X       PS_CLRKEYLHI: %08X\n", PXP_PS_CLRKEYLOW_0,PXP_PS_CLRKEYHIGH_0);
  pxpstrm->println();
  pxpstrm->printf("AS_CTRL:      %08X       AS_BUF:       %08X    AS_PITCH: %6u\n", PXP_AS_CTRL,PXP_AS_BUF, PXP_AS_PITCH & 0xFFFF);
  pxpstrm->printf("AS_CLRKEYLOW: %08X       AS_CLRKEYLHI: %08X\n", PXP_AS_CLRKEYLOW_0,PXP_AS_CLRKEYHIGH_0);
  pxpstrm->println();
  pxpstrm->printf("CSC1_COEF0:   %08X       CSC1_COEF1:   %08X    CSC1_COEF2: %08X\n", 
                                                                PXP_CSC1_COEF0,PXP_CSC1_COEF1,PXP_CSC1_COEF2);
  pxpstrm->println();  // section separator
  pxpstrm->printf("POWER:        %08X       NEXT:         %08X\n", PXP_POWER,PXP_NEXT);
  pxpstrm->printf("PORTER_DUFF:  %08X\n", PXP_PORTER_DUFF_CTRL);
}


// save the PXP registers to the PXP_Next array passed as parameter
void clPXP::SaveNext(uint32_t pxnptr[]) {
  uint16_t i;
  volatile uint32_t *pxptr = &PXP_CTRL;  // set first address
  uint32_t *nxptr = &pxnptr[0];
  for (i = 0; i < 29; i++) { // first 29 are at 16-byte intervals
    *nxptr++ = *pxptr;
    pxptr += 4; // skips ahead 16 bytes at input
  }
  // the last three entries are oddly spaced
  *nxptr++ = PXP_POWER;
  *nxptr++ = PXP_NEXT;
  *nxptr = PXP_PORTER_DUFF_CTRL;
}

// print out a PXP_Next array in a format that can be pasted into
// source code to get the same PXP behavior
void clPXP::PrintNext(const char *arrayname, uint32_t pxnptr[]) {
  uint16_t i;
  pxpstrm->printf("\nuint32_t %s[32] = {", arrayname);
  for (i = 0; i < 31; i++) {
    if ((i % 6) == 0) pxpstrm->println();
    pxpstrm->printf("0x%08X, ", *pxnptr++);
  }
  // last one can't have a comma and needs bracket amd semicolon
  pxpstrm->printf("0x%08X };", *pxnptr);
  pxpstrm->println();
}

bool clPXP::Done(void) {
  return PXP_STAT & PXP_STAT_IRQ;
}

void clPXP::Start(void){
  PXP_STAT_CLR = PXP_STAT;  // clears all flags
  PXP_CTRL_SET =  PXP_CTRL_ENABLE;  // start the PXP	
}

void clPXP::Stop(void){
	PXP_CTRL_CLR =  PXP_CTRL_ENABLE;  // stop the PXP	
}

// Rotate output by 0,90,180,270 degrees
void clPXP::Rotate(tRotval rot) {
  PXP_CTRL_CLR = PXP_CTRL_ROTATE(3);  // clear previous settings
  PXP_CTRL_SET = PXP_CTRL_ROTATE(rot); // set requested rotation
  SaveNext((uint32_t*)&PXPNext0);
  PXP_STAT_CLR = PXP_STAT;  // clears all flags
  PXP_CTRL_SET =  PXP_CTRL_ENABLE;  // start the PXP
  // wait until rotation finished
  while (!Done()) {};
  PXP_CTRL_CLR =  PXP_CTRL_ENABLE;  // stop the PXP
}

// Restart PXP with settings from a PXP_Next array
void clPXP::StartNext(uint32_t pxnptr[]) {
  // turn on clock to PXP
  CCM_CCGR2 |= CCM_CCGR2_PXP(CCM_CCGR_ON);

  PXP_CTRL_SET = PXP_CTRL_SFTRST; //Reset
  PXP_CTRL_CLR = PXP_CTRL_SFTRST | PXP_CTRL_CLKGATE; //Clear reset and gate
  //delay(10);
  // storing pointer in PXP_NEXT causes PXP to restore settings
  PXP_NEXT = (uint32_t)&pxnptr;
}
