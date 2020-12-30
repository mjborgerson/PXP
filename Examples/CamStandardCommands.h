//  Header for OV7670 Standard Commands
#define FRAME_WIDTH 320l
#define FRAME_HEIGHT 240l

#define FB_WIDTH 320l
#define FB_HEIGHT 240l
#define FB_COUNT 2  // two frames in EXTMEM

#define FRAMEBYTES (FRAME_WIDTH * FRAME_HEIGHT *2)
#define FRAMEBUFFBYTES (FB_WIDTH *FB_HEIGHT *2)


extern bool verboseflag;
extern bool yuvmode;

#define MAXPIXIDX 84
extern uint32_t SamplePixIdx[];  // pixel indices for motion sampling
void AddStandardCommands(CMDHandler *cmdptr);
uint16_t Hex2Uint16(char *hex); 
