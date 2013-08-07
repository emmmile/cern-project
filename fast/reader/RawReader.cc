

#include "RawReader.hh"

#include <sys/stat.h> 
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <shift.h>
#include <dirent.h>

using namespace std;


//---------------------------------------------------------
//---------------------------------------------------------


int BOSHead::Read(int fd){
  rfio_read(fd,&(BankName[0]),sizeof(char)*4);
  rfio_read(fd,&RevisionNumber,sizeof(int));
  rfio_read(fd,&Reserved,sizeof(int));
  rfio_read(fd,&NWords,sizeof(int));
  BankName[4]='\0';
  return 0;
}

//---------------------------------------------------------
//---------------------------------------------------------

int ChannelInfo::Read(int fd){
  rfio_read(fd,&(DetType[0]),sizeof(char)*4);
  rfio_read(fd,&DetID,sizeof(int));
  rfio_read(fd,&(ModuleType[0]),sizeof(char)*4);
  rfio_read(fd,&channel,sizeof(char));
  rfio_read(fd,&module,sizeof(char));
  rfio_read(fd,&chassis,sizeof(char));
  rfio_read(fd,&stream,sizeof(char));
  rfio_read(fd,&SampleRate,sizeof(int));
  rfio_read(fd,&SampleSize,sizeof(int));
  rfio_read(fd,&FullScale,sizeof(int));
  rfio_read(fd,&DelayTime,sizeof(int));
  rfio_read(fd,&Threshold,sizeof(int));
  rfio_read(fd,&ThSign,sizeof(int));
  rfio_read(fd,&Offset,sizeof(float));
  rfio_read(fd,&PreSample,sizeof(int));
  rfio_read(fd,&PostSample,sizeof(int));
  rfio_read(fd,&(ClockState[0]),sizeof(char)*4);

  DetType[4]='\0';
  ModuleType[4]='\0';
  ClockState[4]='\0';

  return 0;
}


int ChannelInfo::SaveInfo(ofstream &out){
  
  out<<DetType<<"  "<<DetID<<"  "<<ModuleType<<endl;
  out<<(int)channel<<"  "<<(int)module<<"  "<<(int)chassis<<"  "<<(int)stream<<endl;
  out<<SampleRate<<"  "<<SampleSize<<"  "<<FullScale
     <<"  "<<DelayTime<<"  "<<Threshold<<"  "<<ThSign<<endl;
  out<<Offset<<"  "<<PreSample<<"  "<<PostSample<<"  "<<ClockState<<endl;
  
  return 0;
}

//---------------------------------------------------------
//---------------------------------------------------------

ModuleInfo::ModuleInfo(){
  theChannelInfo=0;
}

ModuleInfo::~ModuleInfo(){
  if(theChannelInfo){
    delete [] theChannelInfo;
  }
}

int ModuleInfo::GetChannelNumber(string DetName,int DetID){

  for(int i=0;i<NChannels;i++){
    if(string(theChannelInfo[i].DetType)==DetName && theChannelInfo[i].DetID==DetID){
      return i;
    }
  }

  return -1;
}

int ModuleInfo::Read(int fd){
  theBOSHead.Read(fd);
  rfio_read(fd,&NChannels,sizeof(int));

  if(theChannelInfo){
    delete [] theChannelInfo;
    theChannelInfo=0;
  }
  if(NChannels>0){
    theChannelInfo=new ChannelInfo[NChannels];
  }
  for(int i=0;i<NChannels;i++){
    theChannelInfo[i].Read(fd);
  }

  return 0;
}

int ModuleInfo::SaveInfo(char* filename){
  ofstream out(filename);
  if(!out.good()){
    cout<<" **** Error opening output file "<<filename<<" ****"<<endl;
    return -1;
  }

  out<<NChannels<<endl;
  for(int i=0;i<NChannels;i++){
    theChannelInfo[i].SaveInfo(out);
  }

  return 0;
}


//---------------------------------------------------------
//---------------------------------------------------------

