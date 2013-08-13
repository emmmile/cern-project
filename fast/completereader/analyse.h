/***************************************************************************
                          analyse.h  -  description
                             -------------------
    begin                : Mon Apr 2 2001
    copyright            : (C) 2001 by Erich Schaefer
    email                : Erich.Schafer@cern.ch
 ***************************************************************************/

#ifndef ANALYSE_H
#define ANALYSE_H

#include <fstream>
#include "DaqType.h"

#define u_int unsigned int






class Histo {
public:
  unsigned int* data;
  unsigned int size;
  float minValue,maxValue,deltaValue;
  float minValueLog,maxValueLog,deltaValueLog;
  bool logX;
  ofstream *file;

	Histo();
	~Histo();
  void resize(unsigned int newSize);
  void setRange(float min,float max);
  void clear();
  void add(float);
  unsigned int getSize() {return size;};
  unsigned int getIntegral(int chLeft,int chRight);
};






class HistoCreateAttr;

class HistoCreateAttr
{
public:
bool running;
Histo* histo;               // histogram in which to add the information
void (*analyseFunc)(HistoCreateAttr*);

u_int stream_crate_mod_ch;  // current channel

vector <u_int> selectedChannel;  // list of stream_crate_mod_ch's
vector <Channel_Config*> curChannelList; // list of the selected channels of the current stream
Channel_Config* config;     // current channel configuration
u_int eventNumber;

unsigned char* data;        // data to analyse
u_int dataSize;             // size of data for analysing

int parameter;              // how to analyse
u_int firstExt,lastExt;     // first run externsion , last rext.
u_int analyseStart_ns;         // actual position in the display
float offsetGammaPeak;      // position of gamma flash in the display


float* muster;              // for testing
u_int anzahl;               // for testing

string info1;               // user feedback (printed below the histogram)
string info2;
string info3;
};



//void analyseTOF(HistoCreateAttr* hcr);
//void analyseBaF2(HistoCreateAttr* hcr);

#endif
