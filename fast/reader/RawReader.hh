#ifndef RAWREADER_HH
#define RAWREADER_HH 1


#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;



//---------------------------------------------------------
//---------------------------------------------------------

class BOSHead{

public:

  BOSHead(){}
  ~BOSHead(){}

  int Read(int fd);

  char BankName[5];
  int RevisionNumber;
  int Reserved;
  int NWords;
};


class ChannelInfo{

public:

  ChannelInfo(){}
  ~ChannelInfo(){}

  int Read(int fd);
  int SaveInfo(ofstream &out);

  char DetType[5];
  int DetID;
  char ModuleType[5];
  char channel;
  char module;
  char chassis;
  char stream;
  int SampleRate;
  int SampleSize;
  int FullScale;
  int DelayTime;
  int Threshold;
  int ThSign;
  float Offset;
  int PreSample;
  int PostSample;
  char ClockState[5];

  friend ostream& operator<< ( ostream& out, ChannelInfo& a ) {
    return out << a.DetType << " " << a.DetID;
  }
};



class ModuleInfo{

public:

  ModuleInfo();
  ~ModuleInfo();

  int Read(int fd);
  int SaveInfo(char* filename);
  int GetChannelNumber(string DetName,int DetID);

  BOSHead theBOSHead;
  int NChannels;
  ChannelInfo* theChannelInfo;

  friend ostream& operator<< ( ostream& out, ModuleInfo& a ) {
    for ( int i = 0; i < a.NChannels; ++i ) out << a.theChannelInfo[i] << endl;
    return out;
  }
};

//---------------------------------------------------------
//---------------------------------------------------------

#define RRMAXEVENTSINFILE 100000
#define RRMAXNCHANNELS 16


class DataStreamPos{

public:

  DataStreamPos(){}
  ~DataStreamPos(){}

  int Read(int fd);
  int GetRunHeaderPos(){return RunHeaderPos;}
  int GetModuleHeaderPos(){return ModuleHeaderPos;}
  int GetEventHeaderPos(int nevent);
  int GetNEvents();
  int GetChannelPos(int nEvent,int nChannel);
  int GetNChannels(int nEvent);
  int SaveInfo(char* filename);
  int ReadSavedInfo(char* filename);

  int RunHeaderPos;
  int ModuleHeaderPos;
  int EventHeaderPos[RRMAXEVENTSINFILE];
  int ChannelHeaderPos[RRMAXEVENTSINFILE][RRMAXNCHANNELS+1];
  BOSHead theBOSHead;
  int FileSize;
  int NEvents;
};



class SignalInfo{

public:

  SignalInfo();
  ~SignalInfo();

  int Read(int fd);
  int Write(ofstream &out);
  void SwapBytes();

  unsigned int TimeStamp;
  unsigned int PulseLength;
  unsigned char* data; 
};



#define RRMAXNSIGNALS 10000

class ACQCInfo{

public:

  ACQCInfo();
  ~ACQCInfo();

  int Read(int fd);
  int PrintInfo();

  BOSHead theBOSHead;
  char DetectorName[5];
  int DetectorID;
  char channel,module,chassis,stream;
  int NSignals;
  SignalInfo* theSignal[RRMAXNSIGNALS];

  friend ostream& operator<< ( ostream& out, ACQCInfo& a ) {
    return out << a.DetectorName << " " << a.DetectorID << " " << a.NSignals;
  }
};


//---------------------------------------------------------
//---------------------------------------------------------

class RunReader{

 public:
  RunReader();
  ~RunReader();

  void SetDirName(string dirname){DirName=dirname;}
  void SetDetName(string detname){DetName=detname;}
  void SetDetID(int detid){DetID=detid;}
  void SetRunNumber(int run){RunN=run;}
  void SetSegmentNumber(int seg){SegmentN=seg;}

  ModuleInfo* GetStructure ();
  int Init();
  int TakeEvent(int event);

  ACQCInfo* GetACQCInfo(){return &theACQCInfo;}
  int GetPreSamples(){return theModuleInfo->theChannelInfo[ChannelN].PreSample;}
  int GetSampleRate(){return theModuleInfo->theChannelInfo[ChannelN].SampleRate;}
  int GetSampleSize(){return theModuleInfo->theChannelInfo[ChannelN].SampleSize;}
  ChannelInfo* GetChannelInfo(){return &(theModuleInfo->theChannelInfo[ChannelN]);}

  string DirName;
  string DetName;
  int DetID;
  int RunN;
  int SegmentN;

  char filename[100];
  int StreamN;
  int ChannelN;
  int ACQCChannelN;
  int fd;
  int NEvents;

  ModuleInfo* theModuleInfo;
  DataStreamPos* theStreampos;
  ACQCInfo theACQCInfo;

};

//---------------------------------------------------------
//---------------------------------------------------------


#endif