int DataStreamPos::Read(int fd){
  struct stat theStat;
  rfio_fstat(fd,&theStat);
  FileSize=theStat.st_size;
  int thePos=0;
  NEvents=0;
  int nchannel=0;
  int check=0;

  while(thePos<FileSize){
    check=rfio_lseek(fd,thePos,SEEK_SET);
    if(check==-1){
      cout<<" **** Error in DataStreamPos::Read -> rfio_lseek("<<
	thePos<<") ****"<<endl;
      return -1;
    }
    check=theBOSHead.Read(fd);
    if(check!=0){
      cout<<" **** Error in DataStreamPos::Read -> theBOSHead.Read ****"<<endl;
      return -1;
    }


    //cout<<string(theBOSHead.BankName)<<"  "<<thePos<<"  "<<FileSize<<endl;

    if(string(theBOSHead.BankName)==string("RCTR")){
      RunHeaderPos=thePos;
    }
    else if(string(theBOSHead.BankName)==string("MODH")){
      ModuleHeaderPos=thePos;
    }
    else if(string(theBOSHead.BankName)==string("EVEH")){
      EventHeaderPos[NEvents]=thePos;
      ChannelHeaderPos[NEvents][0]=0;
      NEvents++;
      nchannel=1;
    }
    else if(string(theBOSHead.BankName)==string("ACQC")){
      ChannelHeaderPos[NEvents-1][0]++;
      nchannel=ChannelHeaderPos[NEvents-1][0];
      ChannelHeaderPos[NEvents-1][nchannel]=thePos;
    }
    else{
      cout<<" **** Error in DataStreamPos::Read -> invalid BankName: ";
      cout<<theBOSHead.BankName<<" ****"<<endl;
      return -1;
    }
    thePos+=16+theBOSHead.NWords*4;
  }

  return 0;
}

int DataStreamPos::GetEventHeaderPos(int nevent){
  if(nevent<=NEvents){
    return EventHeaderPos[nevent-1];
  }
  return -1;
}

int DataStreamPos::GetNEvents(){
  return NEvents;
}

int DataStreamPos::GetChannelPos(int nEvent,int nChannel){
  if(nEvent>NEvents){
    return -1;
  }
  if(nChannel>ChannelHeaderPos[nEvent-1][0]){
    return -1;
  }
  return ChannelHeaderPos[nEvent-1][nChannel];
}

int DataStreamPos::GetNChannels(int nEvent){
  if(nEvent>NEvents){
    return -1;
  }
  return ChannelHeaderPos[nEvent-1][0];
}

int DataStreamPos::SaveInfo(char* filename){
  ofstream out(filename);
  if(!out.good()){
    cout<<" **** Error opening output file "<<filename<<" ****"<<endl;
    return -1;
  }
  out<<RunHeaderPos<<"  "<<ModuleHeaderPos<<"  "<<FileSize
     <<"  "<<NEvents<<endl;
  for(int i=0;i<NEvents;i++){
    out<<EventHeaderPos[i]<<"  ";
    if((i+1)%10==0){out<<endl;}
  }
  out<<endl;
  for(int i=0;i<NEvents;i++){
    for(int j=0;j<RRMAXNCHANNELS+1;j++){
      out<<ChannelHeaderPos[i][j]<<"  ";
    }
    out<<endl;
  }
  return 0;
}

int DataStreamPos::ReadSavedInfo(char* filename){
  ifstream in(filename);
  if(!in.good()){
    cout<<" **** Error opening input file "<<filename<<" ****"<<endl;
    return -1;
  }
  in>>RunHeaderPos>>ModuleHeaderPos>>FileSize>>NEvents;
  for(int i=0;i<NEvents;i++){in>>EventHeaderPos[i];}
  for(int i=0;i<NEvents;i++){
    for(int j=0;j<RRMAXNCHANNELS+1;j++){in>>ChannelHeaderPos[i][j];}
  }
  if(!in.good()){
    cout<<" **** Error reading input file "<<filename<<" ****"<<endl;
    return -1;
  }
  return 0;
}

