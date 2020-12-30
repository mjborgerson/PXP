typedef struct
{
    volatile uint32_t CTRL;
    volatile uint32_t CTRL_SET;
    volatile uint32_t CTRL_CLR;
    volatile uint32_t CTLR_TOG;
    volatile uint32_t STAT;
    volatile uint32_t STAT_SET;
    volatile uint32_t STAT_CLR;
    volatile uint32_t STAT_TOG;
    volatile uint32_t OUT_CTRL;
    volatile uint32_t OUT_CTRL_SET;
    volatile uint32_t OUT_CTRL_CLR;
    volatile uint32_t OUT_CTRL_TOG;
    volatile void*    OUT_BUF;
    volatile uint32_t unused1[3];
    volatile void*    OUT_BUF2;
    volatile uint32_t unused2[3];
    volatile uint32_t OUT_PITCH;
    volatile uint32_t unused3[3];
    volatile uint32_t OUT_LRC;
    volatile uint32_t unused4[3];
    volatile uint32_t OUT_PS_ULC;
    volatile uint32_t unused5[3];
    volatile uint32_t OUT_PS_LRC;
    volatile uint32_t unused6[3];
    volatile uint32_t OUT_AS_ULC;
    volatile uint32_t unused7[3];
    volatile uint32_t OUT_AS_LRC;
    volatile uint32_t unused8[3];
    volatile uint32_t PS_CTRL;
    volatile uint32_t PS_CTRL_SET;
    volatile uint32_t PS_CTRL_CLR;
    volatile uint32_t PS_CTRL_TOG;
    volatile void*    PS_BUF;
    volatile uint32_t unused9[3];
    volatile void*    PS_UBUF;
    volatile uint32_t unused10[3];
    volatile void*    PS_VBUF;
    volatile uint32_t unused11[3];
    volatile uint32_t PS_PITCH;
    volatile uint32_t unused12[3];
    volatile uint32_t PS_BACKGROUND;
    volatile uint32_t unused13[3];
    volatile uint32_t PS_SCALE;
    volatile uint32_t unused14[3];
    volatile uint32_t PS_OFFSET;
    volatile uint32_t unused15[3];
    volatile uint32_t PS_CLRKEYLOW;
    volatile uint32_t unused16[3];
    volatile uint32_t PS_CLRKEYHIGH;
    volatile uint32_t unused17[3];
    volatile uint32_t AS_CTRL;
    volatile uint32_t unused18[3];
    volatile void*    AS_BUF;
    volatile uint32_t unused19[3];
    volatile uint32_t AS_PITCH;
    volatile uint32_t unused20[3];
    volatile uint32_t AS_CLRKEYLOW;
    volatile uint32_t unused21[3];
    volatile uint32_t AS_CLRKEYHIGH;
    volatile uint32_t unused22[3];
    volatile uint32_t CSC1_COEF0;
    volatile uint32_t unused23[3];
    volatile uint32_t CSC1_COEF1;
    volatile uint32_t unused24[3];
    volatile uint32_t CSC1_COEF2;
    volatile uint32_t unused25[87];
    volatile uint32_t POWER;
    volatile uint32_t unused26[55];
    volatile uint32_t NEXT;
    volatile uint32_t unused27[15];
    volatile uint32_t PORTER_DUFF_CTRL;
} IMXRT_PXP_t;
#define IMXRT_PXP        (*(IMXRT_PXP_t *)0x402B4000)
#define PXP_CTRL            (IMXRT_PXP.CTRL)
#define PXP_CTRL_SET            (IMXRT_PXP.CTRL_SET)
#define PXP_CTRL_CLR            (IMXRT_PXP.CTRL_CLR)
#define PXP_CTRL_TOG            (IMXRT_PXP.CTRL_TOG)
#define PXP_STAT            (IMXRT_PXP.STAT)
#define PXP_STAT_SET            (IMXRT_PXP.STAT_SET)
#define PXP_STAT_CLR            (IMXRT_PXP.STAT_CLR)
#define PXP_STAT_TOG            (IMXRT_PXP.STAT_TOG)
#define PXP_OUT_CTRL            (IMXRT_PXP.OUT_CTRL)
#define PXP_OUT_CTRL_SET        (IMXRT_PXP.OUT_CTRL_SET)
#define PXP_OUT_CTRL_CLR        (IMXRT_PXP.OUT_CTRL_CLR)
#define PXP_OUT_CTRL_TOG        (IMXRT_PXP.OUT_CTRL_TOG)
#define PXP_OUT_BUF            (IMXRT_PXP.OUT_BUF)
#define PXP_OUT_BUF2            (IMXRT_PXP.OUT_BUF2)
#define PXP_OUT_PITCH            (IMXRT_PXP.OUT_PITCH)
#define PXP_OUT_LRC            (IMXRT_PXP.OUT_LRC)
#define PXP_OUT_PS_ULC            (IMXRT_PXP.OUT_PS_ULC)
#define PXP_OUT_PS_LRC            (IMXRT_PXP.OUT_PS_LRC)
#define PXP_OUT_AS_ULC            (IMXRT_PXP.OUT_AS_ULC)
#define PXP_OUT_AS_LRC            (IMXRT_PXP.OUT_AS_LRC)
#define PXP_PS_CTRL            (IMXRT_PXP.PS_CTRL)
#define PXP_PS_CTRL_SET            (IMXRT_PXP.PS_CTRL_SET)
#define PXP_PS_CTRL_CLR            (IMXRT_PXP.PS_CTRL_CLR)
#define PXP_PS_CTRL_TOG            (IMXRT_PXP.PS_CTRL_TOG)
#define PXP_PS_BUF            (IMXRT_PXP.PS_BUF)
#define PXP_PS_UBUF            (IMXRT_PXP.PS_UBUF)
#define PXP_PS_VBUF            (IMXRT_PXP.PS_VBUF)
#define PXP_PS_PITCH            (IMXRT_PXP.PS_PITCH)
#define PXP_PS_BACKGROUND_0        (IMXRT_PXP.PS_BACKGROUND)
#define PXP_PS_SCALE            (IMXRT_PXP.PS_SCALE)
#define PXP_PS_OFFSET            (IMXRT_PXP.PS_OFFSET)
#define PXP_PS_CLRKEYLOW_0        (IMXRT_PXP.PS_CLRKEYLOW)
#define PXP_PS_CLRKEYHIGH_0        (IMXRT_PXP.PS_CLRKEYHIGH)
#define PXP_AS_CTRL            (IMXRT_PXP.AS_CTRL)
#define PXP_AS_BUF            (IMXRT_PXP.AS_BUF)
#define PXP_AS_PITCH            (IMXRT_PXP.AS_PITCH)
#define PXP_AS_CLRKEYLOW_0        (IMXRT_PXP.AS_CLRKEYLOW)
#define PXP_AS_CLRKEYHIGH_0        (IMXRT_PXP.AS_CLRKEYHIGH)
#define PXP_CSC1_COEF0            (IMXRT_PXP.CSC1_COEF0)
#define PXP_CSC1_COEF1            (IMXRT_PXP.CSC1_COEF1)
#define PXP_CSC1_COEF2            (IMXRT_PXP.CSC1_COEF2)
#define PXP_POWER            (IMXRT_PXP.POWER)
#define PXP_NEXT            (IMXRT_PXP.NEXT)
#define PXP_PORTER_DUFF_CTRL        (IMXRT_PXP.PORTER_DUFF_CTRL)


