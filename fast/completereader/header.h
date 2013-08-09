/***************************************************************************
                          header.h  -  description
                             -------------------
    begin                : Wed Oct 4 2000
    copyright            : (C) 2000-2002 by Ralf Plag & Erich Schaefer
    email                : Ralf.Plag@cern.ch
 ***************************************************************************/

#ifndef HEADER_H
#define HEADER_H
#include <fstream>
#include <string>
#include <vector>
#include <time.h>
#include <cstring>

#include <netinet/in.h>

//#include <stream.h>
#include "fio.h"
#include "DaqType.h"
#include "misc.h"

#define MAXNumberOfStreams 20

//  65
//   A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  3
//  41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f 50 51 52 53 54 55 56 57 58 59 5a 33

#define Rctr 0x52544352
#define Modh 0x48444f4d
#define Eveh 0x48455645
#define Acqc 0x43514341
#define Beam 0x4d414542
#define Indx 0x58444e49
#define Info 0x4f464e49
#define Slow 0x574f4c53
#define Hivo 0x4f564948
// BEAM
#define beamTOF     0x20464f54            //  ' 'F O T
#define beamEASTC   0x54534145            //   T S A E
#define beamEASTA   0x41534145            //   A S A E
#define beamINVD    0x44564e49            //   D V N I  (unknown error)
#define beamNEGATIV 0x5654474e            //   V T G N  (beam intensity is negativ)
#define beamEXPIRED 0x44525058            //   D R P X  (time stamp expired)

// SLOW (special values)
#define slowSAMPLE 0x4c504d53            //   L P M S (current sample in beam)
#define slowBF3a   0x41334642            //   A 3 F B (1. BF3 counter)
#define slowBF3b   0x42334642            //   B 3 F B (2. BF3 counter)
#define slowBF3c   0x43334642            //   C 3 F B (3. BF3 counter)
#define slowFilter 0x52544c46            //   R T L F (filter in beam)

// -- BOS -------------------------------------------------------------------
// -- BOS -------------------------------------------------------------------


class _BOS {
public:
        static const u_int size = sizeof(BOSHEADER)/sizeof(int);
        BOSHEADER *bos;

        _BOS();
        _BOS(u_int *ptr);
        _BOS(u_int *ptr,u_int title, u_int reVumber, u_int length);
        _BOS(u_int title, u_int revNumber,u_int length);
        ~_BOS();
        BOSHEADER *getBOS();
        void setBOS(u_int *ptr);
        char *getTitle( );
        u_int getIntTitle( ) {return bos->title.l;};
        bool cmpTitle(u_int &myTitle) {return (bos->title.l == myTitle);};
        u_int getRevNumber() {return bos->revNumber;};
        u_int getLength() {return bos->length;};
        u_int getFlag() {return bos->reserved;};
        void setFlag(u_int flag) {bos->reserved = flag;};
        bool writeToFile(int file);
        bool readFromFile(f_i* file);
};

// -- RCTR -------------------------------------------------------------------
// -- RCTR -------------------------------------------------------------------



class _BOS_RCTR : public _BOS
{
protected:
        struct tm *systemTime;
        RCTR_HEADER *rctr;
        vector<int> numberOfChannels;

public:
        _BOS_RCTR();
        _BOS_RCTR(u_int runNumber,u_int runExtNumber);
        ~_BOS_RCTR() {if (rctr) delete rctr;};
        u_int getSize();

        RCTR_HEADER *getRCTR() { return rctr; }
        u_int getRunNumber() {return rctr->runNumber; }
        u_int getRunExtNumber() {return rctr->runExtNumber; }
        u_int getRCTRDate() { return rctr->date; }
        u_int getRCTRTime()  { return rctr->time; }

        void nextFile(u_int runNumber,u_int runExt);
        bool writeToFile(int file);
        bool readFromFile(_BOS *aBos,f_i *file);
};


// -- MODH ----------------------------------------------------------------------
// -- MODH ----------------------------------------------------------------------


/*\
 *	Here we define the module specific header
 *	the settings for each module are the same
 * 	over the lifetime of the run
\*/



class _BOS_MODH : public _BOS {

public:

        //	_BOS_MODH() : _BOS() {};
        _BOS_MODH( bool autoCreate = false ); //  : _BOS(ptr,*(u_int *)"MODH",modId,_MODH::size),

        u_int numberOfChannels;
        u_int numberOfStreams;
        Config_V myConf[MAXNumberOfStreams];    		// run configuration: detektor id, card, channel and settings

