/***************************************************************************
                          header.h  -  description
                             -------------------
    begin                : Wed Oct 4 2000
    copyright            : (C) 2000,2001,2002 by Ralf Plag & Erich Schaefer
    email                : Ralf.Plag@cern.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "header.h"
#include <stdio.h>
#include "DaqDefs.h"
#include <unistd.h>  // for write()

#define NTOF_as_int 0x464f544e

#define rctrRevNbr 1
#define modhRevNbr 0
#define evehRevNbr 0
#define acqcRevNbr 0
#define indxRevNbr 0

_BOS::_BOS() {
		bos = new BOSHEADER();
		}
		 
_BOS::_BOS(u_int *ptr) {  
		bos = (BOSHEADER *)ptr;
		}
		
_BOS::_BOS(u_int title, u_int revNumber,u_int length) {
		bos = new BOSHEADER();
		bos->title.l   = title;
		bos->revNumber = revNumber;
		bos->reserved  = 0;
		bos->length    = length;
		}
		
_BOS::~_BOS() {
		//cout << form("calling ~_BOS()  bos %x \n",bos);
		if (bos) delete bos;		// this is a problem in acq ????
		}
		
_BOS::_BOS(u_int *ptr,u_int title, u_int revNumber, u_int length) {
   	*ptr++	= title;
   	*ptr++	= revNumber;
   	*ptr++  = 0;			// reserved
   	*ptr++	= length;
   	}	

bool _BOS::writeToFile(int file)
{
return (write(file,bos,sizeof(BOSHEADER))==sizeof(BOSHEADER));
}
	
bool _BOS::readFromFile(f_i *file)
{
return (file->read((char*)bos,sizeof(BOSHEADER))==sizeof(BOSHEADER));
}

BOSHEADER *_BOS::getBOS() { return bos; }

void _BOS::setBOS(u_int *ptr) {
		memcpy(bos,ptr,sizeof(BOSHEADER) );		
		}
		
char *_BOS::getTitle( ) {
		char *str = new char[5];
		strncat(str,bos->title.c,4);
		return str;
		}


// -- RCTR -------------------------------------------------------------------
// -- RCTR -------------------------------------------------------------------

_BOS_RCTR::_BOS_RCTR() : _BOS()
{ rctr = new RCTR_HEADER; }


/* This creates a header by determining the hostname and reading
   the corresponding information out of the READOUT.conf file.
*/
_BOS_RCTR::_BOS_RCTR(u_int runNumber,u_int runExtNumber)
   : _BOS(*(u_int *)"RCTR",rctrRevNbr,0)
{
	rctr = new RCTR_HEADER;
  rctr->streamNumber=0;
  rctr->numberOfModules=0;
  nextFile(runNumber,runExtNumber);
  // read number of streams
  GetDefaultUnsigned(rctr->totalNumberOfStreams,configFile,"numberOfStreams");

  rctr->totalNumberOfChassis=rctr->totalNumberOfStreams;
  if (rctr->totalNumberOfChassis>MAXNumberOfStreams) {
     cout << "Too much streams !" << endl;
     return;
     }

  // determine hostname
  char chostname[100];
  gethostname(chostname,100);
  char* point=strchr(chostname,'.');   // remove domain name	
  if (point) *point=0;
  string thisHost = chostname;


  // read config data & try to determine streamNumber
  Config_V myConf[MAXNumberOfStreams];    		// run configuration: detektor id, card, channel and settings
  for(unsigned int i = 0;i<rctr->totalNumberOfStreams;i++) {
	char streamHostStr[10];
	sprintf(streamHostStr,"host%d",i+1);
//	string streamHostStr = form("host%d",i+1);
	string streamHost;
	GetDefaultString(streamHost,configFile,streamHostStr);
	myConf[i] = GetConfigVal(configFile,streamHost+":");
      if (streamHost==thisHost) {
          if (myConf[i].size()>0)
             rctr->streamNumber=myConf[i][0].getStream();
          else
             cout << "There is no configuration for this stream !" << endl;
          }
			}


  // determine # of modules (total) and # of ch per mod
  for (u_int s=0;s<rctr->totalNumberOfStreams;s++) {
     bool found;
     int curModules=0;    // numberOfModules in stream s
     // look for the existence of modules with number 0..16
     for (u_int module=0;module<=16;module++)  {     // module number to check for
       int curChNumber=0; // number of of channels in the current module
       found=false;       // if module 'module' was found in the current stream
       for (u_int i=0;i<myConf[s].size();i++) { // look for the module in all channels of current stream
          if (myConf[s][i].getModule()==module) {
             curChNumber++;
             found=true;
             } // if myConf
          } // for i
       if (found) {
           numberOfChannels.push_back(curChNumber);
           curModules++;
           }
       };

     if (s+1==rctr->streamNumber) rctr->numberOfModules=curModules; // in this file
     } // for s
  rctr->totalNumberOfModules=numberOfChannels.size(); //total in run
  rctr->numberOfChannels=(rctr->streamNumber>0) ? myConf[rctr->streamNumber-1].size() : 0;
  rctr->experiment = NTOF_as_int;
  bos->length=sizeof(RCTR_HEADER)/sizeof(int)+rctr->totalNumberOfModules;
}