#define PXP_ARGB8888                        0x0 //32-bit pixels with alpha
#define PXP_RGBA888                         0x1 //32-bit pixel with alpha at low 8 bits
#define PXP_RGB888                          0x4 //32-bit pixels without alpha (unpacked 24-bit format)
#define PXP_RGB888P                         0x5 //24-bit pixels (packed 24-bit format)
#define PXP_ARGB1555                        0x8 //16-bit pixels with alpha at low 1 bit
#define PXP_ARGB4444                        0x9 //16-bit pixels with alpha at low 4 bits
#define PXP_RGB555                          0xC //16-bit pixels without alpha
#define PXP_RGB444                          0xD //16-bit pixels without alpha
#define PXP_RGB565                          0xE //16-bit pixels without alpha
#define PXP_YUV1P444                        0x10 //32-bit pixels (1-plane XYUV unpacked)
#define PXP_UYVY1P422                       0x12 //16-bit pixels (1-plane U0,Y0,V0,Y1 interleaved bytes)
#define PXP_VYUY1P422                       0x13 //16-bit pixels (1-plane V0,Y0,U0,Y1 interleaved bytes)
#define PXP_Y8                              0x14 //8-bit monochrome pixels (1-plane Y luma output)
#define PXP_Y4                              0x15 //4-bit monochrome pixels (1-plane Y luma, 4 bit truncation)
#define PXP_YUV2P422                        0x18 //16-bit pixels (2-plane UV interleaved bytes)
#define PXP_YUV2P420                        0x19 //16-bit pixels (2-plane UV)
#define PXP_YVU2P422                        0x1A //16-bit pixels (2-plane VU interleaved bytes)
#define PXP_YVU2P420                        0x1B //16-bit pixels (2-plane VU)
#define PXP_YUV422                          0x1E //16-bit pixels (3-plane format)
#define PXP_YUV420                          0x1F //16-bit pixels (3-plane format)

