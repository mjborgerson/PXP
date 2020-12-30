/***************************************************************************/
/*                                                                         */
/*  File: savejpg.h                                                        */
/*  Author: bkenwright@xbdev.net                                           */
/*  URL: www.xbdev.net                                                     */
/*  Date: 19-01-06                                                         */
/*                                                                         */
/***************************************************************************/
/*
	Tiny Simplified C source of a JPEG encoder.
	A BMP truecolor to JPEG encoder

	.bmp -> *.jpg
*/
/***************************************************************************/
#include <arduino.h>
#include <cstdio>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//#include <systemc.h>
#include <SD.h>
#include "OV7670.h"    // needed for toutmode
#include "savejpgA.h"



/***************************************************************************/

static uint8_t bytenew = 0; // The byte that will be written in the JPG file
static int8_t bytepos = 7; //bit position in the byte we write (bytenew)
//should be<=7 and >=0
static uint16_t mask[16] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};

// The Huffman tables we'll use:
static bitstring YDC_HT[12];
static bitstring CbDC_HT[12];
static bitstring YAC_HT[256];
static bitstring CbAC_HT[256];

bitstring bitdata[65536] DMAMEM;
uint8_t catdata[65536] DMAMEM;

static uint8_t *category; //Here we'll keep the category of the numbers in range: -32767..32767
static bitstring *bitcode; // their bitcoded representation

//Precalculated tables for a faster YCbCr->RGB transformation
// We use a int32_t table because we'll scale values by 2^16 and work with integers
static int32_t YRtab[256], YGtab[256], YBtab[256];
static int32_t CbRtab[256], CbGtab[256], CbBtab[256];
static int32_t CrRtab[256], CrGtab[256], CrBtab[256];
static float fdtbl_Y[64];
static float fdtbl_Cb[64]; //the same with the fdtbl_Cr[64]

static colorRGB *RGB_buffer; //image to be encoded
static uint16_t *YUV_buffer;

static uint16_t Ximage, Yimage; // image dimensions divisible by 8
static int8_t YDU[64]; // This is the Data Unit of Y after Rgb->YCbCr transformation
static int8_t CbDU[64];
static int8_t CrDU[64];
static int16_t DU_DCT[64]; // Current DU (after DCT and quantization) which we'll zigzag
static int16_t DU[64]; //zigzag reordered DU which will be Huffman coded

File  fp_jpeg_stream;

#define JPGMAXSIZE 100000
static uint8_t jpgbuff[JPGMAXSIZE];
static uint32_t jpgcount = 0;

uint8_t *wrptr;
void inline writebyte(uint8_t b) {
  if (jpgcount < JPGMAXSIZE) {
    *wrptr++ = b;
    jpgcount++;
  }
}

#define writeword(w) writebyte((w)/256);writebyte((w)%256);


/***************************************************************************/


void write_APP0info()
//Nothing to overwrite for APP0info
{
  writeword(APP0info.marker);
  writeword(APP0info.length);
  writebyte('J'); writebyte('F'); writebyte('I'); writebyte('F'); writebyte(0);
  writebyte(APP0info.versionhi); writebyte(APP0info.versionlo);
  writebyte(APP0info.xyunits);
  writeword(APP0info.xdensity); writeword(APP0info.ydensity);
  writebyte(APP0info.thumbnwidth); writebyte(APP0info.thumbnheight);
}

void write_SOF0info()
// We should overwrite width and height
{
  writeword(SOF0info.marker);
  writeword(SOF0info.length);
  writebyte(SOF0info.precision);
  writeword(SOF0info.height); writeword(SOF0info.width);
  writebyte(SOF0info.nrofcomponents);
  writebyte(SOF0info.IdY); writebyte(SOF0info.HVY); writebyte(SOF0info.QTY);
  writebyte(SOF0info.IdCb); writebyte(SOF0info.HVCb); writebyte(SOF0info.QTCb);
  writebyte(SOF0info.IdCr); writebyte(SOF0info.HVCr); writebyte(SOF0info.QTCr);
}

