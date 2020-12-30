
//  t4.1 PIN input  and image packet test
//  New CameraBB with new pinouts for wired breadboard
//  MJB  10/17/2020

#include "OV7670.h"
//#include "SdFat.h"
//#include "sdios.h"
#include <CMDHandler.h>
#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include <stdio.h>
#include <cstdlib>
#include "CamStandardCommands.h"
#include "PXP.h"

#include <ILI9341_t3n.h>
#include "ili9341_t3n_font_Arial.h"



//Specify the pins used for Non-SPI functions
#define TFT_CS   10  // AD_B0_02
#define TFT_DC   9  // AD_B0_03
#define TFT_RST  8

ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST);

uint32_t lastsample;


elapsedMillis bltimer = 0;
elapsedMicros cvtimer;
uint32_t cvmicros;

//SdFs sd;

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



uint8_t *fb0 = (uint8_t *)0x70000000;
uint8_t *fb1 = (uint8_t *)0x70100000;
uint8_t *fb2 = (uint8_t *)0x70200000;
uint8_t *fcaptbuff = (uint8_t *)0x70300000;

uint8_t ilibuff[320 * 240 * 2]DMAMEM;

timagetype lastimtype;
toutmode lastmode;

void setup() {
  Serial.begin(9600);
  delay(200);
  Wire.begin();

  delay(100);
  pinMode(ledpin, OUTPUT);
  pinMode(imarkpin, OUTPUT);
  pinMode(pinCamReset, OUTPUT);

  delay(10);


  digitalWriteFast(pinCamReset, LOW);
  delay(10);
  digitalWriteFast(pinCamReset, HIGH);  // subsequent resets via SCB

  memset(fb0, 0x55, 0x100000);

  if (OV7670.begin(VGA, fb1, fb2)) {
    Serial.println("OV7670 camera initialized.");
    Serial.printf("fb1 at   %p\n", fb1);
    Serial.printf("fb2 at    %p\n", fb2);
    Serial.printf("fcaptbuff at %p\n", fcaptbuff);
  }
  setSyncProvider(getTeensy3Time); // helps put time into file directory data
  // Start ILI9341
  tft.begin();
  tft.setRotation(3);  // Need rot = 3 for right-side up on MJB proto board

  AddStandardCommands(&mycmds);

  mycmds.AddCommand(&CMSI, "SI", 0);
  mycmds.AddCommand(&CMGP, "GP", 1);
  mycmds.AddCommand(&CMGF, "GF", 0);
  mycmds.AddCommand(&CMPP, "PP", 0);
  InitPXP_Image(VGA, RGB565);
  lastimtype = VGA;
  lastmode = RGB565;
  CMSI(NULL);

}

void loop() {

  if (mycmds.CheckCommandInput()) {

  }

}


void ILIShowFrame(void *bufptr ) {
  static char tmsg[32] = "01/01/2020 01:00:00";

  MakeTimeStamp(tmsg);

  tft.setFont(Arial_12);
  tft.writeRect(0, 0, tft.width(), tft.height(), (uint16_t *)bufptr);
  tft.setTextColor(ILI9341_GREEN);
  tft.drawString(tmsg, 5, 226);
}


void CMSI(void *cp) {

  //  cmdlinetype *cptr;
  //  cptr = (cmdlinetype *) cp;
  Serial.printf("\n\nOV7670 Camera Test 3 %s\n", compileTime);
  OV7670.ShowCamConfig();
  Serial.printf("fcaptbuff: %p\n", fcaptbuff);

}

void CMPP(void *cp) {

  pxp.Show();
  pxp.PrintNext("YUV422_RBG565", pxp.PXPNext1) ;

}


// Send an RBG Frame of the currently specified size to pc

void CMGP(void *cp) {
  uint32_t imagesize;
  static int32_t lasttime = 0;
  timagetype cimtype;
  imagesize = OV7670.ImageSize();
  cimtype = OV7670.GetImageType();
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


  cvtimer = 0;
  if ((cimtype != lastimtype) | (lastmode != OV7670.OutMode())) { // need to change scaling
    InitPXP_Image(cimtype, OV7670.OutMode());
    lastimtype = cimtype;
    lastmode = OV7670.OutMode();
  }
  PXP_STAT_CLR = PXP_STAT;  // clears all flags
  PXP_CTRL_SET =  PXP_CTRL_ENABLE;  // start the PXP
  // wait until conversion finished
  while (!pxp.Done()) {};
  PXP_CTRL_CLR =  PXP_CTRL_ENABLE;  // stop the PXP
  ILIShowFrame((void *)&ilibuff );
}

// Capture a frame and send only to TFT
void CMGF(void *cp) {

  timagetype cimtype;
  static int32_t lasttime = 0;

  cimtype = OV7670.GetImageType();
  Serial.printf("Converting PXP image size %d\n", cimtype);
  OV7670.CaptureFrame(fcaptbuff);
  do {
    delay(1);
  } while (OV7670.CaptureRunning());

  if (now() != lasttime) {
    lasttime = now();
  }
  // put the time stamp into first 4 bytes of image
  uint32_t *tsptr = (uint32_t *)fcaptbuff;
  // put the time stamp into first 4 bytes of image
  *tsptr = lasttime;
  cvtimer = 0;

  if ((cimtype != lastimtype) | (lastmode != OV7670.OutMode())) { // need to change scaling and PS Format
    Serial.printf("Switching PXP to image size %d\n", cimtype);
    InitPXP_Image(cimtype, OV7670.OutMode());
    lastimtype = cimtype;
    lastmode = OV7670.OutMode();
  }
  PXP_STAT_CLR = PXP_STAT;  // clears all flags
  PXP_CTRL_SET =  PXP_CTRL_ENABLE;  // start the PXP
  // wait until conversion finished
  while (!pxp.Done()) {};
  PXP_CTRL_CLR =  PXP_CTRL_ENABLE;  // stop the PXP
  cvmicros = cvtimer;
  Serial.printf("PXP Conversion took %lu microseconds\n", cvmicros);
  ILIShowFrame((void *)&ilibuff );

}

// Set up the PXP to receive YUV in fcaptbuff and convert to RGB565 in ilibuff
void InitPXP_Image( timagetype  citype, toutmode omode) {
  // set input size and scaling based on image size

  if (citype == QVGA) {
    pxp.begin((uint16_t *)fcaptbuff, (uint16_t *)ilibuff,  320, 240, 320, 240 );
    // now set scaling for 1x H and V
    pxp.SetScale(1.0);
  }
  if (citype == VGA) {
    pxp.begin((uint16_t *)fcaptbuff, (uint16_t *)ilibuff,  640, 480, 320, 240 );
    // now set scaling to reduce by 2X H and V
    pxp.SetScale(0.5);
  }
  if (citype == QQVGA) {
    pxp.begin((uint16_t *)fcaptbuff, (uint16_t *)ilibuff,  160, 120, 320, 240 );
    // now set scaling to multiply by 2X H and V
    pxp.SetScale(2.0);
  }
  if (omode == YUV422) { // convert YUV to RGB565
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