u_int _BOS_RCTR::getSize() {
  return (sizeof(RCTR_HEADER)+sizeof(BOSHEADER))/sizeof(int)
        +rctr->totalNumberOfModules;
};

/* This sets a new run extension number and a new date, which is
   necessary to write a new file if the current reaches the 2 GB limit.
*/
void _BOS_RCTR::nextFile(u_int runNbr,u_int runExt)
{
	time_t now = time( (time_t *) 0);
	systemTime = localtime((const time_t *) &now);
  	u_int date = systemTime->tm_year   * 10000 + (systemTime->tm_mon+1) * 100 + systemTime->tm_mday;
    	u_int time = systemTime->tm_hour   * 10000 + systemTime->tm_min    * 100 + systemTime->tm_sec;
 	rctr->date = date;
 	rctr->time = time;
  rctr->runNumber=runNbr;
	rctr->runExtNumber = runExt;
}

bool _BOS_RCTR::writeToFile(int file)
{
if (!_BOS::writeToFile(file)) return false;
cout << "RCTR: write to file" << endl;
if (write(file,rctr,sizeof(RCTR_HEADER))!=sizeof(RCTR_HEADER)) return false;
for (u_int i=0;i<numberOfChannels.size();i++)
  if (write(file,&(numberOfChannels[i]),sizeof(int))!=sizeof(int)) return false;
return true;
}


bool _BOS_RCTR::readFromFile(_BOS *aBos,f_i *file)
{
*bos=*(aBos->bos);
if (file->read((char*)rctr,sizeof(RCTR_HEADER))!=sizeof(RCTR_HEADER)) return false;
numberOfChannels.clear();
for (u_int i=0;i<rctr->totalNumberOfModules;i++) {
  int nbrOfCh;
  if (file->read((char*)&nbrOfCh,sizeof(int))!=sizeof(int)) return false;
  numberOfChannels.push_back(nbrOfCh);
  }
return true;
}

// -- MODH -------------------------------------------------------------------
// -- MODH -------------------------------------------------------------------

	

/*\
 *	Here we define the module specific header 
 *	the settings for each module are the same
 * 	over the lifetime of the run
\*/

/* If autoCreate is true, this constructor extracts all necessary
  data out of the READOUT.conf file
*/
_BOS_MODH::_BOS_MODH( bool autoCreate)
   : _BOS(*(u_int *)"MODH",modhRevNbr,0)
{
  if (!autoCreate) return;

  // read number of streams
  numberOfStreams=0;
  if (!GetDefaultUnsigned(numberOfStreams,configFile,"numberOfStreams")) {
     cout << "MODH: Cannot read number of streams !"<< endl;
     return;
     }

  // read config data
  numberOfChannels=0;
	for (u_int i = 0;i<numberOfStreams;i++) {
		char streamHostStr[10];
		sprintf(streamHostStr,"host%d",i+1);
		//string streamHostStr = form("host%d",i+1);
     string streamHost;
     GetDefaultString(streamHost,configFile,streamHostStr);
     myConf[i] = GetConfigVal(configFile,streamHost+":");
     numberOfChannels+=myConf[i].size();
		 }	
  cout << "MODH: Number of channels: " << numberOfChannels << endl;
  cout << "MODH: Number of streams:  " << numberOfStreams << endl;
  bos->length=numberOfChannels*sizeof(Channel_Config)/sizeof(int)+1;
}