        Config_V getConfigList(u_int streamNumber);
        Channel_Config getConfigForChannel(u_int channel);
        Channel_Config getConfigForStCrMoCh(u_int stream_crate_mod_ch);
        int indexOf(u_int stream_crate_mod_ch);
        void print();
        bool writeToFile(int file);
        bool readFromFile(_BOS* aBos,f_i *file);
        /*
  Channel_Config getConfig(byte crate,byte module,byte channel);
  Channel_Config getConfig(u_int crate_mod_ch);

        float getOffsetVoltage()  { return (( getLength() > 10 ) ? _MODH::getOffsetVoltage() : 0.0); }
        u_int getStreamNumber(int lch);
        u_int getDetectorId(int lch);
  u_int getPhysModule(int lch);
        u_int getPhysChannel(int lch);
        u_int getDetectorChannel(int lch);
        u_int getFullScale(int lch);
        u_int getPreSample(int lch);
        u_int getPostSample(int lch);
        u_int getThreshold(int lch);
        u_int getSampleRate(int lch);
        u_int getSampleSize(int lch);
  */
        //	void setBOS(u_int *ptr) { _BOS(ptr); }
};

// -- EVEH ----------------------------------------------------------------------
// -- EVEH ----------------------------------------------------------------------
/*
class _EVEH
 {

 struct tm *systemTime;

 public:
 static const u_int size = sizeof(EVENT_HEADER)/sizeof(u_int);	// in longwords
 EVENT_HEADER *eveh;

        _EVEH(u_int *ptr);
        _EVEH(u_int *ptr,u_int exparg,u_int runarg,u_int eventarg);

        void setEventSize(u_int arg);

        u_int getExp();
        u_int getRunNumber();

        u_int getDate();
        u_int getTime();
        u_int getEventNumber();
        u_int getEventSize();
 };
*/
class _BOS_EVEH : public _BOS  {
        struct tm *systemTime;
        EVENT_HEADER *eveh;

public:

        static const u_int size = _BOS::size+sizeof(EVENT_HEADER)/sizeof(int);	// in longwords

        _BOS_EVEH() : _BOS() {eveh=new EVENT_HEADER;};
        _BOS_EVEH(u_int eventNumber,u_int runNumber);
        ~_BOS_EVEH() { if (eveh) delete eveh;};

        void setEventSize(u_int arg) {eveh->sizeOfEvent=arg;};

        u_int getEventSize() { return eveh->sizeOfEvent; }
        u_int getEventNumber() { return eveh->eventNumber; }
        u_int getRunNumber() { return eveh->runNumber; }
        u_int getDate() { return eveh->date; }
        u_int getTime() { return eveh->time; }

        u_int writeToFile(int file);
        bool  readFromFile(_BOS *aBos,f_i *file);
};


// -- ACQC ----------------------------------------------------------------------
// -- ACQC ----------------------------------------------------------------------


class _BOS_ACQC : public _BOS
{
        ACQC_HEADER *acqc;
public:
        _BOS_ACQC();
        _BOS_ACQC(Channel_Config* channel);
        ~_BOS_ACQC() {if (acqc) delete acqc;};
        static const u_int size = _BOS::size+sizeof(ACQC_HEADER)/sizeof(int);	// in longwords

        void setConfigData(Channel_Config* channel);
        void setDataSize(u_int dwords) {bos->length=dwords+sizeof(ACQC_HEADER)/sizeof(int);};

        u_int getStCrMoCh() {return acqc->stream_crate_mod_ch;};
        u_int getDetectorChannel() {return acqc->detectorChannel;};
        bool writeToFile(int file);
        bool readFromFile(_BOS *aBos,f_i *file);
};


// -- INDX ----------------------------------------------------------------------
// -- INDX ----------------------------------------------------------------------


// the index header contains the pointers to the
// start of an event from each stream
// the sequence of the index values is
// in rising order with the streams

class _BOS_INDX : public _BOS
{
        u_int streamPosition[MAXNumberOfStreams];
public:
        _BOS_INDX();
        u_int getSize() {return _BOS::size+bos->length;};
        void  setPosition(u_int streamNumber,u_int position);
        u_int getPosition(u_int streamNumber);
        u_int* getList()  { return &(streamPosition[0]);} ;
        bool writeToFile(int file);
        bool readFromFile(_BOS *aBos,f_i *file);

};