//---------------------------------------------------------
//---------------------------------------------------------


SignalInfo::SignalInfo(){
  data=0;
}
SignalInfo::~SignalInfo(){
  if(data){delete [] data;}
}

int SignalInfo::Read(int fd){
  rfio_read(fd,&TimeStamp,sizeof(unsigned int));
  rfio_read(fd,&PulseLength,sizeof(unsigned int));
  SwapBytes();
  if(data){delete [] data; data=0;}
  if(PulseLength!=0){
    data=new unsigned char[PulseLength];
    rfio_read(fd,&(data[0]),sizeof(char)*PulseLength);
  }
  return 0;
}

int SignalInfo::Write(ofstream &out){
  if(data!=0){
    out<<TimeStamp<<"  "<<PulseLength<<endl;
    for(unsigned int i=0;i<PulseLength;i++){
      out<<"  "<<(int)data[i];
      if((i+1)%10==0){out<<endl;}
    }
    out<<endl;
  }
  return 0;
}

void SignalInfo::SwapBytes(){
  union intandchar{
    unsigned int l;
    unsigned char b[4];
  } temp1,temp2;

  temp1.l=TimeStamp;
  temp2.b[0]=temp1.b[3]; temp2.b[1]=temp1.b[2]; 
  temp2.b[2]=temp1.b[1]; temp2.b[3]=temp1.b[0]; 
  TimeStamp=temp2.l;

  temp1.l=PulseLength;
  temp2.b[0]=temp1.b[3]; temp2.b[1]=temp1.b[2]; 
  temp2.b[2]=temp1.b[1]; temp2.b[3]=temp1.b[0]; 
  PulseLength=temp2.l;
}

//---------------------------------------------------------
//---------------------------------------------------------


ACQCInfo::ACQCInfo(){
  for(int i=0;i<RRMAXNSIGNALS;i++){theSignal[i]=0;}
  NSignals=0;
}
ACQCInfo::~ACQCInfo(){
  for(int i=0;i<RRMAXNSIGNALS;i++){
    if(theSignal[i]!=0){delete theSignal[i];}
  }
}

int ACQCInfo::Read(int fd){
  theBOSHead.Read(fd);
  rfio_read(fd,&(DetectorName[0]),sizeof(char)*4);
  rfio_read(fd,&DetectorID,sizeof(int));
  rfio_read(fd,&channel,sizeof(char));
  rfio_read(fd,&module,sizeof(char));
  rfio_read(fd,&chassis,sizeof(char));
  rfio_read(fd,&stream,sizeof(char));
  DetectorName[4]='\0';

  int theLocalPos=3*4;
  int theTotalSize=theBOSHead.NWords*4;
  NSignals=0;
  while((theTotalSize-theLocalPos)>10){
    if(theSignal[NSignals]!=0){delete theSignal[NSignals];}
    theSignal[NSignals]=new SignalInfo();
    theSignal[NSignals]->Read(fd);
    theLocalPos+=8+theSignal[NSignals]->PulseLength;
    NSignals++;
    if(NSignals>=RRMAXNSIGNALS){
      cout<<" **** Error in file "<<__FILE__<<" , line "<<__LINE__<<" ****"<<endl;
      cout<<" ---> Number of signals exceed max.="<<RRMAXNSIGNALS<<endl;
      exit(1);
    }
  }

  return 0;
}

int ACQCInfo::PrintInfo(){
  cout<<"------------------------------------"<<endl;
  cout<<"DetectorName: "<<DetectorName<<endl;
  cout<<"DetectorID: "<<DetectorID<<endl;
  cout<<"channel: "<<(int)channel<<endl;
  cout<<"module: "<<(int)module<<endl;
  cout<<"chassis: "<<(int)chassis<<endl;
  cout<<"stream: "<<(int)stream<<endl;
  cout<<"NSignals: "<<NSignals<<endl;
  cout<<"------------------------------------"<<endl;
  return 0;
}

