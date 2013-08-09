/***************************************************************************
                          unpack.cpp  -  description
                             -------------------
    begin                : Sun Feb 11 2001
    copyright            : (C) 2001 by Erich Schaefer
    email                : Erich.Schaefer@cern.ch
 ***************************************************************************/
/***************************************************************************
                          unpack.cpp  -  description
                             -------------------
    begin                : Sat Feb 10 2001
    copyright            : (C) 2001 by Erich Schaefer
    email                : Erich.Schaefer@cern.ch
 ***************************************************************************/
#include "unpack.h"



unPack::unPack(string filename,bool dbgout = false) {

        dataFile = new f_i(filename);

        /*\
                 * 	first check full size
                \*/

        fileSize =  dataFile->size();			// in byte

        /*\
                 * 	read the RCTR header
                \*/

        myChannel = 0;
        first = true;

        //		emod = new _BOS_EMOD();
        //		modh = new _BOS_MODH();
        rctr = new _BOS_RCTR();

        fileType=-1;
        //eveh = new _BOS_EVEH();	// eveh could be taken out ???? , is set in scanAllBanks
        eveh  = (_BOS_EVEH *)0;
        dbg = dbgout;
}

bool unPack::getBosHeader(BOSHEADER *bank) {
        u_int size = sizeof(BOSHEADER);

        if( size != dataFile->read((char *)bank,size)) {
                cout << "couldn't read BOS header " << endl;
                return false;
        }
        return true;
}

bool unPack::getRCTRHeader(RCTR_HEADER *bank) {
        u_int size = sizeof(RCTR_HEADER);

        if( size != dataFile->read((char *)bank,size)) {
                cout << "couldn't read RCTR_HEADER header " << endl;
                return false;
        }
        return true;
}

bool unPack::getModhHeader(MOD_HEADER *bank,u_int size) {
        //		u_int size = sizeof(MOD_HEADER);

        if( size*4 != dataFile->read((char *)bank,size*4)) {
                cout << "couldn't read MOD_HEADER header " << endl;
                return false;
        }
        return true;
}

bool unPack::getEmodHeader(u_int size) {
        /*
  Channel_Config channelConfig;
  Config_V configList;
  u_int numberOfChannels=size/sizeof(Channel_Config);
  for (unsigned i=0;i<numberOfChannels;i++) {
    if( sizeof(Channel_Config) != dataFile->read((char*)&channelConfig,sizeof(Channel_Config))) {
                                cout << "couldn't read EMOD_HEADER header " << endl;
                                return false;
                                } // if
    configList.push_back(channelConfig);
    } // for
  emod->setMODH(configList);
  return true;
*/
}

/*\
 *	goes throught the data file and checks if there are valid headers
 *	content of RCTR and MODH are stored in the local class
 *	each EVEH file pos is stored in the vector myEventPos_v
\*/