#define PXP_MASKAS                          0x0
#define PXP_MASKNOTAS                       0x1
#define PXP_MASKASNOT                       0x2
#define PXP_MERGEAS                         0x3
#define PXP_MERGENOTAS                      0x4
#define PXP_MERGEASNOT                      0x5
#define PXP_NOTCOPYAS                       0x6
#define PXP_NOT                             0x7
#define PXP_NOTMASKAS                       0x8
#define PXP_NOTMERGEAS                      0x9
#define PXP_XORAS                           0xA
#define PXP_NOTXORAS                        0xB

#define PXP_CTRL_ENABLE                     ((uint32_t)(1))
#define PXP_CTRL_IRQ_ENABLE                 ((uint32_t)(1<<1))
#define PXP_CTRL_NEXT_IRQ_ENABLE            ((uint32_t)(1<<2))
#define PXP_CTRL_ENABLE_LCD_HANDSHAKE       ((uint32_t)(1<<4))
#define PXP_CTRL_ROTATE(n)                  ((uint32_t)(((n) & 0x03) << 8))
#define PXP_CTRL_HFLIP                      ((uint32_t)(1<<10))
#define PXP_CTRL_VFLIP                      ((uint32_t)(1<<11))
#define PXP_CTRL_ROT_POS                    ((uint32_t)(1<<22))
#define PXP_CTRL_BLOCK_SIZE                 ((uint32_t)(1<<23))
#define PXP_CTRL_EN_REPEAT                  ((uint32_t)(1<<28))
#define PXP_CTRL_CLKGATE                    ((uint32_t)(1<<30))
#define PXP_CTRL_SFTRST                     ((uint32_t)(1<<31))

#define PXP_STAT_IRQ                        ((uint32_t)(1))
#define PXP_STAT_AXI_WRITE_ERROR            ((uint32_t)(1<<1))
#define PXP_STAT_AXI_READ_ERROR             ((uint32_t)(1<<2))
#define PXP_STAT_NEXT_IRQ                   ((uint32_t)(1<<3))
#define PXP_STAT_AXI_ERROR_ID               ((uint32_t)((PXP_STAT>>4) & 0xF))
#define PXP_STAT_LUT_DMA_LOAD_DONE_IRQ      ((uint32_t)(1<<8))
#define PXP_STAT_BLOCKY                     ((uint32_t)((PXP_STAT>>16) & 0xFF))
#define PXP_STAT_BLOCKX                     ((uint32_t)((PXP_STAT>>24) & 0xFF))

#define PXP_OUT_CTRL_FORMAT(n)              ((uint32_t)(((n) & 0x1F)))
#define PXP_OUT_CTRL_INTERLACED_OUTPUT(n)   ((uint32_t)(((n) & 0x3) << 8))
#define PXP_OUT_CTRL_ALPHA_OUTPUT           ((uint32_t)(1<<23))
#define PXP_OUT_CTRL_ALPHA(n)               ((uint32_t)(((n) & 0xFF) << 24))

#define PXP_PS_CTRL_FORMAT(n)               ((uint32_t)(((n) & 0x1F)))
#define PXP_PS_CTRL_WB_SWAP                 ((uint32_t)(1<<5))
#define PXP_PS_CTRL_DECY(n)                 ((uint32_t)(((n) & 0x3) << 8))
#define PXP_PS_CTRL_DECX(n)                 ((uint32_t)(((n) & 0x3) << 10))