//---------------------------------------------------------
//---------------------------------------------------------

RunReader::RunReader(){
  theModuleInfo=0;
  theStreampos=0;
}


RunReader::~RunReader(){
  if(theModuleInfo){delete theModuleInfo;}
  if(theStreampos){delete theStreampos;}
}

ModuleInfo* RunReader::GetStructure() {
  //We delete existing memory, if needed:
  if(theModuleInfo){delete theModuleInfo; theModuleInfo=0;}
  if(theStreampos){delete theStreampos; theStreampos=0;}

  //We take the module info:
  sprintf(filename,"%s/stream1/run%d_%d_s1.raw.finished" ,DirName.c_str(),RunN,SegmentN);
  fd=rfio_open(filename,O_RDONLY);
  if(fd<0){
    cout<<" **** Error opening file <"<<filename<<"> ****"<<endl;
    return NULL;
  }
  //Skip the RunInfo:
  BOSHead theBOSHead;
  theBOSHead.Read(fd);
  rfio_lseek(fd,16+theBOSHead.NWords*4,SEEK_SET);
  //Now we take the module info:
  theModuleInfo=new ModuleInfo();
  theModuleInfo->Read(fd);

  return theModuleInfo;
}

int RunReader::Init(){
  // this has been already called
  if ( !GetStructure() ) return -1;

  ChannelN=theModuleInfo->GetChannelNumber(DetName,DetID);
  if(ChannelN<0){
    cout<<" **** Detector: "<<DetName<<"  "<<DetID<<" not found ****"<<endl;
    return -1;
  }
  StreamN=(int)(theModuleInfo->theChannelInfo[ChannelN].stream);
  rfio_close(fd);

  //We open the file and make an index:
  sprintf(filename,"%s/stream%d/run%d_%d_s%d.raw.finished"
	  ,DirName.c_str(),StreamN,RunN,SegmentN,StreamN);
  fd=rfio_open(filename,O_RDONLY);
  if(fd<0){
    cout<<" **** Error opening file <"<<filename<<"> ****"<<endl;
    return -1;
  }
  theStreampos=new DataStreamPos();
  theStreampos->Read(fd);
  NEvents=theStreampos->GetNEvents();

  //We take the ACQCChannelN:
  if(NEvents<1){
    cout<<" **** Number of events is "<<NEvents<<" ****"<<endl;
    exit(1);
  }
  int nChInStream=theStreampos->GetNChannels(1);
  ACQCChannelN=-1;
  for(int i=0;i<nChInStream;i++){
    int thePos=theStreampos->GetChannelPos(1,i+1);
    rfio_lseek(fd,thePos,SEEK_SET);
    theACQCInfo.Read(fd);
    if(string(theACQCInfo.DetectorName)==DetName && theACQCInfo.DetectorID==DetID){
      ACQCChannelN=i+1;
      break;
    }
  }
 
  if(ACQCChannelN<0){
    cout<<" **** Error searching ACQCChannelN (ACQCChannelN="<<ACQCChannelN<<")****"<<endl;
    exit(1);
  }

  return NEvents;
}


int RunReader::TakeEvent(int event){

  if(event<1 || event>NEvents){
    cout<<" **** Error: incorrect event number required: "<<event<<" ****"<<endl;
    return -1;
  }

  int thePos=theStreampos->GetChannelPos(event,ACQCChannelN);
  rfio_lseek(fd,thePos,SEEK_SET);
  theACQCInfo.Read(fd);

  //Simple check:
  if(string(theACQCInfo.DetectorName)!=DetName || theACQCInfo.DetectorID!=DetID){
    cout<<" **** Error: we're reading wrong detector: "
	<<theACQCInfo.DetectorName<<"  "<<theACQCInfo.DetectorID<<" ****"<<endl;
    return -1;
  }

  return theACQCInfo.NSignals;
}


//---------------------------------------------------------
//---------------------------------------------------------