bool unPack::processFile() {	
        /*
                u_int bankSize;
                _BOS *bos = new _BOS();
                u_int totalSize = 0;
                int curChannel=0;
    do {

                std::streampos readPos = dataFile->fPos();
                if( !getBosHeader( bos->getBOS() ) )
                        exit(0);


                //cout << form("bos title %x size 0x%x\n",bos->getIntTitle(),bos->getLength()) << flush;
                switch( bos->getIntTitle() ) {
                        case Rctr:
                                getRCTRHeader(rctr->getRCTR() );

                                bankSize =  (bos->getLength())*sizeof(int);
                                totalSize += bos->getLength()+_BOS::size;
                                //dataFile->step(bankSize);
                                break;

                        case Emod:
        fileType=1;
                                emod->setBOS((u_int *)bos);
                                getEmodHeader(bos->getLength() );
                                cout << form("bos->getLength() %d ",bos->getLength());
                                bankSize =  (bos->getLength())*sizeof(int);
                                totalSize += bos->getLength()+_BOS::size;
                                break;

                        case Modh:
        fileType=0;
                                modh->setBOS((u_int *)bos);
                                getModhHeader(modh->getMODH(),bos->getLength() );
                                cout << form("bos->getLength() %d ",bos->getLength());
                                //bos_modh(bos,modh);	// fill the complete bos mod header
                                bankSize =  (bos->getLength())*sizeof(int);
                                totalSize += bos->getLength()+_BOS::size;
                                //dataFile->step(bankSize);
                                break;

                        case Eveh:
        curChannel=0;
                                bankSize =  (bos->getLength())*sizeof(int);
                                totalSize += bos->getLength()+_BOS::size;
                                dataFile->step(bankSize);

                                // create a new bank vector

                                if( !first ) {
                                        // start of a new channel
                                        myEventPos_v.headerPOS_v = headerPos_v;// copy temporary header list here (all channels of a single run)
                                        runEventPos_v.push_back(myEventPos_v); // and add this description to over-all-list (all events of a run)

                                        }
                                else
                                        first = false;

                                headerPos_v.clear(); // clear temporary list
                                myEventPos_v.evehPos = readPos;
                                //cout << form("Eveh pos %x \n",readPos );
                                break;

                        case Acqc:
                                headerPos_v.push_back(readPos); // add to temporary list

                                bankSize =  (bos->getLength())*sizeof(int);
                                totalSize += bos->getLength()+_BOS::size;
                                dataFile->step(bankSize);

                                break;

                        default: // generic Header
        if ((fileType==0) ||
            (bos->getIntTitle()!=emod->getDetectorId(curChannel))) {
           cout << "unknown GEN header: '" << bos->getTitle() << "' Processing stopped !" << endl;
           return false;
           }


                                headerPos_v.push_back(readPos);
                                bankSize =  (bos->getLength())*sizeof(int);
                                totalSize += bos->getLength()+_BOS::size;
                                dataFile->step(bankSize);


        curChannel++;
                                //cout << form("unknown BANK found %s \n",bos->getTitle());
                        }
                        //cout << form("totalSize %x fileSize/4 %x \n",totalSize,fileSize/4);
                } while(  totalSize < fileSize/4);

                if( totalSize == fileSize/4 ) {			// the last event is valid
                        myEventPos_v.headerPOS_v = headerPos_v;
                        runEventPos_v.push_back(myEventPos_v);
                        }
*/					
        return true;
}

/*\
 *	some member functions to run and module settings
\*/

void unPack::printRun() {
        cout << runEventPos_v.size() << "events in run " << endl;

        for (u_int i=0;i<runEventPos_v.size();i++) {
                cout << form("data at file pos 0x%x \n",runEventPos_v[i].evehPos);
        }
}

void unPack::printModh() {
        /*
                        cout << form("getSampleRate %d \n",modh->getSampleRate());
                        cout << form("getSampleSize %d \n",modh->getSampleSize());
                        cout << form("getFullScale %d \n",modh->getFullScale());
                        cout << form("getDelayTime %f\n",modh->getDelayTime());
                        cout << form("getThreshold %d \n",modh->getThreshold());
                        cout << form("getThresSign %d \n",modh->getThresSign());
                        cout << form("getPreSample %d \n",modh->getPreSample());
                        cout << form("getPostSample %d \n",modh->getPostSample());
                        cout << form("getInputState %d \n",modh->getInputState());
*/	
}

void unPack::printEveh(u_int event) {
        cout << " ----- " << event << endl;
        cout << " ----- " << runEventPos_v[event].headerPOS_v.size() << endl;

        for (u_int i=0;i<runEventPos_v[event].headerPOS_v.size();i++) {
                cout << form("es data at file pos 0x%x \n",runEventPos_v[event].headerPOS_v[i]);
        }
}

u_int unPack::numberOfEvents() { return runEventPos_v.size(); }

u_int unPack::getPreSample(int lch)
{ //return (fileType) ? emod->getPreSample(lch) : modh->getPreSample();
}

u_int unPack::getPostSample(int lch)
{ //return (fileType) ? emod->getPostSample(lch) : modh->getPostSample();
}

u_int unPack::getThreshold(int lch)
{// return (fileType) ? emod->getThreshold(lch) : modh->getThreshold();
}