void write_DQTinfo()
{
  uint8_t i;
  writeword(DQTinfo.marker);
  writeword(DQTinfo.length);
  writebyte(DQTinfo.QTYinfo); for (i = 0; i < 64; i++) writebyte(DQTinfo.Ytable[i]);
  writebyte(DQTinfo.QTCbinfo); for (i = 0; i < 64; i++) writebyte(DQTinfo.Cbtable[i]);
}

void set_quant_table(uint8_t *basic_table, uint8_t scale_factor, uint8_t *newtable) {
  // Set quantization table and zigzag reorder it

  uint8_t i;
  long temp;
  for (i = 0; i < 64; i++) {
    temp = ((long) basic_table[i] * scale_factor + 50L) / 100L;
    //limit the values to the valid range
    if (temp <= 0L) temp = 1L;
    if (temp > 255L) temp = 255L; //limit to baseline range if requested
    newtable[zigzag[i]] = (uint8_t) temp;
  }
}

void set_DQTinfo()
{
  uint8_t scalefactor = 50; // scalefactor controls the visual quality of the image
  // the smaller is, the better image we'll get, and the smaller
  // compression we'll achieve
  DQTinfo.marker = 0xFFDB;
  DQTinfo.length = 132;
  DQTinfo.QTYinfo = 0;
  DQTinfo.QTCbinfo = 1;
  set_quant_table(std_luminance_qt, scalefactor, DQTinfo.Ytable);
  set_quant_table(std_chrominance_qt, scalefactor, DQTinfo.Cbtable);
}

void write_DHTinfo()
{
  uint8_t i;
  writeword(DHTinfo.marker);
  writeword(DHTinfo.length);
  writebyte(DHTinfo.HTYDCinfo);
  for (i = 0; i < 16; i++)  writebyte(DHTinfo.YDC_nrcodes[i]);
  for (i = 0; i <= 11; i++) writebyte(DHTinfo.YDC_values[i]);
  writebyte(DHTinfo.HTYACinfo);
  for (i = 0; i < 16; i++)  writebyte(DHTinfo.YAC_nrcodes[i]);
  for (i = 0; i <= 161; i++) writebyte(DHTinfo.YAC_values[i]);
  writebyte(DHTinfo.HTCbDCinfo);
  for (i = 0; i < 16; i++)  writebyte(DHTinfo.CbDC_nrcodes[i]);
  for (i = 0; i <= 11; i++)  writebyte(DHTinfo.CbDC_values[i]);
  writebyte(DHTinfo.HTCbACinfo);
  for (i = 0; i < 16; i++)  writebyte(DHTinfo.CbAC_nrcodes[i]);
  for (i = 0; i <= 161; i++) writebyte(DHTinfo.CbAC_values[i]);
}

void set_DHTinfo()
{
  uint8_t i;
  DHTinfo.marker = 0xFFC4;
  DHTinfo.length = 0x01A2;
  DHTinfo.HTYDCinfo = 0;
  for (i = 0; i < 16; i++)  DHTinfo.YDC_nrcodes[i] = std_dc_luminance_nrcodes[i + 1];
  for (i = 0; i <= 11; i++)  DHTinfo.YDC_values[i] = std_dc_luminance_values[i];
  DHTinfo.HTYACinfo = 0x10;
  for (i = 0; i < 16; i++)  DHTinfo.YAC_nrcodes[i] = std_ac_luminance_nrcodes[i + 1];
  for (i = 0; i <= 161; i++) DHTinfo.YAC_values[i] = std_ac_luminance_values[i];
  DHTinfo.HTCbDCinfo = 1;
  for (i = 0; i < 16; i++)  DHTinfo.CbDC_nrcodes[i] = std_dc_chrominance_nrcodes[i + 1];
  for (i = 0; i <= 11; i++)  DHTinfo.CbDC_values[i] = std_dc_chrominance_values[i];
  DHTinfo.HTCbACinfo = 0x11;
  for (i = 0; i < 16; i++)  DHTinfo.CbAC_nrcodes[i] = std_ac_chrominance_nrcodes[i + 1];
  for (i = 0; i <= 161; i++) DHTinfo.CbAC_values[i] = std_ac_chrominance_values[i];
}

