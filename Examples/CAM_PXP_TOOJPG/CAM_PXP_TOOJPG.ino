
//  t4.1 PIN input  and image packet test
//  Test toojpeg compression
// 

#include <MTP.h>
#include <Storage.h>
#include <usb1_mtp.h>
#include "OV7670.h"
#include <SD.h>
#include <CMDHandler.h>
#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include <stdio.h>
#include <cstdlib>
#include "CamStandardCommands.h"
#include "PXP.h"
#include "toojpeg.h"

uint32_t lastsample;


elapsedMillis bltimer = 0;
elapsedMicros cvtimer;
uint32_t cvmicros;

File jpgfile;

const char compileTime [] = " Compiled on " __DATE__ " " __TIME__;

const int ledpin = 33;
const int imarkpin = 32;
const int pinCamReset = 14;

#define  LEDON digitalWriteFast(ledpin, HIGH); // Also marks IRQ handler timing
#define  LEDOFF digitalWriteFast(ledpin, LOW);
#define  LEDTOGGLE digitalToggleFast(ledpin);

#define  IMARKHI digitalWriteFast(imarkpin, HIGH); // Also marks IRQ handler timing
#define  IMARKLO digitalWriteFast(imarkpin, LOW);
#define  IMARKTOGGLE digitalToggleFast(imarkpin);
CMDHandler mycmds;

MTPStorage_SD storage;
MTPD       mtpd(&storage);

uint8_t *rgb8 = (uint8_t *)0x70000000;
uint8_t *fb1 = (uint8_t *)0x70100000;
uint8_t *fb2 = (uint8_t *)0x70200000;
uint8_t *fcaptbuff = (uint8_t *)0x70300000;
uint8_t *fb4 = (uint8_t *)0x70400000;

timagetype lastimtype;
toutmode lastmode;

__inline uint8_t Red(uint16_t pixval) {
  return (uint8_t)(8 * ((pixval & 0xF800) >> 11));
}

__inline uint8_t Grn(uint16_t pixval) {
  return (uint8_t)(4 * ((pixval & 0x07E0) >> 5));
}

__inline uint8_t Blu(uint16_t pixval) {
  return (uint8_t)(8 * (pixval & 0x001F));
}


void setup() {
  Serial.begin(9600);
  delay(200);
  Wire.begin();

  delay(100);
  pinMode(ledpin, OUTPUT);
  pinMode(imarkpin, OUTPUT);
  pinMode(pinCamReset, OUTPUT);

  delay(10);


  StartMTP();
  digitalWriteFast(pinCamReset, LOW);
  delay(10);
  digitalWriteFast(pinCamReset, HIGH);  // subsequent resets via SCB


  if (OV7670.begin(VGA, fb1, fb2)) {
    Serial.println("OV7670 camera initialized.");
    Serial.printf("fb1 at   %p\n", fb1);
    Serial.printf("fb2 at    %p\n", fb2);
    Serial.printf("fcaptbuff at %p\n", fcaptbuff);
    Serial.printf("RGB888 buffer at %p\n", rgb8);
  }
  setSyncProvider(getTeensy3Time); // helps put time into file directory data

  StartSDCard();
  AddStandardCommands(&mycmds);

  mycmds.AddCommand(&CMSI, "SI", 0);
  mycmds.AddCommand(&CMGP, "GP", 1);
  mycmds.AddCommand(&CMPP, "PP", 0);
  mycmds.AddCommand(&CMJP, "JP", 1);
  mycmds.AddCommand(&CMRB, "RB", 0);
  mycmds.AddCommand(&CMDI, "DI", 0);
  mycmds.AddCommand(&CMFC, "FC", 1);
  InitPXP_Image(VGA, RGB565);
  lastimtype = VGA;
  lastmode = RGB565;
  CMSI(NULL);
}

void loop() {

  if (mycmds.CheckCommandInput()) {

  }
  mtpd.loop();
}

void StartMTP(void) {
  Serial.println("Starting MTP Responder");
  usb_mtp_configure();
  // NOTE: Next line requires the new SD implementation in TD 1.54B4
  if (!Storage_init(&SD.sdfs)) {
    Serial.println("Could not initialize MTP Storage!");
  }
}

// Disconnect USB to force PC MTP code to rebuilld the database
void CMRB(void) {

  LEDON
  Serial.println("Reconnect serial port or restart Serial Monitor after USB reset.");
  delay(100);
  usb_init();  // shuts down USB if already started, then restarts
  delay(200);
  Serial.begin(9600);
  delay(200);
  LEDOFF

  usb_mtp_configure();
  // NOTE: Next line requires the new SD implementation in TD 1.54B4
  if (!Storage_init(&SD.sdfs)) {
    Serial.println("Could not initialize MTP Storage!");
  }
  Serial.println("USB disconnected and reconnected to force MTP update");
}