Config_V _BOS_MODH::getConfigList(u_int streamNumber)
{
if (streamNumber>=numberOfStreams) {
   cout << "MODH: stream not available !" << endl;
   exit(1);
   }
return myConf[streamNumber];
}

Channel_Config _BOS_MODH::getConfigForChannel(u_int channel)
{
int curChannel=0;
for (u_int s=0;s<numberOfStreams;s++) {
  if (curChannel+myConf[s].size()>channel)
     return myConf[s][channel-curChannel];
  curChannel+=myConf[s].size();
  }
printf("MODH: Channel %d not found !\n",channel);
cout << "numberOfStreams = " << numberOfStreams << endl;
cout << "numberOfChannels in first stream = " << myConf[0].size() << endl;
Channel_Config dummy;
return dummy;
}

Channel_Config _BOS_MODH::getConfigForStCrMoCh(u_int stream_crate_mod_ch)
{
for (u_int s=0;s<numberOfStreams;s++) {
  for (u_int c=0;c<myConf[s].size();c++) {
    if (myConf[s][c].str_crate_mod_ch==stream_crate_mod_ch) return myConf[s][c];
    }
  }
printf("MODH: Channel 0x%x not found !\n",stream_crate_mod_ch);
Channel_Config dummy;
return dummy;
}

int _BOS_MODH::indexOf(u_int stream_crate_mod_ch)
{
int index=0;
for (u_int s=0;s<numberOfStreams;s++) {
  for (u_int i=0;i<myConf[s].size();i++) {
    if (myConf[s][i].str_crate_mod_ch==stream_crate_mod_ch) return index;
    index++;
    }
  }
return -1;
}

bool _BOS_MODH::writeToFile(int file)
{
if (!_BOS::writeToFile(file)) return false;
if (write(file,&numberOfChannels,sizeof(int))!=sizeof(int)) return false;
for (u_int s=0;s<numberOfStreams;s++) {
  for (u_int c=0;c<myConf[s].size();c++) {
    Channel_Config dummy=myConf[s][c];
    if (write(file,&dummy,sizeof(Channel_Config))!=sizeof(Channel_Config)) return false;
    }
  }
return true;
}

/* The trick is here to store a config-struct in the right list.
   There is a list for each host. Fortunately, all channels
   of a host are stored in a group on the hard disk.
*/

bool _BOS_MODH::readFromFile(_BOS* aBos,f_i *file)
{
*bos=*(aBos->bos);
if (file->read((char*)&numberOfChannels,sizeof(int))!=sizeof(int)) return false;
numberOfStreams=0;
int curStream=-1;
for (u_int s=0;s<numberOfChannels;s++) {
   Channel_Config aConfig;
   if (file->read((char*)&aConfig,sizeof(Channel_Config))!=sizeof(Channel_Config)) return false;

   if (aConfig.getStream()!=(u_int)curStream) { // when the streamNumber changes, the numberOfStreams has to be incremented
      numberOfStreams++;
      curStream=aConfig.getStream();
      }

   myConf[curStream-1].push_back(aConfig);
   }
return true;
}


void _BOS_MODH::print()
{
cout << endl << "Detector configuration" << endl << Channel_Config::getDescriptionTitle();
for (u_int s=0;s<numberOfStreams;s++)
  for (u_int c=0;c<myConf[s].size();c++)
     cout << myConf[s][c].getDescription();
cout << endl;
}


// -- EVEH -------------------------------------------------------------------
// -- EVEH -------------------------------------------------------------------