float unPack::getNanoSecPerSample(int lch)
{
        /*
// emod gives us the Samplerate in MB/s
if (fileType) {
  int Rate=emod->getSampleRate(lch);
  return (Rate) ? 1000.0/Rate : 1.0;
  }
// modh gives us just an index
float sampleRate[] = {10.0, 5.0, 4.0, 2.5, 2.0, 1.0, 0.5};
int Rate=modh->getSampleRate();
return (Rate<7) ?
   sampleRate[Rate] : 1.0;
*/
}

u_int unPack::getSampleSize(int lch)
{ //return (fileType) ? emod->getSampleSize(lch) : modh->getSampleSize();
}


/*\
 *	scans for all banks which are contained in an Eveh
 *	bank. Up to now we only have ACQC banks in each event.
\*/

void unPack::scanAllBanks(u_int eventNumber) {
        /*
                bankVect.clear();

                dataFile->goToPos(runEventPos_v[eventNumber].evehPos);

                //_BOS_EVEH *eveh;
                getBankHeader(eveh);

                if( dbg ) {
                        cout << "scanning event Number :" << eveh->getEventNumber() << endl;
                        cout << "event Size :           " << eveh->getEventSize() << endl;
                        }

                for(u_int c=0;c<runEventPos_v[eventNumber].headerPOS_v.size();c++) {
                        u_int thisPos = runEventPos_v[eventNumber].headerPOS_v[c];
                        dataFile->goToPos(thisPos);
                        _BOS_GEN *gen;
                        getBankHeader(gen);
//test			gen->putFilePos(thisPos);
                        bankVect.push_back(gen);
                        if( dbg ) cout << form("modid %x length %x \n",gen->getModId(),gen->getLength() );
                        }
*/	
}

/*\
 *	returns all the module numbers which are in a
 *	particular event
\*/

uint_V unPack::getModId(u_int eventNumber) {
        /*
                uint_V number_v;
                        scanAllBanks(eventNumber);		// this generates the right bankVect for this event

                        for(u_int i = 0 ;i<bankVect.size();i++ )
                                number_v.push_back( bankVect[i]->getModId() );

                return number_v;
*/	
}


/*\
 *	calculates the file position of the data for a particular module
 *	bankVect has to be filled by getModId( eventNumber ) -> scanAllBanks(eventNumber)
\*/

u_int unPack::getACQCFilePos(u_int modid) { 
        /*
                u_int pos = 0xffffffff;

                for(u_int i = 0 ;i<bankVect.size();i++ ) {			// loop over all banks in this event

                        if( bankVect[i]->getModId() == modid )
              return 0; //test
//test				return (bankVect[i]->getFilePos()+_BOS_GEN::size*sizeof(int));
                        }
                return pos;
*/	
}

/*\
 *	calculates the vector position of the data for a particular module
 *	bankVect has to be filled by getModId( eventNumber ) -> scanAllBanks(eventNumber)
\*/


int unPack::getACQCVectPos(u_int modid) { 	
        /*
                int pos = -1;
                for(u_int i = 0 ;i<bankVect.size();i++ ) 			// loop over all banks in this event

                        if( bankVect[i]->getModId() == modid )
                                return i;

                return pos;
*/
}

/*\
 *	reads the data of a certain channel and puts it into "data"
 *	bankVect has to be filled by getModId( eventNumber ) -> scanAllBanks(eventNumber)
 *	returns size of the data in byte
\*/

u_int unPack::getACQCData(char *&data,u_int modid) {
        /*
                u_int dataSize = 0;
                int index = 0;

                if( (index = getACQCVectPos(modid)) != -1 ) {
                        if( bankVect[index]->getLength() > 0 ) {
                                dataSize = bankVect[index]->getLength()*sizeof(int)-_BOS_GEN::size*sizeof(int);

                                data = new char[dataSize];		// what if there is no memory ????
//test				dataFile->goToPos(bankVect[index]->getFilePos()+_BOS_GEN::size*sizeof(int));

                                if(  dataSize != dataFile->read(data,dataSize))
                                        cout << form("couldn't read GEN  0x%x \n",bankVect[index]->getModId());
                                else
                                        return dataSize;
                                }
                        else
                                return 0;

                        }

                return dataSize;
*/
}

