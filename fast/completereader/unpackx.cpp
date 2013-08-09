/***************************************************************************
                          unpackx.cpp  -  description
                             -------------------
    begin                : Fri Mar 9 2001
    copyright            : (C) 2001,2002 by Ralf Plag
    email                : Ralf.Plag@cern.ch
 ***************************************************************************/

#include "unpackx.h"
#include "header.h"
#include "DaqDefs.h"
#include "analyse.h"
#include <unistd.h> // for sleep()

extern int g_debug;
u_int maxCacheSize=10000000;

// if the data files are located on different machines, that means, in different
// directories, these are the paths were we can look for files. They
// must end with '/' !
char* pathList[maxNumberOfStreams]={"/tdata/data01/",
                                    "/tdata/data02/",
                                    "/tdata/data03/",
                                    "/tdata/data04/",
                                    "/tdata/data05/",
                                    "/tdata/data06/",
                                    "/tdata/data07/",
                                    "/tdata/data08/",
                                    "/tdata/data09/",
                                    "/opt/data/",
                                    "/opt/data/",
                                    "/tdata2/data01/",
                                    "/tdata2/data02/",
                                    "/tdata2/data03/",
                                    "/tdata2/data04/",
                                    "/tdata2/data05/",
                                    "/tdata2/data06/",
                                    "/tdata2/data07/",
                                    "/tdata2/data08/",
                                    "/tdata2/data09/"
                                   };

// once a file was found, we store the directory here to have faster access next time.
string dataPath[maxNumberOfStreams]={"","","","",
                                     "","","","",
                                     "","","","",
                                     "","","",""};