_BOS_EVEH::_BOS_EVEH(u_int eventNumber,u_int runNumber)
   : _BOS(*(u_int *)"EVEH",evehRevNbr,sizeof(EVENT_HEADER)/sizeof(int))
{
	time_t now = time( (time_t *) 0);
	
	eveh = new EVENT_HEADER;
	eveh->sizeOfEvent = 0;
	eveh->eventNumber = eventNumber;
 	eveh->runNumber   = runNumber;
	
	systemTime = localtime((const time_t *) &now);
 	u_int date = systemTime->tm_year   * 10000 + (systemTime->tm_mon+1) * 100 + systemTime->tm_mday;
 	u_int time = systemTime->tm_hour   * 10000 + systemTime->tm_min    * 100 + systemTime->tm_sec;
	
 	eveh->date = date;
 	eveh->time = time;
}

u_int _BOS_EVEH::writeToFile(int file)
{
if (!_BOS::writeToFile(file)) return 0;
if (write(file,eveh,sizeof(EVENT_HEADER))!=sizeof(EVENT_HEADER)) return 0;
return 1;
}


bool _BOS_EVEH::readFromFile(_BOS *aBos,f_i *file)
{
*bos=*(aBos->bos);
return (file->read((char*)eveh,sizeof(EVENT_HEADER))==sizeof(EVENT_HEADER));
}

// -- ACQC ---------------------------------------------------------------------
// -- ACQC ---------------------------------------------------------------------

_BOS_ACQC::_BOS_ACQC()
   : _BOS(*(u_int *)"ACQC",acqcRevNbr,sizeof(ACQC_HEADER)/sizeof(int))  // this line included, 13.4.02, Ralf
{
  acqc=new ACQC_HEADER;
}

_BOS_ACQC::_BOS_ACQC(Channel_Config* channel)
   : _BOS(*(u_int *)"ACQC",acqcRevNbr,sizeof(ACQC_HEADER)/sizeof(int))
{
	acqc = new ACQC_HEADER;
  acqc->detectorType=channel->id;
  acqc->detectorChannel=channel->detectorChannel;
  acqc->stream_crate_mod_ch=channel->str_crate_mod_ch;
}

void _BOS_ACQC::setConfigData(Channel_Config* channel)
{
  acqc->detectorType=channel->id;
  acqc->detectorChannel=channel->detectorChannel;
  acqc->stream_crate_mod_ch=channel->str_crate_mod_ch;
}

bool _BOS_ACQC::writeToFile(int file)
{
if (!_BOS::writeToFile(file)) return false;
if (write(file,acqc,sizeof(ACQC_HEADER))!=sizeof(ACQC_HEADER)) return false;
return true;
}


bool _BOS_ACQC::readFromFile(_BOS *aBos,f_i *file)
{
*bos=*(aBos->bos);
return (file->read((char*)acqc,sizeof(ACQC_HEADER))==sizeof(ACQC_HEADER));
}
	
// -- INDX ---------------------------------------------------------------------
// -- INDX ---------------------------------------------------------------------

_BOS_INDX::_BOS_INDX()
    : _BOS(*(u_int *)"INDX",indxRevNbr,0)
{
  // read number of streams
  for(u_int i = 0 ; i<MAXNumberOfStreams;i++)streamPosition[i] = 0;
  u_int numberOfStreams=0;
  if (!GetDefaultUnsigned(numberOfStreams,configFile,"numberOfStreams")) {
     cout << "INDX: Cannot read number of streams !"<< endl;
     exit(1);
     }
  bos->length=numberOfStreams;
}

void _BOS_INDX::setPosition(u_int streamNumber,u_int position)
{
//printf("SetPosition bos=%d\n",bos);
if (streamNumber<bos->length)
   streamPosition[streamNumber]=position;
else
  cout << "INDX: Wrong stream number ! (set)" << endl;
}

u_int _BOS_INDX::getPosition(u_int streamNumber)
{
if (streamNumber<bos->length)
   return streamPosition[streamNumber];
cout << "INDX: Wrong stream number ! (get)" << endl;
return 0;
}

bool _BOS_INDX::writeToFile(int file)
{
if (!_BOS::writeToFile(file)) return false;
//cout << "INDX: writing " << bos->length*4 << " bytes" << endl;
if (write(file,(char*)streamPosition,bos->length*4)!=(int)bos->length*4) return false;
return true;
}