void write_SOSinfo()
//Nothing to overwrite for SOSinfo
{
  writeword(SOSinfo.marker);
  writeword(SOSinfo.length);
  writebyte(SOSinfo.nrofcomponents);
  writebyte(SOSinfo.IdY); writebyte(SOSinfo.HTY);
  writebyte(SOSinfo.IdCb); writebyte(SOSinfo.HTCb);
  writebyte(SOSinfo.IdCr); writebyte(SOSinfo.HTCr);
  writebyte(SOSinfo.Ss); writebyte(SOSinfo.Se); writebyte(SOSinfo.Bf);
}

void write_comment(uint8_t *comment)
{
  uint16_t i, length;
  writeword(0xFFFE); //The COM marker
  length = (uint16_t)strlen((const char *)comment);
  writeword(length + 2);
  for (i = 0; i < length; i++) writebyte(comment[i]);
}

void writebits(bitstring bs)
// A portable version; it should be done in assembler
{
  uint16_t value;
  int8_t posval;//bit position in the bitstring we read, should be<=15 and >=0
  value = bs.value;
  posval = bs.length - 1;
  while (posval >= 0)
  {
    if (value & mask[posval])
      bytenew |= mask[bytepos];

    posval--; bytepos--;

    if (bytepos < 0) {
      if (bytenew == 0xFF) {
        writebyte(0xFF); writebyte(0);
      }
      else {
        writebyte(bytenew);
      }

      bytepos = 7; bytenew = 0;
    }
  }
}

void compute_Huffman_table(uint8_t *nrcodes, uint8_t *std_table, bitstring *HT)
{
  uint8_t k, j;
  uint8_t pos_in_table;
  uint16_t codevalue;
  codevalue = 0; pos_in_table = 0;
  for (k = 1; k <= 16; k++)
  {
    for (j = 1; j <= nrcodes[k]; j++) {
      HT[std_table[pos_in_table]].value = codevalue;
      HT[std_table[pos_in_table]].length = k;
      pos_in_table++;
      codevalue++;
    }
    codevalue *= 2;
  }
}
void init_Huffman_tables()
{
  compute_Huffman_table(std_dc_luminance_nrcodes, std_dc_luminance_values, YDC_HT);
  compute_Huffman_table(std_dc_chrominance_nrcodes, std_dc_chrominance_values, CbDC_HT);
  compute_Huffman_table(std_ac_luminance_nrcodes, std_ac_luminance_values, YAC_HT);
  compute_Huffman_table(std_ac_chrominance_nrcodes, std_ac_chrominance_values, CbAC_HT);
}

void exitmessage(char *error_message)
{
  Serial.printf("%s\n", error_message); exit(EXIT_FAILURE);
}

void set_numbers_category_and_bitcode()
{
  int32_t nr;
  int32_t nrlower, nrupper;
  uint8_t cat;

  // category and bitcode data reside in arrays in DMAMEM
  // Arrays in DMAMEM are not initialized, but we fill in the
  // values here

  category = &catdata[32767]; //allow negative subscripts
  bitcode = &bitdata[32767];
  nrlower = 1; nrupper = 2;
  for (cat = 1; cat <= 15; cat++) {
    //Positive numbers
    for (nr = nrlower; nr < nrupper; nr++)
    { category[nr] = cat;
      bitcode[nr].length = cat;
      bitcode[nr].value = (uint16_t)nr;
      //<<JC>>
      //printf("bitcode[%d].length = %d, bitcode[%d].value = %d\n", nr, cat, nr, bitcode[nr].value);
    }
    //Negative numbers
    for (nr = -(nrupper - 1); nr <= -nrlower; nr++)
    { category[nr] = cat;
      bitcode[nr].length = cat;
      bitcode[nr].value = (uint16_t)(nrupper - 1 + nr);
      //<<JC>>
      //printf("bitcode[%d].length = %d, bitcode[%d].value = %d\n", nr, cat, nr, bitcode[nr].value);
    }
    nrlower <<= 1;
    nrupper <<= 1;
  }
}

