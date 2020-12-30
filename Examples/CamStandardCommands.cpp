/********************************************************
  Standard commands for use with OV7670 library
  Separated out to reduce size of calling sketch.
********************************************************/
#include "OV7670.h"
#include <CMDHandler.h>
#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include <stdio.h>
#include <cstdlib>
#include "CamStandardCommands.h"


bool verboseflag = false;
bool cbarmode = false;
bool yuvmode = false;

uint8_t oldregs[256], newregs[256];
//  This function is used in functions that set
//  camera registers using hex input
uint16_t Hex2Uint16(char *hex) {
  uint16_t val = 0;
  while (*hex) {
    // get current character then increment
    uint8_t byte = *hex++;
    // transform hex character to the 4bit equivalent number, using the ascii table indexes
    if (byte >= '0' && byte <= '9') byte = byte - '0';
    else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
    else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
    // shift 4 to make space for new digit, and add the 4 bits of the new digit
    val = (val << 4) | (byte & 0xF);
  }
  return val;
}

// set or clear verbose flag
void CMVB(void *cp) {
  cmdlinetype *cptr;
  uint16_t fi;
  cptr = (cmdlinetype *) cp;

  if (cptr->pstr[0] == NULL) { // no parameter
    verboseflag = true;
    return;
  } else { // valid parameter string
    fi = atoi(cptr->pstr[0]);
  }
  if (fi) verboseflag = true; else verboseflag = false;

}


// Switch the CSI external clock frequency.
// Valid inputs are 12 and 24.  everything else does not change clock

// Change the CSI XCLK
void CMXC(void *cp) {
  cmdlinetype *cptr;
  uint16_t xc;
  cptr = (cmdlinetype *) cp;

  if (cptr->pstr[0] == NULL) { // no parameter
    return;
  } else { // valid parameter string
    xc  = atoi(cptr->pstr[0]);
    if (xc ==  12) {
      Serial.println("Changing CSI XCLK to 12MHz");
      OV7670.SetCSIClock(CSI12);
      return;
    }
    if (xc == 24) {
      Serial.println("Changing CSI XCLK to 24MHz");
      OV7670.SetCSIClock(CSI24);
      return;
    }
    Serial.println("Invalid ");

  }
}


// Change the camera window size
void CMWS(void *cp) {
  cmdlinetype *cptr;
  uint16_t ws;
  bool result = false;
  cptr = (cmdlinetype *) cp;

  if (cptr->pstr[0] == NULL) { // no parameter
    return;
  } else { // valid parameter string
    ws = atoi(cptr->pstr[0]);
    if (ws ==  VGA) {
      result =  OV7670.SetResolution(VGA);
    }
    if (ws == QVGA) {
      result =   OV7670.SetResolution(QVGA);
    }
    if (ws == QQVGA) {
      result =  OV7670.SetResolution(QQVGA);
    }
    if (result) {
      Serial.print("Changed resolution to ");
      if (OV7670.GetImageType() == VGA) Serial.print("VGA");
      if (OV7670.GetImageType() == QVGA) Serial.print("QVGA");
      if (OV7670.GetImageType() == QQVGA) Serial.print("QQVGA");
      Serial.println();
    } else Serial.println("Unable to change resolution.");
  }

}



// Set the camera clock frequency
void CMCC(void *cp) {
  cmdlinetype *cptr;
  uint16_t clk;
  cptr = (cmdlinetype *) cp;
  if (cptr->pstr[0] == NULL) { // no parameter
    return;
  } else { // valid parameter string
    clk = atof(cptr->pstr[0]);
    OV7670.SetCamClock(clk);
  }
  Serial.printf("Camera Clock set to %5.1fMHz\n", OV7670.GetCamClock());
}

// Set contrast
void CMCT(void *cp) {
  cmdlinetype *cptr;
  uint8_t ct;
  cptr = (cmdlinetype *) cp;
  if (cptr->pstr[0] == NULL) { // no parameter
    return;
  } else { // valid parameter string
    ct = atoi(cptr->pstr[0]);
    OV7670.SetContrast(ct);
    Serial.printf("Contrast set to %u\n", ct);
  }

}