bool _BOS_INDX::readFromFile(_BOS *aBos,f_i *file)
{
*bos=*(aBos->bos);
return (file->read((char*)streamPosition,bos->length*4)==bos->length*4);
}


// -- BEAM ---------------------------------------------------------------------
// -- BEAM ---------------------------------------------------------------------

// change Bos length and rev. number !

//void _BOS_BEAM::setAttr(float aDate,float aIntensity)
void _BOS_BEAM::setAttr(float aDate,float aIntensity,int beamType)
{
data[0]=aDate;
data[1]=aIntensity;
type=beamType;
}

bool _BOS_BEAM::writeToFile(int file)
{
if (!_BOS::writeToFile(file)) return false;
if (write(file,&data[0],2*sizeof(float))!=8) return false;
if (write(file,&type   ,  sizeof(int)  )!=4) return false;
return true;
}

bool _BOS_BEAM::readFromFile(_BOS *aBos,f_i *file)
{
*bos=*(aBos->bos);
if (file->read((char*)data,8)!=8) return false;    // read 2* float
if (getRevNumber()==0) return true;
return (file->read((char*)&type,4)==4);  // read beam type (int), since RevNumber=1
}

// -- SLOW ---------------------------------------------------------------------
// -- SLOW ---------------------------------------------------------------------

_BOS_SLOW::_BOS_SLOW() : _BOS(*(u_int *)"SLOW",0,3)
{
numberOfHVValues=0;
numberOfLVValues=0;
numberOfSpecialValues=0;
hvList=NULL;
};


bool _BOS_SLOW::writeToFile(int file)
{
if (bos->length==0) setBosLength();  // this just safety
if (!_BOS::writeToFile(file)) return false;
// high voltages
if (write(file,&numberOfHVValues,sizeof(int))!=sizeof(int)) return false;
if (write(file,hvList,numberOfHVValues*sizeof(slow_hv))!=int(numberOfHVValues*sizeof(slow_hv))) return false;
// low voltages
if (write(file,&numberOfLVValues,sizeof(int))!=sizeof(int)) return false;
// special values
if (write(file,&numberOfSpecialValues,sizeof(int))!=sizeof(int)) return false;
for (u_int i=0;i<numberOfSpecialValues;i++)
  if (write(file,&svList[i],sizeof(slow_sv))!=sizeof(slow_sv)) return false;
return true;
}

bool _BOS_SLOW::readFromFile(_BOS *aBos,f_i *file)
{
*bos=*(aBos->bos);
u_int number=0;
if (file->read((char*)&number,sizeof(int))!=sizeof(int)) return false;
setNumberOfHVValues(number);
if (file->read((char*)hvList,number*sizeof(slow_hv))!=number*sizeof(slow_hv)) return false;
if (file->read((char*)&numberOfLVValues,sizeof(int))!=sizeof(int)) return false;
if (file->read((char*)&numberOfSpecialValues,sizeof(int))!=sizeof(int)) return false;
svList.clear();
for (u_int i=0;i<numberOfSpecialValues;i++) {
  slow_sv sv;
  if (file->read((char*)&sv,sizeof(slow_sv))!=sizeof(slow_sv)) return false;
  svList.push_back(sv);
  }
return true;
}

void _BOS_SLOW::setBosLength()
{
bos->length=numberOfHVValues*2+numberOfSpecialValues*2+3;
}


void _BOS_SLOW::setNumberOfHVValues(u_int number)
{
if (number==numberOfHVValues) return;
numberOfHVValues=number;
delete [] hvList;
if (number) hvList=new slow_hv[number]; else hvList=NULL;
setBosLength();
}

void _BOS_SLOW::setHVValue(u_int index,u_int StCrMoCh,float value)
{
if (index>=numberOfHVValues) {
  cout << "HV index too big !" << endl;
  return;
  }
hvList[index].stream_crate_mod_ch=StCrMoCh;
hvList[index].voltage=value;
}