void precalculate_YCbCr_tables()
{
  int16_t R, G, B;
  for (R = 0; R <= 255; R++) {
    YRtab[R] = (int32_t)(65536 * 0.299 + 0.5) * R;
    CbRtab[R] = (int32_t)(65536 * -0.16874 + 0.5) * R;
    CrRtab[R] = (int32_t)(32768) * R;
  }
  for (G = 0; G <= 255; G++) {
    YGtab[G] = (int32_t)(65536 * 0.587 + 0.5) * G;
    CbGtab[G] = (int32_t)(65536 * -0.33126 + 0.5) * G;
    CrGtab[G] = (int32_t)(65536 * -0.41869 + 0.5) * G;
  }
  for (B = 0; B <= 255; B++) {
    YBtab[B] = (int32_t)(65536 * 0.114 + 0.5) * B;
    CbBtab[B] = (int32_t)(32768) * B;
    CrBtab[B] = (int32_t)(65536 * -0.08131 + 0.5) * B;
  }
}


// Using a bit modified form of the FDCT routine from IJG's C source:
// Forward DCT routine idea taken from Independent JPEG Group's C source for
// JPEG encoders/decoders

// For float AA&N IDCT method, divisors are equal to quantization
//   coefficients scaled by scalefactor[row]*scalefactor[col], where
//   scalefactor[0] = 1
//   scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
//   We apply a further scale factor of 8.
//   What's actually stored is 1/divisor so that the inner loop can
//   use a multiplication rather than a division.
void prepare_quant_tables()
{
  double aanscalefactor[8] = {1.0, 1.387039845, 1.306562965, 1.175875602,
                              1.0, 0.785694958, 0.541196100, 0.275899379
                             };
  uint8_t row, col;
  uint8_t i = 0;
  for (row = 0; row < 8; row++)
  {
    for (col = 0; col < 8; col++)
    {
      fdtbl_Y[i] = (float) (1.0 / ((double) DQTinfo.Ytable[zigzag[i]] *
                                   aanscalefactor[row] * aanscalefactor[col] * 8.0));
      fdtbl_Cb[i] = (float) (1.0 / ((double) DQTinfo.Cbtable[zigzag[i]] *
                                    aanscalefactor[row] * aanscalefactor[col] * 8.0));

      i++;
    }
  }
}