/*\
 *	reads the data of a certain channel and puts the data size into "dataSize"
 *	bankVect has to be filled by getModId( eventNumber ) -> scanAllBanks(eventNumber)
 *	returns a pointer to the data
\*/

/* dataSize: is it correct to subtract sizeof(_BOS_GEN)=16 ?
   should we better subtract sizeof(_GEN)=0 ?
   length = size of data
   FilePos = Pos von Header
*/
char *unPack::getACQCData(u_int &dataSize,u_int modid) {
        /*
                dataSize = 0;
                int index = 0;

                if( (index = getACQCVectPos(modid)) != -1 ) {
                        if( bankVect[index]->getLength() > 0 ) {
        dataSize = bankVect[index]->getLength()*sizeof(int)-_BOS_GEN::size*sizeof(int);

//test				dataFile->goToPos(bankVect[index]->getFilePos()+_BOS_GEN::size*sizeof(int));

                                char *data = new char[dataSize];		// what if there is no memory ????
                                if(  dataSize == dataFile->read(data,dataSize))
                                        return data;
                                else	{
                                        cout << form("couldn't read GEN  0x%x \n",bankVect[index]->getModId());
                                        return (char *)0;
                                        }
                                }
                        }
                return (char *)0;
*/	
}

/*\
 *	generates a description vector of all pulses
\*/


BOS_V unPack::generateDesc(u_int modid,bool debug = false) {
        /*
        myChannel = modid;

        for(u_int i = 0;i<dataDesc_v.size();i++) {
                dataDesc_v[i]->~_BOS();
                }

        dataDesc_v.clear();

        u_int fullSize;

        char *data = getACQCData(fullSize,modid);

        if(debug) {
    cout << form("1 data %x fullSize %d\n",data,fullSize);
    cout << form("%c%c%c%c \n",data[0],data[1],data[2],data[3]);
    cout << form("%d%d%d%d \n",(int)data[0],(int)data[1],(int)data[2],(int)data[3]);
    }

        if(fullSize ) {
    // we have uncompressed data for testing
                //_BOS *bank = new _BOS(*(u_int *)"xxxx",0,0,fullSize);
        //dataDesc_v.push_back( bank );
    //delete data;

    // this handles compression
                char *dataBegin = data;
                fullSize -= sizeof(CompHead);
                do{
                        PtrCompHead head = (PtrCompHead)data;
                        u_int length = ntohl(head->length);
                        if(debug) cout << form("index %x length %x \n",ntohl(head->index),length);

                        _BOS *bank = new _BOS(*(u_int *)"xxxx",ntohl(head->index),data - dataBegin,length);

                        data += length+sizeof(CompHead);
                        dataDesc_v.push_back( bank );

                        //delete bank;
                        } while( (u_int)(data - dataBegin) < fullSize );
                delete dataBegin;
                }

        if(debug) printDataDesc();

        return dataDesc_v;
*/
}

/*\
 *	generate a descriptor for the etep zero suppressed data
\*/