void CMSI(void *cp) {

  //  cmdlinetype *cptr;
  //  cptr = (cmdlinetype *) cp;
  Serial.printf("\n\nOV7670 Camera JPG Test 3 %s\n", compileTime);
  OV7670.ShowCamConfig();
  Serial.printf("fcaptbuff: %p\n", fcaptbuff);

}

void CMDI(void *cp) {
  Serial.println("\nSD Card Directory");
  SD.sdfs.ls(LS_DATE | LS_SIZE);
  Serial.println();
}


#define JPGMAXSIZE 200000
static uint8_t jpgbuff[JPGMAXSIZE] DMAMEM;
static uint32_t jpgcount = 0;
uint8_t *wrptr;
static void myoutput(unsigned char bt) {
  if (jpgcount < JPGMAXSIZE) {
    *wrptr++ = bt;
    jpgcount++;
  }
}

// Modified to use toojpeg, which doesn't require tables in DMAMEM
void CMJP(void *cp) {
  elapsedMillis etm;
  uint32_t endtime1, endtime2;
  char *jpfnptr;
  bool jpresult;
  bool rgbfmt = true;
  bool dwnsample = false;
  uint8_t qual;
  cmdlinetype *cptr;
  cptr = (cmdlinetype *) cp;
  if (cptr->pstr[0] == NULL) { // no parameter
    qual = 90;
  } else { // valid parameter string
    qual = atoi(cptr->pstr[0]);
  }

  jpfnptr = NewFileName();
  jpgfile = SD.open(jpfnptr, FILE_WRITE);
  etm = 0;
  jpgcount = 0;
  wrptr = &jpgbuff[0];
  if (OV7670.OutMode() == YUV422) {
    TooJpeg::UseYUV(true);
    Serial.println("Using YUV directly");
    jpresult = TooJpeg::writeJpeg(&myoutput, fcaptbuff, 640, 480, rgbfmt, qual, dwnsample, NULL);
  } else {
    TooJpeg::UseYUV(false);
    Serial.println("Using RGB888");
    jpresult = TooJpeg::writeJpeg(&myoutput, rgb8, 640, 480, rgbfmt, qual, dwnsample, NULL);
  }
  endtime1 = etm;
  etm = 0;
  if (jpgcount < JPGMAXSIZE) {
    jpgfile.write(&jpgbuff, jpgcount);
    Serial.printf("Wrote %s with length %lu\n", jpfnptr, jpgcount);
  } else Serial.println("JPG buffer overflow");

  jpgfile.close();
  endtime2 = etm;
  Serial.printf("JPG file conversion  took %u mSec.   File write took %u mSec \n", endtime1, endtime2);
}

void CMPP(void *cp) {
  pxp.Show();
  pxp.PrintNext("YUV422_RBG888", pxp.PXPNext1) ;
}

void RGBLine(uint16_t line, uint16_t offset) {
  uint32_t i, x;
  uint16_t *fcp = (uint16_t *)&fcaptbuff[0];
  Serial.printf("FCaptBuff RGB line 2  Offset = %u\n", offset);
  for (i = 0; i < 8; i++) {
    x = (640 * (line - 1)) + ( i * 80 ) + offset / 2;
    Serial.printf("%3u ", Red(fcp[x]));
    Serial.printf("%3u ", Grn(fcp[x]));
    Serial.printf("%3u | ", Blu(fcp[x]));
    if (i == 3)Serial.println();
  }
  Serial.println();
}

void YUVLine(uint16_t line, uint16_t offset) {
  uint32_t i, x;
  Serial.printf("FCaptBuff YUV line 2  Offset = %u\n", offset);
  for (i = 0; i < 8; i++) {
    x = i * 160 + (1280 * (line - 1)) + offset;
    Serial.printf("%02X ", fcaptbuff[x]);
    Serial.printf("%02x ", fcaptbuff[x + 1]);
    Serial.printf("%02X ", fcaptbuff[x + 2]);
    Serial.printf("%02x | ", fcaptbuff[x + 3]);
    if (i == 3)Serial.println();
  }
  Serial.println();
}

//  Show bytes 2,3, 162,163, 322,323, ... in 2nd line of fcaptbuff
void CMFC(void *cp) {
  uint16_t i, x, ofs;
  cmdlinetype *cptr;
  cptr = (cmdlinetype *) cp;
  if (cptr->pstr[0] == NULL) { // no parameter
    ofs = 20;
  } else { // valid parameter string
    ofs = atoi(cptr->pstr[0]);
  }
  if (OV7670.OutMode() == YUV422) YUVLine(2, ofs);
  if (OV7670.OutMode() == RGB565) RGBLine(2, ofs);
}



// Send an RBG Frame of the currently specified size to pc