// define some value types
#define vtEventHeader     1
#define vtBeamHeader      2
#define vtSlowHeader      4
UnPackX::UnPackX(string fName,bool debug,void (*callback)(int valueType,void *value),int wantedValues)
{
        memset(pulsesInPSCycle,0,14*4);
        // init variables
        runHeader=new _BOS_RCTR();
        moduleHeader=new _BOS_MODH();
        indexHeader=new _BOS_INDX();
        eventHeader=new _BOS_EVEH();
        firstEventHeader=new _BOS_EVEH();
        infoHeader=new _BOS_INFO();
        beamHeader=new _BOS_BEAM();
        hivoHeader=new _BOS_HIVO();
        _BOS_SLOW *slowHeader=new _BOS_SLOW();

        u_int* indexItem;
        firstEvent=0xffffffff;
        lastEvent=0xffffffff;
        fileName=fName;
        cacheSize=0;
        nbrOfHVEvents=0;
        totalNumberOfProtons=0.0;
        nbrOfEventsWithProtonIntensity=0;
        nbrOfEventsWithoutProtonIntensity=0;
        memset(pulsesInPSCycle,0,14*4);
        memset(protonsInPSCycle,0,14*sizeof(double));
        double lastBeamDate=0;
        double beamDateOffset=0;   // to have the beam as time since midnight before start (if run crosses midnight)

        // beginn processing
        dataFile = new f_i(fileName);
        if (!dataFile) {
                if (debug) cout << "Cannot open file !"<<endl;
                fileName="";
                bad=true;
                return;
        }
        //u_int fileSize =  dataFile->size();			// in byte
        _BOS bosHeader;
        bool error=false;
        bad=false;

        while ((!error) && (bosHeader.readFromFile(dataFile))) {
                //if (debug) cout << "Found header " << bosHeader.getTitle() << endl;
                switch (bosHeader.getIntTitle()) {

                case Rctr :
                        if (!runHeader->readFromFile(&bosHeader,dataFile)) {
                                cout << "Error reading runHeader"<<endl;
                                error=true;
                        }
                        break;

                case Modh :
                        if (!moduleHeader->readFromFile(&bosHeader,dataFile)) {
                                cout << "Error reading moduleHeader"<<endl;
                                error=true;
                        }
                        else
                                if (debug) moduleHeader->print();
                        break;

                case Eveh :
                        if (!eventHeader->readFromFile(&bosHeader,dataFile)) {
                                cout << "Error reading eventHeader"<<endl;
                                error=true;
                        }
                        else {
                                if (wantedValues&vtEventHeader) callback(vtEventHeader,eventHeader);
                                if ((debug) && ((eventHeader->getEventNumber()%10)==0)) cout << "Eventnumber " << eventHeader->getEventNumber() << endl;
                                indexItem=new u_int[2+moduleHeader->numberOfStreams];
                                indexList.push_back(indexItem);
                                lastEvent=eventHeader->getEventNumber();
                                if (firstEvent==0xffffffff) {
                                        firstEvent=lastEvent;
                                        firstEventHeader=eventHeader;
                                }
                        }
                        break;

                case Indx :
                        if (!indexHeader->readFromFile(&bosHeader,dataFile)) {
                                cout << "Error reading indexHeader"<<endl;
                                error=true;
                                lastEvent--;
                        }
                        else {
                                memcpy(&indexItem[2],indexHeader->getList(),moduleHeader->numberOfStreams*4);
                                //cout << "Index stream 0 = " << indexItem[2] << endl;
                        }
                        break;

                case Beam :
                        if (!beamHeader->readFromFile(&bosHeader,dataFile)) {
                                cout << "Error reading beamHeader"<<endl;
                                error=true;
                                lastEvent--;
                        }
                        else {
                                memcpy(indexItem,beamHeader->getList(),8);
                                if ((beamHeader->getList()[0]!=0.0) && (beamHeader->getList()[0]<lastBeamDate)) {
                                        beamDateOffset+=60.0*60.0*24.0;  // another nice day just started...
                                        if (debug) printf("Midnight crossing, offset now %f\n",beamDateOffset);
                                }
                                double beamDate=double(beamHeader->getList()[0])+beamDateOffset;
                                u_int cycle=(u_int)((beamDate - 16.8*int(beamDate/16.8))/1.2);
                                if ((cycle<14) && (beamHeader->getList()[1]>0)) {
                                        pulsesInPSCycle[cycle]++;
                                        protonsInPSCycle[cycle]+=beamHeader->getList()[1];
                                }
                                if (beamHeader->getList()[0]!=0.0) lastBeamDate=beamHeader->getList()[0];
                                if ((debug) && (cycle>=14)) printf("Wrong cycle: %d !\n",cycle);
                                if ((debug) && ((eventHeader->getEventNumber()%10)==0)) printf("Time: %16f Intensity %3.2e\n",beamDate,beamHeader->getList()[1]);
                                if ((beamHeader->getList()[1]>1E10) && (beamHeader->getList()[1]<1E13)) {
                                        totalNumberOfProtons+=beamHeader->getList()[1];
                                        nbrOfEventsWithProtonIntensity++;
                                }
                                else
                                        nbrOfEventsWithoutProtonIntensity++;
                                if (wantedValues&vtBeamHeader) callback(vtBeamHeader,beamHeader);
                        }
                        break;

                case Info :
                        if (debug) cout << "reading info" << endl;
                        if (!infoHeader->readFromFile(&bosHeader,dataFile)) {
                                cout << "Error reading infoHeader"<<endl;
                                error=true;
                        }
                        break;

                case Slow :
                        if (debug) cout << "reading Slow" << endl;
                        if (wantedValues&vtSlowHeader) {
                                if (!slowHeader->readFromFile(&bosHeader,dataFile)) {
                                        cout << "Error reading slowHeader"<<endl;
                                        error=true;
                                }
                                else
                                        callback(vtSlowHeader,slowHeader);
                        } // if wanted
                        else
                                dataFile->step(bosHeader.getLength()*4);break;
                        break;
                case Hivo :
                        if (nbrOfHVEvents<maxHVEvents) {
                                hvEvent[nbrOfHVEvents]=eventHeader->getEventNumber();
                                hvFilePos[nbrOfHVEvents]=dataFile->fPos();
                                nbrOfHVEvents++;
                        };
                        dataFile->step(bosHeader.getLength()*4);break;
                default :
                        printf("unknown header after event %d filePos: %d. Skipping.\n",lastEvent,(int)dataFile->fPos());
                        char* name=bosHeader.getTitle();
                        printf("Name: '%s' rev: %d res: %d size %d\n",name,bosHeader.bos->revNumber,bosHeader.bos->reserved,bosHeader.getLength()*4);
                        for (int i=0;i<4;i++) if (name[i]<32) error=true;
                        if (bosHeader.bos->reserved!=0) error=true; // stopIt!
                        dataFile->step(bosHeader.getLength()*4);break;
                } // switch

        } // while
        if (firstEvent==0xffffffff) bad=true;
        setCurrentEvent(firstEvent);
        /* -> now in EventDisplay
printf("\n---Beam info--------------------------------------\n");
printf("Total number of protons:    %3.2e\n",totalNumberOfProtons);
printf("Events with p-intensity:    %d\n",nbrOfEventsWithProtonIntensity);
printf("Events without p-intensity: %d\n",nbrOfEventsWithoutProtonIntensity);
for (u_int i=0;i<14;i++)
  printf("%6d pulses in the %2d. cycle\n",pulsesInPSCycle[i],i+1);
printf("--------------------------------------------------\n\n");
*/
        delete slowHeader;
}

