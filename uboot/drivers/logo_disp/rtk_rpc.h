#ifndef _RTK_RPC_H_
#define _RTK_RPC_H_

typedef long HRESULT;

typedef struct RPCRES_LONG {
	HRESULT result;
	long data;
}RPCRES_LONG;

typedef struct { 
	unsigned long  info;
	RPCRES_LONG retval;
	unsigned long ret;
} RPC_DEFAULT_INPUT_T;

//AUDIO_RPC_ToAgent_AOUT_HDMI_Set_0
struct AUDIO_HDMI_SET {
     u_int HDMI_Frequency;
};
typedef struct AUDIO_HDMI_SET AUDIO_HDMI_SET;

//AUDIO_RPC_ToAgent_HDMI_Mute_0
struct AUDIO_HDMI_MUTE_INFO {
     long instanceID;
     char hdmi_mute;
};
typedef struct AUDIO_HDMI_MUTE_INFO AUDIO_HDMI_MUTE_INFO;

//AUDIO_RPC_ToAgent_HDMI_OUT_VSDB_0
struct AUDIO_HDMI_OUT_VSDB_DATA {
     long HDMI_VSDB_delay;
};
typedef struct AUDIO_HDMI_OUT_VSDB_DATA AUDIO_HDMI_OUT_VSDB_DATA;

//AUDIO_RPC_ToAgent_HDMI_INFO_0
struct HDMI_INFO {
     long video_type;
};
typedef struct HDMI_INFO HDMI_INFO;



typedef enum
{
    ENUM_KERNEL_RPC_CREATE_AGENT,
    ENUM_KERNEL_RPC_INIT_RINGBUF,
    ENUM_KERNEL_RPC_PRIVATEINFO,
    ENUM_KERNEL_RPC_RUN,
    ENUM_KERNEL_RPC_PAUSE,
    ENUM_KERNEL_RPC_SWITCH_FOCUS,
    ENUM_KERNEL_RPC_MALLOC_ADDR,
    ENUM_KERNEL_RPC_VOLUME_CONTROL,
    ENUM_KERNEL_RPC_FLUSH,
    ENUM_KERNEL_RPC_CONNECT,
    ENUM_KERNEL_RPC_SETREFCLOCK,
    ENUM_KERNEL_RPC_DAC_I2S_CONFIG,
    ENUM_KERNEL_RPC_DAC_SPDIF_CONFIG,
    ENUM_KERNEL_RPC_HDMI_OUT_EDID,
    ENUM_KERNEL_RPC_HDMI_OUT_EDID2,
    ENUM_KERNEL_RPC_HDMI_SET,
    ENUM_KERNEL_RPC_HDMI_MUTE,
    ENUM_KERNEL_RPC_ASK_DBG_MEM_ADDR,
    ENUM_KERNEL_RPC_DESTROY,
    ENUM_KERNEL_RPC_STOP,
    ENUM_KERNEL_RPC_CHECK_READY,   // check if Audio get memory pool from AP
    ENUM_KERNEL_RPC_GET_MUTE_N_VOLUME,   // get mute and volume level
    ENUM_KERNEL_RPC_EOS,
    ENUM_KERNEL_RPC_ADC0_CONFIG,
    ENUM_KERNEL_RPC_ADC1_CONFIG,
    ENUM_KERNEL_RPC_ADC2_CONFIG,
#if defined(AUDIO_TV_PLATFORM)
    ENUM_KERNEL_RPC_BBADC_CONFIG,
    ENUM_KERNEL_RPC_I2SI_CONFIG,
    ENUM_KERNEL_RPC_SPDIFI_CONFIG,
#endif // AUDIO_TV_PLATFORM
    ENUM_KERNEL_RPC_HDMI_OUT_VSDB,
    ENUM_VIDEO_KERNEL_RPC_CONFIG_TV_SYSTEM,
    ENUM_VIDEO_KERNEL_RPC_CONFIG_HDMI_INFO_FRAME,
} ENUM_AUDIO_KERNEL_RPC_CMD;

// Video
enum VO_HDMI_MODE
{
  VO_DVI_ON,
  VO_HDMI_ON,
  VO_HDMI_OFF,               /* turn off HDMI/DVI */
  VO_MHL_ON
} ;
typedef enum VO_HDMI_MODE VO_HDMI_MODE;

