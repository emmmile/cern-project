/***************************************************************************
                          unpackx.h  -  description
                             -------------------
    begin                : Fri Mar 9 2001
    copyright            : (C) 2001 by Ralf Plag
    email                : Ralf.Plag@cern.ch
 ***************************************************************************/

#ifndef UNPACKX_H
#define UNPACKX_H

//#include "fio.h"
#include "header.h"
#include "analyse.h"


extern u_int maxCacheSize;
#define maxHVEvents 2048
/**
  *@author Ralf Plag
  */









/*           !! Important note !!

To optimize the performance of the EventDisplay (and other software), the
class UnPackX caches data. If a programm requests data, UnPackX returns a
pointer to the memory in the cache. This has several consequences:
- The requesting application is _not_ allowed to delete the data (e.g. delete pulseList)
  Deletion is done when the cache is full or a new event is selected.
- The requesting application cannot save the pointer for later requests. The pointer
  may be invalid after the next call of getPulses()
- The requesting application is not allowed to change the data without copying before.
- The main goal of this technique is probably not to reduce file access (the file is
  also cached by the operating system) but to avoid processing the data each time.
  Processing means in the acqiris case only to build a list with pointers to the
  data but in the ETEP case there is much more effort needed !!!
*/


typedef vector< unsigned char* > PulseList;  // list of pointers pointing to the beginning of the pulses in one event

class Pulses
{
public:
  unsigned char* data;
  u_int stream_crate_mod_ch;
  u_int dataSize;
  u_int date;
  u_int time;
  bool isCorrupt;
  PulseList pulseList;
  Pulses(u_int StCrMoCh) {stream_crate_mod_ch=StCrMoCh;dataSize=0;data=NULL;isCorrupt=false;};
  ~Pulses() {if (data) delete [] data;};
  u_int getOffset(u_int idx) {return  ntohl(*(u_int*)pulseList[idx]);};
  u_int getLength(u_int idx) {return  ntohl(*(u_int*)(pulseList[idx]+4));};
  unsigned char* getData(u_int idx) {return pulseList[idx]+8;};
};

typedef vector< Pulses* > PulseCache;

class UnPackX {
	f_i *dataFile;					// file input
  _BOS_RCTR* runHeader;
  _BOS_MODH* moduleHeader;
  _BOS_INDX* indexHeader;
  _BOS_EVEH* eventHeader;
  _BOS_EVEH* firstEventHeader;
  _BOS_INFO* infoHeader;
  _BOS_HIVO* hivoHeader;
  _BOS_BEAM* beamHeader;
  u_int* currentIndexStruct;    // list of u_int containing the indexpositions for one event. (one value for each stream)
  vector <u_int*> indexList;    // list of indexStructs, one for each event.
  PulseCache pulseCache;        // this caches the signal data for one event to minimize HD access during painting.
  u_int cacheSize;              // size of the pulsecache
  u_int hvEvent[maxHVEvents];
  u_int hvFilePos[maxHVEvents];
  int nbrOfHVEvents;
  string fileName;              // :-)
  bool bad;
  bool isFinished;

  f_i* openDataStream(u_int runNumber,u_int runExtNumber,u_int stream);
  f_i* openFZKDataStream(u_int runNumber,u_int runExtNumber,u_int stream);
  void cacheAdd(Pulses*);
  void cacheClear();
public:
  u_int firstEvent,lastEvent,currentEvent;

  u_int pulsesInPSCycle[14];
  double protonsInPSCycle[14];
  double totalNumberOfProtons;
  int nbrOfEventsWithProtonIntensity;
  int nbrOfEventsWithoutProtonIntensity;

  UnPackX() {};
  UnPackX(string fileName,bool debug,void (*callback)(int a,void *b)=NULL,int wantedValues=0) ;
	~UnPackX();

  u_int getNumberOfEvents();
  _BOS_MODH* getModh() {if (moduleHeader==NULL) {
  				printf("ModuleHeader=NULL! -> Exit\n");
				exit(1);
				};
			return moduleHeader;
			};
  _BOS_RCTR* getRctr() {return runHeader;};
  _BOS_EVEH* getLastEveh() {return eventHeader;};
  string getFileName() {return fileName;};
  char* getDescription() {return infoHeader->getData();};
  Pulses* getPulses(u_int stream_crate_mod_ch);
  int getHVEventList(u_int* list,int listSize);
  _BOS_HIVO* getHivo(int &aEvent) throw (std::bad_alloc);
  bool isBad() {return bad;};

  void setCurrentEvent(u_int eventNumber);  // eventNumber as displayed by runControl (firstEvent is subtracted to obtain the array index.
  void fillHisto(HistoCreateAttr* hcr);
  void analyseStream(HistoCreateAttr* hcr);
  u_int getFileSize(u_int aStream);
};





#endif