UnPackX::~UnPackX(){
        cacheClear();
        dataFile->close();
        delete dataFile;     dataFile=NULL;
        delete runHeader;    runHeader=NULL;
        delete moduleHeader; moduleHeader=NULL;
        delete eventHeader;  eventHeader=NULL;
        delete infoHeader;   infoHeader=NULL;
        delete indexHeader;  indexHeader=NULL;
        delete beamHeader;   beamHeader=NULL;
        delete hivoHeader;   hivoHeader=NULL;
}
// -------------------------------------------------------------------------------------------
u_int UnPackX::getNumberOfEvents()
{
        return indexList.size();
}
// -------------------------------------------------------------------------------------------
void UnPackX::setCurrentEvent(u_int eventNumber)
{
        if (eventNumber==0xffffffff) return;
        cacheClear();
        if ((eventNumber>=firstEvent)  && (eventNumber<=lastEvent)) {
                currentEvent=eventNumber;
                currentIndexStruct=indexList[eventNumber-firstEvent];
                return;
        }
        cout << "UnPackX::setCurrentEvent: Corrupt event number: "<< eventNumber << endl;
}
// -------------------------------------------------------------------------------------------
u_int UnPackX::getFileSize(u_int aStream)
{
        if (aStream==0) { // the index stream itself!
                FILE* dummy=fopen(fileName.c_str(),"rb");
                fseek(dummy,0,SEEK_END);
                u_int fsize=ftell(dummy);
                fclose(dummy);
                return fsize;
        };
        if (aStream>moduleHeader->numberOfStreams) return 0;
        return indexList[lastEvent-firstEvent][aStream+1]; // aStream=1 for the first data stream= [2]
}
// -------------------------------------------------------------------------------------------
int UnPackX::getHVEventList(u_int* list,int listSize)
{
        if (listSize>nbrOfHVEvents) listSize=nbrOfHVEvents;
        memcpy(list,hvEvent,4*listSize);
        return listSize;
};
// -------------------------------------------------------------------------------------------
_BOS_HIVO* UnPackX::getHivo(int &aEvent) throw (std::bad_alloc)
{
        int eventIndex=0;
        while ((eventIndex<nbrOfHVEvents) && (aEvent>(int)hvEvent[eventIndex])) eventIndex++;
        dataFile->resetFilePos();
        dataFile->goToPos(hvFilePos[eventIndex]);
        _BOS bosHeader;
        if (!hivoHeader->readFromFile(&bosHeader,dataFile)) {
                cout << "Error reading hivoHeader"<<endl;
                return NULL;
        }
        aEvent=hvEvent[eventIndex];
        return hivoHeader;
}
// -------------------------------------------------------------------------------------------
/* This function should return the filename of stream with number 'stream'.
   The name structure is the following:
   - the ending may be .raw.finished or just .raw
   - the middle is runXXX_X_sX  (runNumber, runExtNumber, streamNumber)
   - the path may be same as that of the index file, but it is also possible
     that the data files lie on other machines than the index file. In this case
     the other machines are mounted and we have to search these directories for
     the desired file. For this purpose, we have the pathList, which tells us were
     to look for the file.
     If the file was found, we know which streamNumber corresponds to which
     directory and we save this directory as dataPath[streamNumber] to have a
     faster access for the next time.
*/
f_i* UnPackX::openDataStream(u_int runNumber,u_int runExtNumber,u_int stream)
{
        if (stream>=maxNumberOfStreams) {
                cout << "getStreamName: stream number too big ! Trying stream 1." << endl;
                stream=1;
        }
        // check if the file has the ending that indicates that the file is finished. Will be added to the dataFileName.
        string finished=fileFinished;

        // construct filename
        char buffer[1024];
        sprintf(buffer,"run%d_%d_s%d.raw",runNumber,runExtNumber,stream);
        string name=buffer;

        string dataStream;
        // perhaps we have already stored the path in dataPath -------------------------
        if (dataPath[stream]!="") {
                isFinished=false;
                dataStream=string(dataPath[stream])+name;
                f_i *file = new f_i(dataStream);
                if (file->isBad())
                        delete file;
                else
                        return file;
                isFinished=true;
                dataStream=string(dataPath[stream])+name+finished;
                file = new f_i(dataStream);
                if (file->isBad())
                        delete file;
                else
                        return file;
        }
        // if not, we try the same path as the index file ------------------------------
        strcpy(buffer,fileName.c_str());
        // remove the fileNAME from the original path
        char* lastDelimiter=strrchr(buffer,'/');
        if (lastDelimiter) lastDelimiter++;
        else
                lastDelimiter=buffer;
        *lastDelimiter=0;

        isFinished=false;
        dataStream=string(buffer)+name;
        f_i *file = new f_i(dataStream);
        if (file->isBad())
                delete file;
        else {
                dataPath[stream]=buffer;  // save it for the next time
                return file;
        }
        isFinished=true;
        dataStream=string(buffer)+name+finished;
        file = new f_i(dataStream);
        if (file->isBad())
                delete file;
        else {
                dataPath[stream]=buffer;  // save it for the next time
                return file;
        }
        // last possibility: we have a list of paths where we can look for the file ---
        for (u_int i=0;i<maxNumberOfStreams;i++) {

                isFinished=false;
                dataStream=string(pathList[i])+name;
                file = new f_i(dataStream);

                if (file->isBad())
                        delete file;
                else {
                        dataPath[stream]=pathList[i];  // save it for the next time
                        return file;
                }

                isFinished=true;
                dataStream=string(pathList[i])+name+finished;

                file = new f_i(dataStream);
                if (file->isBad())
                        delete file;
                else {
                        dataPath[stream]=pathList[i];  // save it for the next time
                        return file;
                }
        }
        cout << "openDataStream failed: Cannot find file " << name .c_str() << endl;
        return NULL;
}