// -- BEAM ----------------------------------------------------------------------
// -- BEAM ----------------------------------------------------------------------
class _BOS_BEAM : public _BOS
{
        float data[2];  // first date, then intensity
        int   type;     // beam coming from EASTC, TOF, reading failed (beamEASTC, beamTOF, beamInvd)
public:
        static const u_int size = _BOS::size+2;	// in longwords
        _BOS_BEAM() : _BOS(*(u_int *)"BEAM",1,3) {type=0;};
        //  void setAttr(float aDate,float aIntensity);
        void setAttr(float aDate,float aIntensity,int type);
        float* getList() {return &(data[0]);};
        int    getType() {return type;};
        bool writeToFile(int file);
        bool readFromFile(_BOS *aBos,f_i *file);

};


struct slow_hv {u_int stream_crate_mod_ch;float voltage;};
struct slow_sv {u_int identifier;float value;};
// -- SLOW ----------------------------------------------------------------------
// -- SLOW ----------------------------------------------------------------------
class _BOS_SLOW : public _BOS
{
        u_int numberOfHVValues;
        u_int numberOfLVValues;
        u_int numberOfSpecialValues;
        slow_hv* hvList;
        vector <slow_sv > svList;
        void setBosLength();
public:
        // 	static const u_int size = _BOS::size;	// in longwords

        _BOS_SLOW();// : _BOS(*(u_int *)"SLOW",0,0) {numberOfHVValues=0;hvList=NULL;};
        ~_BOS_SLOW() {delete hvList;};
        bool writeToFile(int file);
        bool readFromFile(_BOS *aBos,f_i *file);
        // HV values
        void setNumberOfHVValues(u_int number);
        void setHVValue(u_int index,u_int StCrMoCh,float value);
        // LV values
        // special values
        void setSpecialValue(u_int Ident,float val);
        int getNumberOfSpecialValues() { return svList.size();};
        int getSpecialValueName(unsigned int index) {if (index>=svList.size()) return 0;return svList[index].identifier;};
        float getSpecialValue(unsigned int index) {if (index>=svList.size()) return 0;return svList[index].value;};
};

// -- INFO ----------------------------------------------------------------------
// -- INFO ----------------------------------------------------------------------
class _BOS_INFO : public _BOS
{
        char* data;
public:
        static const u_int size = _BOS::size;	// in longwords
        _BOS_INFO() : _BOS(*(u_int *)"INFO",0,0) {data=NULL;};
        ~_BOS_INFO() {delete data;};

        void setData(char* aData,u_int aDataSize);
        char* getData() {return data;};

        bool writeToFile(int file);
        bool readFromFile(_BOS *aBos,f_i *file);
};

// -- HIVO ----------------------------------------------------------------------
// -- HIVO ----------------------------------------------------------------------
class _BOS_HIVO : public _BOS
{
        char* data;
        int dataSize;  // in bytes
public:
        int colCount;
        int rowCount;

        static const u_int size = _BOS::size;	// in longwords
        _BOS_HIVO() : _BOS(*(u_int *)"HIVO",0,0) {data=NULL;};
        ~_BOS_HIVO() {delete data;};

        void setData(char* aData,u_int aDataSize);
        char* getData() {return data;};

        bool writeToFile(int file);
        bool readFromFile(_BOS *aBos,f_i *file);
};


/*\
 *	Here we define the temporary header
 *	used in the unpacking for the event display
\*/
/*
class _TEMP {

 protected:

 TEMP_HEADER *rctr;

 public:
 static const u_int size          = sizeof(TEMP_HEADER)/sizeof(int);

        _TEMP();
        _TEMP(u_int *ptr);
        _TEMP(u_int *ptr,u_int runarg);

        TEMP_HEADER *getRCTR();
        u_int getRunNumber();
        u_int getRCTRDate();
        u_int getRCTRTime();

 };


class _BOS_TEMP : public _BOS, public _TEMP
 {
 public:
 static const u_int size = _BOS::size+_TEMP::size;	// in longwords
        _BOS_TEMP() {}
        _BOS_TEMP(u_int *ptr):_BOS(ptr), _TEMP(ptr+_BOS::size) {}
        _BOS_TEMP(u_int *ptr, u_int runarg) :
        _BOS(ptr,*(u_int *)"TEMP",runarg,_TEMP::size),
        _TEMP(ptr+_BOS::size,runarg){}
 };

  */

#endif