enum VO_HDMI_AUDIO_SAMPLE_FREQ {
     VO_HDMI_AUDIO_NULL = 0,
     VO_HDMI_AUDIO_32K = 1,
     VO_HDMI_AUDIO_44_1K = 2,
     VO_HDMI_AUDIO_48K = 3,
     VO_HDMI_AUDIO_88_2K = 4,
     VO_HDMI_AUDIO_96K = 5,
     VO_HDMI_AUDIO_176_4K = 6,
     VO_HDMI_AUDIO_192K = 7,
};
typedef enum VO_HDMI_AUDIO_SAMPLE_FREQ VO_HDMI_AUDIO_SAMPLE_FREQ;


struct VIDEO_RPC_VOUT_CONFIG_HDMI_INFO_FRAME {
     enum VO_HDMI_MODE hdmiMode;
     enum VO_HDMI_AUDIO_SAMPLE_FREQ audioSampleFreq;
     u_char audioChannelCount;
     u_char dataByte1;
     u_char dataByte2;
     u_char dataByte3;
     u_char dataByte4;
     u_char dataByte5;
     u_int dataInt0;	//see Note 1
     u_int hdmi2p0_feature;//[Bit0]HDMI 2.0 [Bit1]Scrabmle
     u_int reserved2;
     u_int reserved3;
     u_int reserved4;
};
typedef struct VIDEO_RPC_VOUT_CONFIG_HDMI_INFO_FRAME VIDEO_RPC_VOUT_CONFIG_HDMI_INFO_FRAME;

//  Notes on struct VIDEO_RPC_VOUT_CONFIG_HDMI_INFO_FRAME:
//  1. b'31:7 = Rsdv.
//	   b'6    = xvMetaData(ignored). 0:disable 1:enable.  
//     b'5:2  = color bit depth;depend on deepcolor bit. 4:24bits 5:30bits, 6:36bits. 
//     b'1    = deepColor. 0:disable 1:enable                           
//     b'0    = disAudio;disable audio setting(ignored). 1:disable 0:enable.


enum VO_INTERFACE_TYPE {
     VO_ANALOG_AND_DIGITAL = 0,
     VO_ANALOG_ONLY = 1,
     VO_DIGITAL_ONLY = 2,
 };
typedef enum VO_INTERFACE_TYPE VO_INTERFACE_TYPE;

enum VO_PEDESTAL_TYPE {
     VO_PEDESTAL_TYPE_300_700_ON = 0,
     VO_PEDESTAL_TYPE_300_700_OFF = 1,
     VO_PEDESTAL_TYPE_286_714_ON = 2,
     VO_PEDESTAL_TYPE_286_714_OFF = 3,
};
typedef enum VO_PEDESTAL_TYPE VO_PEDESTAL_TYPE;