f_i* UnPackX::openFZKDataStream(u_int runNumber,u_int runExtNumber,u_int stream)
{
        if (stream>=maxNumberOfStreams) {
                cout << "getStreamName: stream number too big ! Trying stream 1." << endl;
                stream=1;
        }
        // check if the file has the ending that indicates that the file is finished. Will be added to the dataFileName.
        string finished=fileFinished;

        // extract filename base
        char buffer[1024];
        strcpy(buffer,fileName.c_str());
        char* ptr=strstr(buffer,".idx");
        if (!ptr) {
                printf("No '.idx' found.\n");
                return NULL;
        };
        // try to open .raw.finished-Version
        sprintf(ptr,"_s%d.raw.finished",stream);
        f_i *file = new f_i(buffer);
        if (file->isBad())
                delete file;
        else
                return file;
        // try to open .raw-Version
        sprintf(ptr,"_s%d.raw",stream);
        file = new f_i(buffer);
        if (file->isBad())
                delete file;
        else
                return file;

        cout << "openDataStream failed: Cannot find file " << buffer << endl;
        return NULL;
}

/* This is the most important function !
   The goal of UnPack is of course to get the signal data stored in the files
   which is done here. The event is selected via setCurrentEvent (see above).
   To get data, the user must only specify the channel which is done via the
   stream_crate_module_channel identifier. This function then creates a class
   of type Pulses which contains all information. The steps are:
   - Determine the size of the raw data to allocate memory
   - read raw data from file into this memory
   - process raw data to build a list with pointers to the start of the pulses
   
   The memory allocated for the Pulses class belongs to UnPackX and must not
   be freed by the user. Freeing memory is done when a new 'Pulses' is stored
   in the cache and the maxCacheSize is not sufficient.
   
   Hence: Please be aware the the liftime of the pulses pointer is relatively short!
          Don't store pulse pointers for a later access!
*/
Pulses* UnPackX::getPulses(u_int stream_crate_mod_ch)
{
        // first try cache. This can be optimized by setting the last used pulse
        // at the first position in the list.
        for (u_int i=0;i<pulseCache.size();i++) {
                if (pulseCache[i]->stream_crate_mod_ch==stream_crate_mod_ch) {
                        //      printf("UnPackX: data for %x found in cache.\n",stream_crate_mod_ch);
                        return pulseCache[i];
                }
        }
        //printf("UnPackX: data for %x not cached. Reading from file.\n",stream_crate_mod_ch);
        Pulses* pulses=new Pulses(stream_crate_mod_ch);

        // open the data stream
        u_int stream=(stream_crate_mod_ch&0xff000000) >> 24;
        f_i *file = openDataStream(runHeader->getRunNumber(),runHeader->getRunExtNumber(),stream);
        if (!file) {
                file = openFZKDataStream(runHeader->getRunNumber(),runHeader->getRunExtNumber(),stream);
                if (!file) {
                        pulses->isCorrupt=true;
                        return pulses;
                }
        }

        file->goToPos(currentIndexStruct[1+stream]); // element 0 & 1 reserved for beam, but stream begins with 1 ! => stream+2-1
        _BOS bos;
        bos.readFromFile(file);
        _BOS_EVEH eveh;
        eveh.readFromFile(&bos,file);
        pulses->date=eveh.getDate();
        pulses->time=eveh.getTime();

        if (g_debug) cout << "Eventnumber: " << eveh.getEventNumber() << endl;
        // go to start of data, this means look for ACQC section with stream_crate_mod_ch
        _BOS_ACQC acqc;
        for (;;)  {
                if (!bos.readFromFile(file)) {
                        cout << "getPulses: Cannot read bos header !"<<endl;
                        file->close();
                        pulses->isCorrupt=true;
                        return pulses;
                }
                if (bos.getIntTitle()!=Acqc) {       // serious error !
                        cout << "getPulses: ACQC bank expected but not found !"<<endl;
                        file->close();
                        pulses->isCorrupt=true;
                        return pulses;
                }
                if (!acqc.readFromFile(&bos,file)) {         // read Type, Det Channel, Str_crate_mod_ch
                        cout << "getPulses: Cannot read ACQC header !"<<endl;
                        file->close();
                        pulses->isCorrupt=true;
                        return pulses;
                };
                if (acqc.getStCrMoCh()==stream_crate_mod_ch) break;
                file->step(bos.getLength()*4-sizeof(ACQC_HEADER));
        }
        // Now we are at the right position in the file
        if (g_debug) printf("GetPulses, file pos: %u\n",(int)file->fPos());
        if (bos.getLength()>sizeof(ACQC_HEADER)/4)  {
                u_int dataSize=bos.getLength()*4-sizeof(ACQC_HEADER);  // in bytes
                pulses->data=new unsigned char[dataSize];
                pulses->dataSize=dataSize;
                if (file->read(pulses->data,dataSize)!=dataSize) {
                        cout << "getPulses: Error reading file !" << endl;
                        pulses->isCorrupt=true;
                }
                file->close();
                // build vector of pulses
                u_int Index=0; // in bytes
                while (Index<dataSize-4) {
                        //    printf("Index: %d length: %d\n",Index,ntohl(*(u_int*)&(pulses->data[Index+4])));
                        pulses->pulseList.push_back(&(pulses->data[Index])); // save position in data
                        if (g_debug) printf("offset: %u index: %d length: %d\n",ntohl(*(u_int*)&(pulses->data[Index])),Index,ntohl(*(u_int*)&(pulses->data[Index+4])));
                        Index+=ntohl(*(u_int*)&(pulses->data[Index+4]))+8;      // read the length here !
                } // while
        }
        if (g_debug) cout << "Leaving getPulses" << endl;
        delete file;file=NULL;
        cacheAdd(pulses);
        return pulses;
}