BOS_V unPack::generateETEPDesc(u_int modid,bool debug = false) {
        /*
        ETEPCOMP *ecomp;
        unsigned int dataP = 0,pre_dataP = 0,pre_time = 0;
        unsigned int dataLength = 0;
        unsigned int mytime,mylength;
        myChannel = modid;
  unsigned int *t;
        for(u_int i = 0;i<dataDesc_v.size();i++) {
                dataDesc_v[i]->~_BOS();
                }

        dataDesc_v.clear();

        u_int fullSize;

        char *data = getACQCData(fullSize,modid);

        if(debug) {
    cout << form("1 data %x fullSize %d\n",data,fullSize);
    cout << form("%c%c%c%c \n",data[0],data[1],data[2],data[3]);
    cout << form("%d%d%d%d \n",(int)data[0],(int)data[1],(int)data[2],(int)data[3]);
    }

        if(fullSize ) {

                fullSize -= sizeof(CompHead);

                u_int idx = 8; // that's where the first sync word can start

                do {

                        // first scan for the sync word
                        while( idx < fullSize && *(data + idx) != 0x15) idx++;

                                t = (unsigned int *)(data + idx);
                                if(*t == SyncStamp) {
                                        ecomp = (ETEPCOMP *)(t-2);
                                        pre_dataP = dataP;
                                        idx += 24;					// beginning of data block
                                        dataP = idx;

                                        if( pre_dataP ) {
                                                dataLength = (dataP - pre_dataP) - sizeof(ETEPCOMP);
            mylength = dataLength;
                                                mytime = pre_time;
                                                // this has to change since there is  time0MSB+time0LSB (64 bit !)
                                                if(dataLength>1) {
                                                        _BOS *bank = new _BOS(*(u_int *)"xxxx",pre_time,pre_dataP-sizeof(ETEPCOMP),dataLength); // ????
                                                        dataDesc_v.push_back( bank );
                                                        }

                                                if( dataLength <= 0) {
                                                                cout << "dataLength " << dataLength << endl;
                                                                }
                                                if( ecomp->time0LSB + 1 != ecomp->time1LSB )
                                                        cout << form("time 0 0x%x time 1 0x%x \n",ecomp->time0LSB,ecomp->time1LSB);
            }

                                        if(debug) {
                                                cout << form("0x%08x 0x%08x ",ecomp->time0LSB,ecomp->time0MSB );
                                                cout << form("0x%08x 0x%08x ",ecomp->sync,ecomp->empty );
                                                cout << form("0x%08x 0x%08x ",ecomp->time1LSB,ecomp->time1MSB );
                                                cout << form("0x%08x 0x%08x ",ecomp->sync1,ecomp->notknown );
                                                cout << form("mt 0x%08x ml 0x%08x mt+l 0x%08x ",mytime,mylength,mytime+mylength );

            if( mytime+mylength > ecomp->time0LSB ) cout << ">>----------------<<" << endl;
                                                if(dataLength)
                                                        cout << form(" 0x%08x 0x%08x 0x%08x\n",pre_dataP,dataP,dataLength );
                                                else
                                                        cout << endl;
                                                }
                                        }

     // pre_time =( ecomp->time0LSB ) * 16 ;   // * 16 ns !
      pre_time =( ecomp->time0LSB );

                        } while( ++idx < fullSize );

                }

        if(debug) printDataDesc();

        return dataDesc_v;
*/
}



/*\
 *	for debugging
\*/

void unPack::printDataDesc() {
        /*
                cout << form("DESC OUTPUT : size %d \n",dataDesc_v.size());
                for(u_int i = 0;i<dataDesc_v.size();i++)
                        cout << form("file pos %x  index %x size %x \n",dataDesc_v[i]->getFilePos(),dataDesc_v[i]->getNumber(),dataDesc_v[i]->getLength());
*/	
}


/*\
 *	reads the puls defined by the _BOS bank pointer
 *	the data is placed in "data"
 *	returns true if the data of the pulse can be read
\*/

bool unPack::getPuls(_BOS *bank,unsigned char *&data) {
        /*
          // search in bankVect for modId myChannel and return filePos of this bank
                u_int eventPos = getACQCFilePos(myChannel);

                u_int absPos = eventPos+bank->getFilePos()+sizeof(CompHead);

                dataFile->goToPos(absPos);

                u_int dataSize = bank->getLength();

                data = new unsigned char[dataSize];

                if(  dataSize == dataFile->read(data,dataSize))
                        return true;

                return false;
*/	
}

/*\
 *	reads the puls from an ETEP digitizer defined by the _BOS bank pointer
 *	the data is placed in "data"
 *	returns true if the data of the pulse can be read
\*/

bool unPack::getETEPPuls(_BOS *bank,unsigned char *&data) {
        /*
          // search in bankVect for modId myChannel and return filePos of this bank
                u_int eventPos = getACQCFilePos(myChannel);

                u_int absPos = eventPos+bank->getFilePos()+sizeof(ETEPCOMP);

                dataFile->goToPos(absPos);

                u_int dataSize = bank->getLength();

                data = new unsigned char[dataSize];

                if(  dataSize == dataFile->read(data,dataSize))
                        return true;

                return false;
*/	
}

void unPack::terminate() {
        dataFile->close();
}