enum VO_STANDARD {
	VO_STANDARD_NTSC_M = 0,
	VO_STANDARD_NTSC_J = 1,
	VO_STANDARD_NTSC_443 = 2,
	VO_STANDARD_PAL_B = 3,
	VO_STANDARD_PAL_D = 4,
	VO_STANDARD_PAL_G = 5,
	VO_STANDARD_PAL_H = 6,
	VO_STANDARD_PAL_I = 7,
	VO_STANDARD_PAL_N = 8,
	VO_STANDARD_PAL_NC = 9,
	VO_STANDARD_PAL_M = 10,
	VO_STANDARD_PAL_60 = 11,
	VO_STANDARD_SECAM = 12,
	VO_STANDARD_HDTV_720P_60 = 13,
	VO_STANDARD_HDTV_720P_50 = 14,
	VO_STANDARD_HDTV_720P_30 = 15,
	VO_STANDARD_HDTV_720P_25 = 16,
	VO_STANDARD_HDTV_720P_24 = 17,
	VO_STANDARD_HDTV_1080I_60 = 18,
	VO_STANDARD_HDTV_1080I_50 = 19,
	VO_STANDARD_HDTV_1080P_30 = 20,
	VO_STANDARD_HDTV_1080P_25 = 21,
	VO_STANDARD_HDTV_1080P_24 = 22,
	VO_STANDARD_VGA = 23,
	VO_STANDARD_SVGA = 24,
	VO_STANDARD_HDTV_1080P_60 = 25,
	VO_STANDARD_HDTV_1080P_50 = 26,
	VO_STANDARD_HDTV_1080I_59 = 27,
	VO_STANDARD_HDTV_720P_59 = 28,
	VO_STANDARD_HDTV_1080P_23 = 29,
	VO_STANDARD_HDTV_1080P_59 = 30,
	VO_STANDARD_HDTV_1080P_60_3D = 31,
	VO_STANDARD_HDTV_1080P_50_3D = 32,
	VO_STANDARD_HDTV_1080P_30_3D = 33,
	VO_STANDARD_HDTV_1080P_24_3D = 34,
	VO_STANDARD_HDTV_720P_60_3D = 35,
	VO_STANDARD_HDTV_720P_50_3D = 36,
	VO_STANDARD_HDTV_720P_30_3D = 37,
	VO_STANDARD_HDTV_720P_24_3D = 38,
	VO_STANDARD_HDTV_720P_59_3D = 39,
	VO_STANDARD_HDTV_1080I_60_3D = 40,
	VO_STANDARD_HDTV_1080I_59_3D = 41,
	VO_STANDARD_HDTV_1080I_50_3D = 42,
	VO_STANDARD_HDTV_1080P_23_3D = 43,
	VO_STANDARD_HDTV_2160P_30 = 44,
	VO_STANDARD_HDTV_2160P_29 = 45,
	VO_STANDARD_HDTV_2160P_25 = 46,
	VO_STANDARD_HDTV_2160P_24 = 47,
	VO_STANDARD_HDTV_2160P_23 = 48,
	VO_STANDARD_HDTV_4096_2160P_24 = 49,
	VO_STANDARD_HDTV_2160P_60 = 50,
	VO_STANDARD_HDTV_2160P_50 = 51,
	VO_STANDARD_HDTV_4096_2160P_25 = 52,
	VO_STANDARD_HDTV_4096_2160P_30 = 53,
	VO_STANDARD_HDTV_4096_2160P_50 = 54,
	VO_STANDARD_HDTV_4096_2160P_60 = 55,
	VO_STANDARD_HDTV_2160P_60_420 = 56,
	VO_STANDARD_HDTV_2160P_50_420 = 57,
	VO_STANDARD_HDTV_4096_2160P_60_420 = 58,
	VO_STANDARD_HDTV_4096_2160P_50_420 = 59,
};
typedef enum VO_STANDARD VO_STANDARD;


struct VIDEO_FORMAT_CONVERTER {
	 enum VO_STANDARD standard;
     u_int vid;  
};

struct VIDEO_RPC_VOUT_CONFIG_VIDEO_STANDARD {
     enum VO_STANDARD standard;
     u_char enProg;
     u_char enDIF;
     u_char enCompRGB;
     enum VO_PEDESTAL_TYPE pedType;
     u_int dataInt0;	//see Note 1
     u_int dataInt1;		
};
typedef struct VIDEO_RPC_VOUT_CONFIG_VIDEO_STANDARD VIDEO_RPC_VOUT_CONFIG_VIDEO_STANDARD;

//  Notes on struct VIDEO_RPC_VOUT_CONFIG_VIDEO_STANDARD:
//  1. b'31:16 = Rsdv.
//	   b'15:12 = expectVideoFormat_3D. 0:disable 1:enable.  
//     b'11:8  = Format_3D (ignored).
//     b'4:3   = TVE_DAC_mode;output mode of TVE DAC (ignored).                           
//     b'2:1   = NTSC source select (ignored).
//     b'0	   = comp_sel;component output source select (ignored).	

struct VIDEO_RPC_VOUT_CONFIG_TV_SYSTEM {
    enum VO_INTERFACE_TYPE interfaceType;
    struct VIDEO_RPC_VOUT_CONFIG_VIDEO_STANDARD videoInfo;
    struct VIDEO_RPC_VOUT_CONFIG_HDMI_INFO_FRAME hdmiInfo;
 };
typedef struct VIDEO_RPC_VOUT_CONFIG_TV_SYSTEM VIDEO_RPC_VOUT_CONFIG_TV_SYSTEM;

struct AUDIO_HDMI_OUT_EDID_DATA2 {
	long Version;
	long HDMI_output_enable;
	long EDID_DATA_addr;
};
typedef struct AUDIO_HDMI_OUT_EDID_DATA2 AUDIO_HDMI_OUT_EDID_DATA2;

typedef struct _BOOT_TV_STD_INFO
{
   unsigned int dwMagicNumber;
   VIDEO_RPC_VOUT_CONFIG_TV_SYSTEM tv_sys;
}BOOT_TV_STD_INFO ;

#endif