void UnPackX::cacheAdd(Pulses* pulses)
{
        while ((cacheSize+pulses->dataSize>maxCacheSize) && (pulseCache.size()>0)) {
                cacheSize-=pulseCache[0]->dataSize;
                delete pulseCache[0];
                //   pulseCache.erase(&(pulseCache[0]));
                pulseCache.erase(pulseCache.begin());
        }
        cacheSize+=pulses->dataSize;
        pulseCache.push_back(pulses);
}



void UnPackX::cacheClear()
{
        for (u_int i=0;i<pulseCache.size();i++) delete pulseCache[i];
        pulseCache.clear();
}


void UnPackX::fillHisto(HistoCreateAttr* hcr)
{
        for (u_int s=0;s<moduleHeader->numberOfStreams;s++) {
                hcr->curChannelList.clear();
                // put all selected channels of stream s in curChannelList
                for (u_int c=0;c<moduleHeader->myConf[s].size();c++) {
                        for (u_int sel=0;sel<hcr->selectedChannel.size();sel++) {
                                if (hcr->selectedChannel[sel]==moduleHeader->myConf[s][c].str_crate_mod_ch)
                                        hcr->curChannelList.push_back(&moduleHeader->myConf[s][c]);
                        }
                }
                // then go through the stream
                if (hcr->curChannelList.size()>0) analyseStream(hcr);
        } // for s
}