void _BOS_SLOW::setSpecialValue(u_int ident,float val)
{
// check if ident is already in list
int index=-1;
for (u_int i=0;i<svList.size();i++)
  if (svList[i].identifier==ident) {index=i;break;};
if (index==-1) {       // create new entry
  slow_sv sv;
  sv.identifier=ident;
  sv.value=val;
  svList.push_back(sv);
  }
else
  svList[index].value=val;  // update existing
numberOfSpecialValues=svList.size();
setBosLength();
}

// -- INFO ---------------------------------------------------------------------
// -- INFO ---------------------------------------------------------------------

bool _BOS_INFO::writeToFile(int file)
{
cout << "writing bos" << endl;
if (!_BOS::writeToFile(file)) return false;
cout << "writing info data: " << data << endl;
return (write(file,data,bos->length*4)==(int)bos->length*4);
}

bool _BOS_INFO::readFromFile(_BOS *aBos,f_i *file)
{
*bos=*(aBos->bos);
if (data) delete [] data;
data=new char[4*bos->length];
return (file->read(data,4*bos->length)==4*bos->length);
}


void _BOS_INFO::setData(char* aData,u_int aDataSize)   // dataSize given in bytes
{
bos->length=u_int((aDataSize+3)/4);  // now in units of words

delete data;
data=new char[bos->length*4];

memcpy(data,aData,aDataSize);
}
// -- HIVO ---------------------------------------------------------------------
// -- HIVO ---------------------------------------------------------------------

bool _BOS_HIVO::writeToFile(int file)
{
if (!_BOS::writeToFile(file)) return false;
if (write(file,&colCount,4)!=4) return false;
if (write(file,&rowCount,4)!=4) return false;
if (write(file,&dataSize,4)!=4) return false;
return (write(file,data,dataSize)==dataSize);
}

bool _BOS_HIVO::readFromFile(_BOS *aBos,f_i *file)
{
*bos=*(aBos->bos);
file->read((char*)(&colCount),4);
file->read((char*)(&rowCount),4);
file->read((char*)(&dataSize),4);
if (dataSize>100000) return false;
delete [] data;
data=new char[dataSize];
return (file->read(data,dataSize)==u_int(dataSize));
}


void _BOS_HIVO::setData(char* aData,u_int aDataSize)   // dataSize given in bytes
{
dataSize=int((aDataSize+3)/4)*4;

delete data;
data=new char[dataSize];

// copy aData into buffer and remove white spaces and the stupid windows line break (13)
char* curData=aData;
char* curDest=data;
while (*curData!=0) {if ((*curData!=' ') && (*curData!=13)) *curDest++=*curData;curData++;};
*curDest=0;

dataSize=curDest-data+1;
int nbrWords=int((dataSize+3)/4);
dataSize=nbrWords*4;
bos->length=3+nbrWords;  // now in units of words

//memcpy(data,aData,aDataSize);
}

// -- END ---------------------------------------------------------------------
// -- END ---------------------------------------------------------------------

/*
_EVEH::_EVEH(u_int *ptr) { eveh = (EVENT_HEADER *)ptr;	}

_EVEH::_EVEH(u_int *ptr,u_int exparg,u_int runarg,u_int eventarg) {
  	time_t now = time( (time_t *) 0);

	eveh = (EVENT_HEADER *)ptr;
	eveh->sizeOfEvent = size;
	eveh->exp = exparg;
  	eveh->run = runarg;
	
	systemTime = localtime((const time_t *) &now);
  	u_int date = systemTime->tm_year   * 10000 + (systemTime->tm_mon+1) * 100 + systemTime->tm_mday;
    	u_int time = systemTime->tm_hour   * 10000 + systemTime->tm_min    * 100 + systemTime->tm_sec;
	
   	eveh->date = date;
   	eveh->time = time;
   	eveh->event = eventarg;
  	}

	void _EVEH::setEventSize(u_int arg) { eveh->sizeOfEvent = arg; }

	u_int _EVEH::getExp() { return eveh->exp; }
	u_int _EVEH::getRunNumber() { return eveh->run; }
	
	u_int _EVEH::getDate() { return eveh->date; }
	u_int _EVEH::getTime() { return eveh->time; }
	u_int _EVEH::getEventNumber() { return eveh->event; }
	u_int _EVEH::getEventSize() { return eveh->sizeOfEvent; }
*/