//Set Brightness
void CMBR(void *cp) {
  cmdlinetype *cptr;
  uint8_t br;
  cptr = (cmdlinetype *) cp;
  if (cptr->pstr[0] == NULL) { // no parameter
    return;
  } else { // valid parameter string
    br = atoi(cptr->pstr[0]);
    OV7670.SetBrightness(br);
    Serial.printf("Brightness set to %u\n", br);
  }

}


// Set LED compensation
void CMLE(void *cp) {
  cmdlinetype *cptr;
  bool bval;
  cptr = (cmdlinetype *) cp;
  if (cptr->pstr[0] == NULL) { // no parameter
    return;
  } else { // valid parameter string
    bval  = atoi(cptr->pstr[0]) != 0;
    OV7670.SetLEDComp(bval);
  }
  Serial.print("LED Compensation ");
  if (bval) Serial.println("ON"); else Serial.println("OFF");
}


// Set a Camera Register
void CMSR(void *cp) {
  cmdlinetype *cptr;
  char *sp, *vp;
  uint8_t reg, rval;

  cptr = (cmdlinetype *) cp;
  sp = (char *) cptr->pstr[0];
  vp = (char *) cptr->pstr[1];
  // Serial.printf("&cptr->pstr[0] = %p\n", &cptr->pstr[0]);
  if (sp == NULL) { // no parameter, show all
    //cameraReadI2C();
    Serial.println();
  } else { // valid parameter string
    reg = Hex2Uint16(sp);
    if (reg > 0xC9) {
      Serial.println("Invalid register number.");
    } else { // get value to send to register
      if (vp != NULL) {
        rval = Hex2Uint16(vp);
        OV7670.WriteRegister(reg, rval);
        Serial.printf("Wrote register 0x%02X with value 0x%02X\n", reg, rval);
      } else {
        Serial.println("Invalid register value");
      }
    }

  }

}


// Frame Settings for HSTART, HSTOP, VSTART, VSTOP
void CMFS(void *cp) {
  cmdlinetype *cptr;
  char *sp, *vp;
  uint16_t reg, rval, href, vref;
  cptr = (cmdlinetype *) cp;
  sp = (char *) cptr->pstr[0];
  vp = (char *) cptr->pstr[1];
  href = OV7670.ReadRegister(0x32);
  vref = OV7670.ReadRegister(0x03);
  if (sp == NULL) return; // no parameter, exit
  if (vp == NULL) return;
  // valid parameter strings
  reg = atoi(sp);
  rval = atoi(vp);
  Serial.printf("in CMFS reg:%u  val: %u  href: %02X  vref %02X\n", reg, rval, href, vref);
  switch (reg) {
    case 1: // HSTART
      OV7670.WriteRegister(0x17, rval >> 3);
      href = (href & ~0x07) | (rval & 0x07) | 0x80; // add some delay after HREF active
      OV7670.WriteRegister(0x32, href);
      break;
    case 2: //HSTOP
      OV7670.WriteRegister(0x18, rval >> 3);
      href = (href & ~0x38) | ((rval & 0x07) << 3);
      OV7670.WriteRegister(0x32, href);
      break;
    case 3: // VSTART
      OV7670.WriteRegister(0x19, rval >> 2);
      vref = (vref & ~0x03) | (rval & 0x03);
      OV7670.WriteRegister(0x03, vref);
      break;
    case 4: // VSTOP
      OV7670.WriteRegister(0x1A, rval >> 2);
      vref = (vref & ~0x0C) | ((rval & 0x03) << 2);
      OV7670.WriteRegister(0x03, vref);
      break;

  }  //  end of switch(reg)
}

// get frame data:  H and V start and stop
void CMFD(void *cp) {
  uint8_t hsh, hl, heh;
  uint8_t vsh, vl, veh;
  uint16_t hs, he, vs, ve;
  hsh = OV7670.ReadRegister(0x17); heh =  OV7670.ReadRegister(0x18);
  hl =  OV7670.ReadRegister(0x32);
  vsh = OV7670.ReadRegister(0x19); veh =  OV7670.ReadRegister(0x1A);
  vl =  OV7670.ReadRegister(0x3);
  hs = (hsh << 3) + (hl & 0x07);
  he = (heh << 3) + (hl & 0x38) / 8;
  vs = (vsh << 2) + (vl & 0x03);
  ve = (veh << 2) + (vl & 0x0C) / 4;
  Serial.printf("HStart: %u  HStop: %u\n", hs, he);
  Serial.printf("VStart: %u  VStop: %u\n", vs, ve);

}