void fdct_and_quantization(int8_t *data, float *fdtbl, int16_t *outdata)
{
  float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  float tmp10, tmp11, tmp12, tmp13;
  float z1, z2, z3, z4, z5, z11, z13;
  float *dataptr;
  float datafloat[64];
  float temp;
  int8_t ctr;
  uint8_t i;
  for (i = 0; i < 64; i++) datafloat[i] = data[i];

  // Pass 1: process rows.
  dataptr = datafloat;
  for (ctr = 7; ctr >= 0; ctr--) {
    tmp0 = dataptr[0] + dataptr[7];
    tmp7 = dataptr[0] - dataptr[7];
    tmp1 = dataptr[1] + dataptr[6];
    tmp6 = dataptr[1] - dataptr[6];
    tmp2 = dataptr[2] + dataptr[5];
    tmp5 = dataptr[2] - dataptr[5];
    tmp3 = dataptr[3] + dataptr[4];
    tmp4 = dataptr[3] - dataptr[4];

    // Even part

    tmp10 = tmp0 + tmp3;	// phase 2
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    dataptr[0] = tmp10 + tmp11; // phase 3
    dataptr[4] = tmp10 - tmp11;

    z1 = (tmp12 + tmp13) * ((float) 0.707106781); // c4
    dataptr[2] = tmp13 + z1;	// phase 5
    dataptr[6] = tmp13 - z1;

    // Odd part

    tmp10 = tmp4 + tmp5;	// phase 2
    tmp11 = tmp5 + tmp6;
    tmp12 = tmp6 + tmp7;

    // The rotator is modified from fig 4-8 to avoid extra negations
    z5 = (tmp10 - tmp12) * ((float) 0.382683433); // c6
    z2 = ((float) 0.541196100) * tmp10 + z5; // c2-c6
    z4 = ((float) 1.306562965) * tmp12 + z5; // c2+c6
    z3 = tmp11 * ((float) 0.707106781); // c4

    z11 = tmp7 + z3;		// phase 5
    z13 = tmp7 - z3;

    dataptr[5] = z13 + z2;	// phase 6
    dataptr[3] = z13 - z2;
    dataptr[1] = z11 + z4;
    dataptr[7] = z11 - z4;

    dataptr += 8;		//advance pointer to next row
  }

  // Pass 2: process columns

  dataptr = datafloat;
  for (ctr = 7; ctr >= 0; ctr--) {
    tmp0 = dataptr[0] + dataptr[56];
    tmp7 = dataptr[0] - dataptr[56];
    tmp1 = dataptr[8] + dataptr[48];
    tmp6 = dataptr[8] - dataptr[48];
    tmp2 = dataptr[16] + dataptr[40];
    tmp5 = dataptr[16] - dataptr[40];
    tmp3 = dataptr[24] + dataptr[32];
    tmp4 = dataptr[24] - dataptr[32];

    //Even part/

    tmp10 = tmp0 + tmp3;	//phase 2
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    dataptr[0] = tmp10 + tmp11; // phase 3
    dataptr[32] = tmp10 - tmp11;

    z1 = (tmp12 + tmp13) * ((float) 0.707106781); // c4
    dataptr[16] = tmp13 + z1; // phase 5
    dataptr[48] = tmp13 - z1;

    // Odd part

    tmp10 = tmp4 + tmp5;	// phase 2
    tmp11 = tmp5 + tmp6;
    tmp12 = tmp6 + tmp7;

    // The rotator is modified from fig 4-8 to avoid extra negations.
    z5 = (tmp10 - tmp12) * ((float) 0.382683433); // c6
    z2 = ((float) 0.541196100) * tmp10 + z5; // c2-c6
    z4 = ((float) 1.306562965) * tmp12 + z5; // c2+c6
    z3 = tmp11 * ((float) 0.707106781); // c4

    z11 = tmp7 + z3;		// phase 5
    z13 = tmp7 - z3;
    dataptr[40] = z13 + z2; // phase 6
    dataptr[24] = z13 - z2;
    dataptr[8] = z11 + z4;
    dataptr[56] = z11 - z4;

    dataptr++;			// advance pointer to next column
  }

  // Quantize/descale the coefficients, and store into output array
  for (i = 0; i < 64; i++) {
    // Apply the quantization and scaling factor
    temp = datafloat[i] * fdtbl[i];

    //Round to nearest integer.
    //Since C does not specify the direction of rounding for negative
    //quotients, we have to force the dividend positive for portability.
    //The maximum coefficient size is +-16K (for 12-bit data), so this
    //code should work for either 16-bit or 32-bit ints.

    outdata[i] = (int16_t) ((int16_t)(temp + 16384.5) - 16384);
  }
}