void UnPackX::analyseStream(HistoCreateAttr* hcr)
{
        hcr->config=NULL;
        u_int stream=(hcr->curChannelList[0]->str_crate_mod_ch&0xff000000) >> 24;

        _BOS bosHeader;

        _BOS_RCTR* curRunHeader=new _BOS_RCTR();
        _BOS_MODH* curModuleHeader=new _BOS_MODH();
        _BOS_EVEH* curEventHeader=new _BOS_EVEH();
        _BOS_ACQC* curAcqcHeader=new _BOS_ACQC();
        hcr->data=NULL;
        u_int physSize=0;
        u_int pos;
        char dummy[512];

        for (unsigned int ext=hcr->firstExt;ext<=hcr->lastExt;ext++) {
                sprintf(dummy,"Processing file run%d_%d_s%d\n",runHeader->getRunNumber(),ext,stream);
                hcr->info3=dummy;
                f_i *file = openDataStream(runHeader->getRunNumber(),ext,stream);
                if ((!file) || (file->isBad())) {
                        cout << "cannot open file." <<endl;
                        delete file;file=NULL;
                        break;
                }
                //u_int fileSize =  file->size();			// in byte

                bool error=false;
                while (!error) {

                        // if the (current) end of the file is reached, this part of codes waits for the
                        // file being written further.
                        pos=file->fPos();
                        while ((!(bosHeader.readFromFile(file))) && (hcr->running)) {
                                cout << "Error reading bosHeader at " << pos << ". Waiting for retry..."<<endl;
                                file->close();
                                delete file;file=NULL;
                                if (isFinished) {
                                        if (g_debug) cout << "Finished file completely processed. Leaving." << endl;
                                        error=true;
                                        break;
                                };
                                for (u_int i=0;i<30;i++) {
                                        if (!hcr->running) {error=true;break;} else sleep(1);
                                }
                                file = openDataStream(runHeader->getRunNumber(),ext,stream);
                                if (file->isBad()) {
                                        cout << "cannot reopen file." <<endl;
                                        delete file;file=NULL;
                                        break;
                                }
                                file->goToPos(pos);
                        }
                        if ((!hcr->running) || (error)) break;  // lets quit


                        switch (bosHeader.getIntTitle()) {

                        case Rctr :
                                if (!curRunHeader->readFromFile(&bosHeader,file)) {
                                        cout << "Error reading runHeader"<<endl;
                                        error=true;
                                }
                                break;

                        case Modh :
                                if (!curModuleHeader->readFromFile(&bosHeader,file)) {
                                        cout << "Error reading moduleHeader"<<endl;
                                        error=true;
                                }
                                break;

                        case Eveh :
                                if (g_debug) cout << "FilePos: " << pos << endl;
                                pos=file->fPos();
                                while ((!curEventHeader->readFromFile(&bosHeader,file)) && (hcr->running)) {
                                        if (g_debug) cout << "Error reading eventHeader. Waiting for retry..."<<endl;
                                        sleep(2);
                                        file->goToPos(pos);
                                }
                                if (!hcr->running) error=true;  // lets quit
                                hcr->eventNumber=curEventHeader->getEventNumber();
                                break;

                        case Acqc :
                                pos=file->fPos();
                                while ((!curAcqcHeader->readFromFile(&bosHeader,file)) && (hcr->running)) {
                                        if (g_debug) cout << "Error reading acqcHeader. Waiting for retry..."<<endl;
                                        sleep(2);
                                        file->goToPos(pos);
                                }
                        {
                                bool doAnalyse=false;
                                if (bosHeader.getLength()>sizeof(ACQC_HEADER)/4) {
                                        for (u_int i=0;i<hcr->curChannelList.size();i++) {
                                                if (hcr->curChannelList[i]->str_crate_mod_ch==curAcqcHeader->getStCrMoCh()) {
                                                        doAnalyse=true;
                                                        hcr->config=hcr->curChannelList[i];
                                                        break;
                                                } // if stream_crate
                                        } // for i
                                        if (doAnalyse) {
                                                hcr->dataSize=bosHeader.getLength()*4-sizeof(ACQC_HEADER);  // in bytes
                                                if (hcr->dataSize>physSize) {
                                                        delete hcr->data;
                                                        physSize=hcr->dataSize;
                                                        hcr->data=new unsigned char[physSize];
                                                }
                                                if (file->read(hcr->data,hcr->dataSize)!=hcr->dataSize)
                                                        cout << "getPulses: Error reading file !" << endl;
                                                else
                                                        hcr->analyseFunc(hcr);
                                        }
                                        else
                                                file->step(bosHeader.getLength()*4-sizeof(ACQC_HEADER));
                                } // if bos
                        }
                                break;



                        default :
                                printf("unknown header. Skipping.\n");
                                file->step(bosHeader.getLength()*4);break;
                        } // switch

                } // while



                if (file) {file->close(); delete file;}
        } // for
        delete hcr->data;
        if (g_debug) cout << "Leaving histogram routine" << endl;
}