// Show Camera Registers
void CMSC(void *cp) {
  cmdlinetype *cptr;
  char *sp;
  uint8_t reg;
  uint16_t hexval;
  cptr = (cmdlinetype *) cp;
  sp = (char *) cptr->pstr[0];

  //Serial.printf("In CMSc sp = %p\n",sp);
  //delay(5);
  if (sp == NULL) { // no parameter, show all
    OV7670.ReadAll(oldregs);
    OV7670.ShowAll(oldregs);
    delay(10);
    Serial.println("\n");
  } else { // valid parameter string

    //    delay(15);
    hexval = Hex2Uint16(sp);
    //    Serial.printf("Hex2Uint16 returned %ld\n",hexval);
    //    delay(5);
    if (hexval > 0xC9) {
      Serial.println("Invalid register number.");
    } else {
      reg = OV7670.ReadRegister(hexval);

      Serial.printf("Register 0x%02X = 0x%02X\n", hexval, reg );
    }
  }

}

void CMCS(void *cp) {  // Show CSI registers
  OV7670.ShowCSIRegisters();;
}



void CMRG(void *cp) {
  OV7670.SetOutMode(RGB565);
  

}


void CMYU(void *cp) {
  OV7670.SetOutMode(YUV422);
 

}


// Set the YUV byte order
void CMYM(void *cp) {
  cmdlinetype *cptr;
  uint16_t ym;
  uint8_t reg3a, reg3d;
  cptr = (cmdlinetype *) cp;
  if (cptr->pstr[0] == NULL) { // no parameter
    return;
  } else { // valid parameter string
    ym = atoi(cptr->pstr[0]) & 0x03;
    reg3a = OV7670.ReadRegister(0x3a) & ~0x08;  // clear bit 3
    reg3d = OV7670.ReadRegister(0x3d) & ~0x01; // clear lsb
    if (ym & 0x02) reg3a |= 0x08 ; // set bit 3 in TSLB
    if (ym & 0x01) reg3d |= 0x01 ; // set bit 0 in COM12
    OV7670.WriteRegister(0x3A, reg3a);
    OV7670.WriteRegister(0x3D, reg3d);

  }
  Serial.printf("YUV Byte order %u\n", ym);
}



// Set/clear the color bar test bit
void CMCB(void *cp) {
  cmdlinetype *cptr;
  uint16_t cb;
  uint8_t rval;
  cptr = (cmdlinetype *) cp;
  if (cptr->pstr[0] == NULL) { // no parameter
    return;
  } else { // valid parameter string
    cb = atoi(cptr->pstr[0]);
    if (cb == 1) {
      rval = OV7670.ReadRegister(0x71);
      rval |= 0x80;
      OV7670.WriteRegister(0x71, rval);
      cbarmode = true;
    } else {
      rval = OV7670.ReadRegister(0x71);
      rval &= ~0x80;
      OV7670.WriteRegister(0x71, rval);
      cbarmode = false;
    }
  }
}



void AddStandardCommands(CMDHandler *cmdptr) {
  cmdptr->AddCommand(&CMBR, "BR", 1);
  cmdptr->AddCommand(&CMCT, "CT", 1);
  cmdptr->AddCommand(&CMLE, "LE", 1);
  cmdptr->AddCommand(&CMFD, "FD", 0);
  cmdptr->AddCommand(&CMSC, "SC", 1);
  cmdptr->AddCommand(&CMCC, "CC", 1);
  cmdptr->AddCommand(&CMSR, "SR", 2);
  cmdptr->AddCommand(&CMFS, "FS", 2);
  cmdptr->AddCommand(&CMCS, "CS", 0);
  cmdptr->AddCommand(&CMCC, "CC", 1);
  cmdptr->AddCommand(&CMXC, "XC", 1);
  cmdptr->AddCommand(&CMWS, "WS", 1);
  cmdptr->AddCommand(&CMVB, "VB", 1);
  cmdptr->AddCommand(&CMCB, "CB", 1);
  cmdptr->AddCommand(&CMYM, "YM", 1);
  cmdptr->AddCommand(&CMYU, "YU", 0);
  cmdptr->AddCommand(&CMRG, "RG", 0);
}