void process_DU(int8_t *ComponentDU, float *fdtbl, int16_t *DC,
                bitstring *HTDC, bitstring *HTAC)
{
  bitstring EOB = HTAC[0x00];
  bitstring M16zeroes = HTAC[0xF0];
  uint8_t i;
  uint8_t startpos;
  uint8_t end0pos;
  uint8_t nrzeroes;
  uint8_t nrmarker;
  int16_t Diff;

  fdct_and_quantization(ComponentDU, fdtbl, DU_DCT);
  //zigzag reorder
  for (i = 0; i <= 63; i++) DU[zigzag[i]] = DU_DCT[i];
  Diff = DU[0] - *DC;
  *DC = DU[0];
  //Encode DC
  if (Diff == 0)
    writebits(HTDC[0]); //Diff might be 0
  else {
    writebits(HTDC[category[Diff]]);
    writebits(bitcode[Diff]);
  }
  //Encode ACs
  for (end0pos = 63; (end0pos > 0) && (DU[end0pos] == 0); end0pos--) ;
  //end0pos = first element in reverse order !=0
  if (end0pos == 0) {
    writebits(EOB);
    return;
  }

  i = 1;
  while (i <= end0pos)
  {
    startpos = i;

    for (; (DU[i] == 0) && (i <= end0pos); i++) ;

    nrzeroes = i - startpos;

    if (nrzeroes >= 16) {
      for (nrmarker = 1; nrmarker <= nrzeroes / 16; nrmarker++)
        writebits(M16zeroes);

      nrzeroes = nrzeroes % 16;
    }
    //<<JC>> writebits(HTAC[nrzeroes<<4+category[DU[i]]]);
    writebits(HTAC[nrzeroes * 16 + category[DU[i]]]);
    writebits(bitcode[DU[i]]);
    i++;
  }
  if (end0pos != 63) writebits(EOB);
}
/****************************************

  #define  Y(R,G,B) ((uint8_t)( (YRtab[(R)]+YGtab[(G)]+YBtab[(B)])>>16 ) - 128) 
  #define Cb(R,G,B) ((uint8_t)( (CbRtab[(R)]+CbGtab[(G)]+CbBtab[(B)])>>16 ) )
  #define Cr(R,G,B) ((uint8_t)( (CrRtab[(R)]+CrGtab[(G)]+CrBtab[(B)])>>16 ) )
 **********************************************/
void ShowDU(void){
  uint16_t i;
  for(i= 0; i<8; i++){
    Serial.printf("%3u  %3d  %3d\n", (uint8_t)YDU[i]/2, (int8_t)CbDU[i], (int8_t)CrDU[i]);
  }
  Serial.println();
}
void load_data_units_from_RGB_buffer(uint16_t xpos, uint16_t ypos)
{
  uint8_t x, y;
  uint8_t pos = 0;
  uint32_t location;
  uint8_t R, G, B;
  location = ypos * Ximage + xpos;
  for (y = 0; y < 8; y++)
  {
    for (x = 0; x < 8; x++)
    {
      R = RGB_buffer[location].R; G = RGB_buffer[location].G; B = RGB_buffer[location].B;
      YDU[pos] = Y(R, G, B);
      CbDU[pos] = Cb(R, G, B);
      CrDU[pos] = Cr(R, G, B);
      location++; pos++;
    // if((location %10000) == 1) Serial.printf("RGB load at location %lu  JPG length = %lu\n", location,jpgcount);
    }
    location += Ximage - 8;
  }
  if((xpos==72) && (ypos==8)) {
    Serial.println("RGB load");
    ShowDU();
  }
  
}

// Bypass the RGB888 ---> YCbCr conversion
// fetch data from packed YUV422 buffer
void load_data_units_from_YUV_buffer(uint16_t xpos, uint16_t ypos) {
  uint8_t x, y;
  uint8_t pos = 0;
  uint32_t location;
  uint8_t yuvbts[16];
  int8_t Yv;
  int8_t Cb, Cr;
  location = ypos * Ximage + xpos;  // pixel location in 16-bit YUV buffer
  // Note that YCbCr buffer is an array of YUV422 16-bit words
  for (y = 0; y < 8; y++){
    for (x = 0; x < 8; x++)  {
      // Assuming format in memory is YUV MOde 0   YUYV
      if((xpos == 8) && (ypos == 8)){
          yuvbts[x*2] = YUV_buffer[location] >> 8;
          yuvbts[x*2 + 1] = YUV_buffer[location] & 0xFF;
      }
      Yv = (YUV_buffer[location] >> 8);
      if (location & 0x01) { // odd buffer address
        Cb = (YUV_buffer[location -1] &0xFF)-128;
        Cr = (YUV_buffer[location] &0xFF) - 128;
      } else {  // even buffer address
        Cb = (YUV_buffer[location] &0xFF) -128;
        Cr = (128+ YUV_buffer[location + 1] &0xFF) -128;
      }
      YDU[pos] = Yv;
      CbDU[pos] = Cb;
      CrDU[pos] = Cr;
      location++; pos++;

   //   if((location %10000) == 1) Serial.printf(" YUV Load at location %lu  JPG length = %lu\n", location,jpgcount);
    }
   
    location += Ximage - 8;
  }  if((xpos==72) && (ypos==8)) {

    Serial.println("YUV load");
     for(x = 0; x<16; x++){
      Serial.printf("%02X ", yuvbts[x]);
    }   
    Serial.println();
    ShowDU();
  }
}