void CMGP(void *cp) {
  uint32_t imagesize, endtime;
  elapsedMillis etimer;

  static int32_t lasttime = 0;

  imagesize = OV7670.ImageSize();
  timagetype cimtype = OV7670.GetImageType();
  OV7670.CaptureFrame(fcaptbuff);
  do {
    delay(1);
  } while (OV7670.CaptureRunning());

  if (now() != lasttime) {
    lasttime = now();
  }
  uint32_t *tsptr = (uint32_t *)fcaptbuff;
  // put the time stamp into first 4 bytes of image
  *tsptr = lasttime;

  Serial.write((uint8_t*)fcaptbuff, imagesize);

  etimer = 0;

  InitPXP_Image(cimtype, OV7670.OutMode());
  pxp.SetOutput((uint16_t *)rgb8, 640, 480, PXP_RGB888P);
  PXP_OUT_PITCH = 640 * 3;
  pxp.Start();
  // wait until conversion finished
  while (!pxp.Done()) {};
  pxp.Stop();
  endtime = etimer;
  Serial.printf("RGB888 conversion took %lu mSec.\n", endtime); \

}



// Set up the PXP to receive YUV in fcaptbuff and convert to VGA RGB888  at rgb8 in EXTMEM

// Set up the PXP to receive YUV in fcaptbuff and convert to VGA RGB888  at rgb8 in EXTMEM
void InitPXP_Image( timagetype  citype, toutmode omode) {
  // set input size and scaling based on image size
   // modified to use latest PXP library 1/26/2021
  pxp.begin();  // start clocks, etc.
  if (citype == QVGA) {
    pxp.SetPS((uint16_t *)fcaptbuff,  320, 240, PXP_RGB565 );
    pxp.SetOutput( (uint16_t *)rgb8, 640, 480, PXP_RGB888);
    // now set scaling for 2x H and V
    pxp.SetScale(2.0);
  }
  if (citype == VGA) {  
    pxp.SetPS((uint16_t *)fcaptbuff,  640, 480, PXP_RGB565 );
    pxp.SetOutput( (uint16_t *)rgb8, 640, 480, PXP_RGB888);
    // now set scaling for 2x H and V
    pxp.SetScale(1.0);
  }
  if (citype == QQVGA) {
    pxp.SetPS((uint16_t *)fcaptbuff,  160, 120, PXP_RGB565 );
    pxp.SetOutput( (uint16_t *)rgb8, 640, 480, PXP_RGB888);
    // now set scaling for 4x H and V
    pxp.SetScale(4.0);
  }
  if (omode == YUV422) { // convert YUV to RGB888
    PXP_PS_CTRL_CLR = PXP_PS_CTRL;  // Clear PS buffer format
    PXP_PS_CTRL_SET = PXP_UYVY1P422 | PXP_PS_CTRL_WB_SWAP ;  // PS buffer format is interleaved UYVY
    //  Works with camera mode YM 2
    PXP_CSC1_COEF0  = PXP_COEF0_UV_OFFSET(-128) | PXP_COEF0_C0(0x12A) | PXP_COEF0_YCBCR_MODE ;
    PXP_CSC1_COEF1 = (0x01230208); // YUV coefficients: C1, C4
    PXP_CSC1_COEF2 = (0x076B079C); // YUV coefficients: C2, C3
    Serial.println("PS_CTRL set to YUV422");
  } else {  // PS is RGB565
    PXP_PS_CTRL_CLR = PXP_PS_CTRL;  // Clear PS buffer format
    PXP_PS_CTRL_SET = PXP_RGB565 ;  // PS buffer format is RGB565
    Serial.println("PS_CTRL set to RGB565");
  }

  pxp.SaveNext(pxp.PXPNext1);
}

/*****************************************************************************
   Read the Teensy RTC and return a time_t (Unix Seconds) value

 ******************************************************************************/
time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

void MakeTimeStamp(char *str) {
  time_t nn;
  nn = now();
  int yr = year(nn);
  int mo = month(nn);
  int dd = day(nn);
  int hh = hour(nn);
  int mn = minute(nn);
  int se = second(nn);
  sprintf(str, "%02d/%02d/%04d  %02d:%02d:%02d", mo, dd, yr, hh, mn, se);

}

//------------------------------------------------------------------------------
/*
   User provided date time callback function.
   See SdFile::dateTimeCallback() for usage.
*/

void dateTime(uint16_t* date, uint16_t* time) {
  // use the year(), month() day() etc. functions from timelib

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(year(), month(), day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour(), minute(), second());
}

bool StartSDCard() {

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("\nSD File initialization failed.\n");
    return false;
  } else  Serial.println("initialization done.");

  // set date time callback function
  SdFile::dateTimeCallback(dateTime);
  return true;
}

char* NewFileName(void) {
  static char fname[32];
  time_t nn;
  nn = now();
  int mo = month(nn);
  int dd = day(nn);
  int hh = hour(nn);
  int mn = minute(nn);
  int ss = second(nn);
  sprintf(fname, "JP_%02d%02d%02d%02d%02d.jpg", mo, dd, hh, mn, ss);
  return &fname[0];
}