/*\
 *	Here we define the etep module specific header
\*/

/*
  _EMOD::_EMOD(u_int *ptr,Config_V conf_v) {
		channel_conf =  conf_v;
		
//		this will generate a linear buffer from the vector
		Channel_Config *tmp_config = (Channel_Config *)ptr;
				
		for(u_int i=0;i<conf_v.size();i++)
				tmp_config[i] = channel_conf[i];
		}

	Config_V _EMOD::getMODH() { return channel_conf                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              ; }
  void _EMOD::setMODH(Config_V aConf) {channel_conf=aConf;};

	u_int _EMOD::getStreamNumber(int lch) { 	return channel_conf[lch].streamNumber; }

  u_int _EMOD::getDetectorId(int lch) {
    if (lch==channel_conf.size()) return 0;
    return channel_conf[lch].id;
    }

  u_int _EMOD::getPhysModule(int lch) { 	return channel_conf[lch].module; }
	u_int _EMOD::getPhysChannel(int lch) { 	return channel_conf[lch].channel; }
	u_int _EMOD::getDetectorChannel(int lch)  { 	return channel_conf[lch].detectorChannel; }
	u_int _EMOD::getFullScale(int lch)        { 	return channel_conf[lch].fullScale; }
	u_int _EMOD::getPreSample(int lch) { 		return channel_conf[lch].pre; }
	u_int _EMOD::getPostSample(int lch) { 	return channel_conf[lch].post; }
	u_int _EMOD::getThreshold(int lch) { 		return channel_conf[lch].threshold; }
	u_int _EMOD::getSampleRate(int lch) { 	return channel_conf[lch].sampleRate; }
	u_int _EMOD::getSampleSize(int lch) { 	return channel_conf[lch].sampleSize; }


	_MODH::_MODH() { modh = new MOD_HEADER; }
	
	_MODH::_MODH(u_int *ptr) {
		modh = (MOD_HEADER *)ptr;
		}
	
  	_MODH::_MODH(u_int *ptr,MOD_HEADER *mptr) {
		modh = (MOD_HEADER *)ptr;
		memcpy(ptr,mptr,sizeof(MOD_HEADER) );		
		}

  	_MODH::_MODH(u_int *ptr,u_int sampleRate,u_int sampleSize,u_int fullScale,
		float delayTime,u_int threshold,u_int threshSign,u_int preSample,
		u_int postSample,u_int inputState, float offsetVoltage) {
		modh = (MOD_HEADER *)ptr;
		modh->sampleRate = sampleRate;
		modh->sampleSize = sampleSize;
		modh->fullScale = fullScale;
		modh->delayTime = delayTime;
		modh->threshold = threshold;
		modh->threshSign = threshSign;
		modh->preSample = preSample;
		modh->postSample = postSample;
		modh->inputState = inputState;
		modh->offsetVoltage = offsetVoltage;
		}	
			
	MOD_HEADER *_MODH::getMODH() { return modh; }
	u_int _MODH::getSampleRate() { return modh->sampleRate; }
	u_int _MODH::getSampleSize() { return modh->sampleSize; }
	u_int _MODH::getFullScale()  { return modh->fullScale; }
	float _MODH::getDelayTime() { return modh->delayTime; }
	u_int _MODH::getThreshold() { return modh->threshold; }
	u_int _MODH::getThresSign()  { return modh->threshSign; }
	u_int _MODH::getPreSample() { return modh->preSample; }
	u_int _MODH::getPostSample() { return modh->postSample; }
	u_int _MODH::getInputState()  { return modh->inputState; }
	// Offset Voltage was added later !!!
	//float _MODH::getOffsetVoltage()  { return (( getLength() > 10 ) ? modh->offsetVoltage : 0); }

	float _MODH::getOffsetVoltage()  { return modh->offsetVoltage; }


u_int _BOS_ACQC::getModId() {
    return 0;
		//return (0xff&getNumber());
		}

u_int _BOS_GEN::getModId() {
		return 0;
//    return (0xff&getNumber());
		}


  */

	