// process 8 x 8 pixel blocks
void main_encoder(toutmode cmode)
{
  int16_t DCY = 0, DCCb = 0, DCCr = 0; //DC coefficients used for differential encoding
  uint16_t xpos, ypos;
  uint32_t numprocessed = 0;
  for (ypos = 0; ypos < Yimage; ypos += 8) {
    for (xpos = 0; xpos < Ximage; xpos += 8) {
      if(cmode == RGB565) load_data_units_from_RGB_buffer(xpos, ypos);
      if(cmode == YUV422) load_data_units_from_YUV_buffer(xpos, ypos);
      process_DU(YDU, fdtbl_Y, &DCY, YDC_HT, YAC_HT);
      process_DU(CbDU, fdtbl_Cb, &DCCb, CbDC_HT, CbAC_HT);
      process_DU(CrDU, fdtbl_Cb, &DCCr, CbDC_HT, CbAC_HT);
      numprocessed++;
    }
  }
  Serial.printf("main_encoder processed %lu blocks\n", numprocessed);
}


// This initialization needs to be done just once at program startup
void JPGinit_all(void) {
  set_DQTinfo();
  set_DHTinfo();
  init_Huffman_tables();
  set_numbers_category_and_bitcode();
  precalculate_YCbCr_tables();
  prepare_quant_tables();
}


// our RBG888 data is present at rgbuff in EXTMEM
void SaveJpgFile(uint8_t *rgbuff, toutmode cmode,  uint16_t wd, uint16_t ht, const  char* szJpgFileNameOut) {

  char JPG_filename[64];
  strcpy(JPG_filename,  szJpgFileNameOut);
  uint16_t Ximage_original = wd;
  uint16_t Yimage_original = ht;	//the original image dimensions,

  Ximage = wd;
  Yimage = ht;
  if(cmode == RGB565) RGB_buffer =  (colorRGB *)rgbuff;
  if(cmode == YUV422) YUV_buffer =  (uint16_t *) rgbuff;
  wrptr = &jpgbuff[0];
  jpgcount = 0;

  bitstring fillbits; //filling bitstring for the bit alignment of the EOI marker

  //  load_bitmap(BMP_filename, &Ximage_original, &Yimage_original);

  //  init_all();  Initialization done once in Setup()

  SOF0info.width = Ximage_original;
  SOF0info.height = Yimage_original;
  writeword(0xFFD8); //SOI
  write_APP0info();

  write_DQTinfo();
  write_SOF0info();
  write_DHTinfo();
  write_SOSinfo();
  Serial.printf("After header, JPG length is %lu\n", jpgcount);
  bytenew = 0; bytepos = 7;
  
  main_encoder(cmode);

  //Do the bit alignment of the EOI marker
  if (bytepos >= 0)
  {
    fillbits.length = bytepos + 1;
    fillbits.value = (1 << (bytepos + 1)) - 1;
    writebits(fillbits);
  }
  writeword(0xFFD9); //EOI
  fp_jpeg_stream = SD.open(JPG_filename, FILE_WRITE);
  if (jpgcount < JPGMAXSIZE) {
    fp_jpeg_stream.write(&jpgbuff, jpgcount);
    Serial.printf("Wrote %s with length %lu\n", JPG_filename, jpgcount);
  } else Serial.println("JPG buffer overflow");
      fp_jpeg_stream.close();
  }
