#ifndef DAQTYPE_H
#define DAQTYPE_H

#include <string>
#include <vector>
#include <cstdio>

using namespace std;


struct ProdInfo {
  int runNumber;
  int runExtNumber;
  int eventNumber;
  int numberOfStreams;
  float beamIntensity;
  int beamSource;
  };

// channel definition for the Acqiris digitizer modules
// here the settings are valid for all the channels

// struct for general BOS header

typedef struct {
 	union title {
 		u_int l;
 		char c[4];
 		} title;
 	u_int revNumber,reserved,length;
 	} BOSHEADER;

// struct for the RCTR ( run control Header )

typedef struct {
	u_int 	runNumber,
          runExtNumber,
          streamNumber,
          experiment,
       		date,
		      time,
          numberOfModules,
          numberOfChannels,
          totalNumberOfStreams,
          totalNumberOfChassis,
          totalNumberOfModules;
	} RCTR_HEADER;

// struct for the Acqiris Module settings

typedef struct 	{
			unsigned int 	sampleRate,	// setting for the digitizer
	 		sampleSize,	//	"	
			fullScale;	//	"
			float		delayTime;	//	"
			int		threshold;	// setting for the compression
			unsigned int	threshSign,	//	"
			preSample,	//	"
			postSample,	//	"
			clockState,
			inputState;	// has to be checked for each event
			float	offsetVoltage;	// offset voltage in volt
			} MOD_HEADER;


// struct for the EVEH ( event  Header )

typedef struct {
	u_int	  sizeOfEvent;
	u_int 	eventNumber;
	u_int 	runNumber,
       		time,
		      date;
	} EVENT_HEADER;

// struct for the ACQC ( data  Header )

typedef struct {
	u_int	  detectorType;
	u_int 	detectorChannel;
	u_int 	stream_crate_mod_ch;
	} ACQC_HEADER;



typedef enum eXc { eXsend,eXreceive };	// exchange type

// channel definition for the ETEP digitizer modules
// this is the setting for one channel and can
// change for each individual channel
// the setup is stored in Conf/READOUT.conf file
// and at the beginning of the index file.
// the data file (file from one stream)  will only have the local settings relevant for
// it's readout part

typedef struct {	// definition of a channel configuration
	u_int id;     // detector type (4*char, for example 'SILI' or 'C6D6'
	u_int detectorChannel; 	// physical detector channel
        u_int moduleType;       // 1 = acqiris  2 = etep
        u_int str_crate_mod_ch; // 4 bytes: stream, crate, module, channel
	u_int sampleRate;       // MS/s
	u_int sampleSize;       // MB
        u_int fullScale;        // index in table
        int     delayTime;        // samples
	u_int threshold;        // ADC counts
	u_int thresholdSign;    // ADC counts
        float   offset;           // mV
	u_int 	pre;              // samples/16
	u_int 	post;             // samples/16
        u_int   clockState;       // INTC /EXTC internal/external clock (Acqiris only)

  float getFullScaleInV() {
    float val[7]={0.05,0.1,0.2,0.5,1.0,2.0,5.0};
    if (fullScale<7) return val[fullScale];
    return 0;
    };
  float getNanoSecPerSample() {return (sampleRate!=0) ? 1000.0/sampleRate : 1.0;};
  u_int getChannel() {return (str_crate_mod_ch&0x000000ff);};
  u_int getModule()  {return (str_crate_mod_ch&0x0000ff00)>>8;};
  u_int getCrate()   {return (str_crate_mod_ch&0x00ff0000)>>16;};
  u_int getStream()  {return (str_crate_mod_ch&0xff000000)>>24;};

  static string getDescriptionTitle() {
    return "Type  ID Strm Crate Mod Ch  MS/s    kB   Pre  Post Delay FScale Offset Thresh Type\n";
    }
  string getDescription() {
    char buffer[356];
    memcpy(buffer,&id,4);buffer[4]=32;
    char modType[5];memcpy(modType,&moduleType,4);modType[4]=0;
    float fullscale_mV=getFullScaleInV()*1000.0;
    sprintf(&buffer[5],"%3d%5d%6d%4d%3d%6d%6d%6d%6d%6d  %1.2fV  %3.1fmV %3.3fmV %5s\n",
           detectorChannel,
           getStream(),
           getCrate(),
           getModule(),
           getChannel(),
           sampleRate,
           sampleSize,
           pre,
           post,
           delayTime,
           fullscale_mV,
           offset,
           -offset-0.5*fullscale_mV+float(threshold)/256.0*fullscale_mV,
           modType
           );
    return buffer;
    }
} Channel_Config;


struct HV_Config {
   u_int detectorType;       // same as in Channel_Config
   u_int detectorChannel;    // same as in Channel_Config
   u_int hvCrate;            // may be 0, if only one crate is used.
   u_int hvBoard;
   u_int hvChannel;
   float vSet;               // in Volts
   float vMax;               // in Volts
   float iMax;               // in microAmperes
   float deltaV;             // region for alarm
   float deltaI;             // region for alarm
   u_int status;             // 0 = off (V<Vset-deltaV), 1 = on |Vset-V|<deltaV 2 = V>Vset+deltaV
};

typedef vector< HV_Config > HVList;


/*
void printStCrMoCh(u_int StCrMoCh) {
printf("St = %d Cr = %d Mo = %d Ch = %d",
  (StCrMoCh&0xff000000)>>24,
  (StCrMoCh&0x00ff0000)>>16,
  (StCrMoCh&0x0000ff00)>>8,
   StCrMoCh&0x000000ff         );

}
*/

// Channel Table was originally introduced
// by Richard Hirst who provided a Linux
// driver for the acquiris digitizer

typedef struct s_chanTable {  // know it's yust an extension, if the physical channel is used or not
	Channel_Config config;
	bool inUse;
} ChanTable_t, *ChanTablePtr;


typedef	vector< ChanTable_t > Chan_V;  // contains all channel specific data

typedef struct {  //
	Chan_V chan_v;
	bool inUse;
} ModTable_t;


typedef struct {	// definition of a channel configuration
	} TEMP_HEADER;

typedef vector< Channel_Config > Config_V;


typedef unsigned int u_int;
typedef bool (*PFL)(void);
typedef int (*PFI)(void);
typedef int (*PFI_PC)(char *);
typedef vector< string > S_V;

class acq_routines;

#endif