#define PXP_AS_CTRL_ALPHA_CTRL(n)           ((uint32_t)(((n) & 0x3) << 1))
#define PXP_AS_CTRL_ENABLE_COLORKEY         ((uint32_t)(1<<3))
#define PXP_AS_CTRL_FORMAT(n)               ((uint32_t)(((n) & 0xF) << 4))
#define PXP_AS_CTRL_ALPHA(n)                ((uint32_t)(((n) & 0xFF) << 8))
#define PXP_AS_CTRL_ROP(n)                  ((uint32_t)(((n) & 0xF) << 16))
#define PXP_AS_CTRL_ALPHA_INVERT            ((uint32_t)(1<<20))

#define PXP_COEF0_Y_OFFSET(n)               ((uint32_t)(((n) & 0x1FF)))
#define PXP_COEF0_UV_OFFSET(n)              ((uint32_t)(((n) & 0x1FF) << 9))
#define PXP_COEF0_C0(n)                     ((uint32_t)(((n) & 0x7FF) << 18))
#define PXP_COEF0_BYPASS                    ((uint32_t)(1<<30))
#define PXP_COEF0_YCBCR_MODE                ((uint32_t)(1<<31))

#define PXP_COEF1_C4(n)                     ((uint32_t)(((n) & 0x7FF)))
#define PXP_COEF1_C1(n)                     ((uint32_t)(((n) & 0x7FF) << 16))

#define PXP_COEF2_C3(n)                     ((uint32_t)(((n) & 0x7FF)))
#define PXP_COEF2_C2(n)                     ((uint32_t)(((n) & 0x7FF) << 16))

#define PXP_POWER_ROT_MEM_LP_STATE(n)       ((uint32_t)(((n) & 0x7) << 9))
#define PXP_POWER_CTRL(n)                   ((uint32_t)(((n) & 0xFFFFF) << 12))

#define PXP_NEXT_ENABLED                    ((uint32_t)(1))
#define PXP_NEXT_POINTER(n)                 ((uint32_t)(((n) & 0xFFFFFFFC)))

#define PXP_PORTER_DUFF_CTRL_PORTER_DUFF_ENABLE        ((uint32_t)(1))
#define PXP_PORTER_DUFF_CTRL_S0_S1_FACTOR_MODE(n)      ((uint32_t)(((n) & 0x3) << 1))
#define PXP_PORTER_DUFF_CTRL_S0_GLOBAL_ALPHA_MODE(n)   ((uint32_t)(((n) & 0x3) << 3))
#define PXP_PORTER_DUFF_CTRL_S0_ALPHA_MODE             ((uint32_t)(1<<5))
#define PXP_PORTER_DUFF_CTRL_S0_COLOR_MODE             ((uint32_t)(1<<6))
#define PXP_PORTER_DUFF_CTRL_S1_S0_FACTOR_MODE(n)      ((uint32_t)(((n) & 0x3) << 8))
#define PXP_PORTER_DUFF_CTRL_S1_GLOBAL_ALPHA_MODE(n)   ((uint32_t)(((n) & 0x3) << 10))
#define PXP_PORTER_DUFF_CTRL_S1_ALPHA_MODE             ((uint32_t)(1<<12))
#define PXP_PORTER_DUFF_CTRL_S1_COLOR_MODE             ((uint32_t)(1<<13))
#define PXP_PORTER_DUFF_CTRL_S0_GLOBAL_ALPHA(n)        ((uint32_t)(((n) & 0xFF) << 16))
#define PXP_PORTER_DUFF_CTRL_S1_GLOBAL_ALPHA(n)        ((uint32_t)(((n) & 0xFF) << 24))

#define PXP_PITCH(n)                        ((uint32_t)(((n) & 0xFFFF)))

#define PXP_XCOORD(n)                       ((uint32_t)(((n) & 0x3FFFFFFF) << 16))
#define PXP_YCOORD(n)                       ((uint32_t)(((n) & 0x3FFFFFFF)))

#define PXP_XSCALE(n)                       ((uint32_t)(((n) & 0x7FFFFFFF)))
#define PXP_YSCALE(n)                       ((uint32_t)(((n) & 0x7FFFFFFF) << 16))

#define PXP_XOFFSET(n)                      ((uint32_t)(((n) & 0xFFFFFFF)))
#define PXP_YOFFSET(n)                      ((uint32_t)(((n) & 0xFFFFFFF) << 16))

#define PXP_COLOR(n)                        ((uint32_t)(((n) & 0xFFFFFF)))

// 54.6: page 2990