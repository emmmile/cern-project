/*==========================================================================================
====  Carlos Guerrero (July 16th 2013)                                                ======
====  A simple program to acces RAW n_TOF data based on E. Mendoza's "SignalAnalyzer" ======
============================================================================================*/

#include <fstream>
#include "RawReader.hh"
#define CASTORDIR "/castor/cern.ch/ntof/2012/fission/"
using namespace std;

ACQCInfo* theACQCInfo;
SignalInfo* theSignalInfo;
RunReader* theRunReader;

int main(int argc, char **argv) 
{
        int TotalNEvents,TotalNSignals;
        int theEventN,theSignalN;
        string DetName, DirName;
        int RunN,DetID, NSegment,firstevent;
        int check=0;

        // Read input choices
        if(argc!=4 && argc!=5 && argc!=6 && argc!=7 && argc!=8){
                cout<<endl<<"  **** Input has to be like: RawReader.exe <Run> <DetName> <DetId> [<CASTORDIR>=2012/fission] [<segment=0>] [<first event=1>] ****"<<endl<<endl;
                return -1;
        }
        RunN=atoi(argv[1]);
        DetName=string(argv[2]);
        DetID=atoi(argv[3]);
        NSegment=0;
        firstevent=1;
        char castordir[100]=CASTORDIR;
        if(argc>4)
                sprintf(castordir,"/castor/cern.ch/ntof/%s/",argv[4]);
        if(argc>5)
                NSegment=atoi(argv[5]);
        if(argc>6)
                firstevent=atoi(argv[6]);


        // Initializing run data
        printf("Run %d %s %d in %s\n",RunN,DetName.c_str(),DetID, castordir);
        theRunReader=new RunReader();
        theRunReader->SetDirName(castordir);
        theRunReader->SetDetName(DetName);
        theRunReader->SetDetID(DetID);
        theRunReader->SetRunNumber(RunN);
        theRunReader->SetSegmentNumber(NSegment);

        // Check number of events in the file
        TotalNEvents=theRunReader->Init();
        if(TotalNEvents<0)
                return 1;
        //  else
        //     cout<<"Total number of events in this file: "<<TotalNEvents<<endl;
        theACQCInfo=theRunReader->GetACQCInfo();

        // Determine number of signals in each event, loop over events and save all signals into a file
        int i=0, j=0;
        char foutname[100];
        double first_channel = 0, channelwidth = 0, first_time = 0, last_time = 0;
        FILE *fout;
        // Loop over events
        for(theEventN=firstevent; theEventN<=TotalNEvents; theEventN++)
        {
                //  Loop over signals
                TotalNSignals=theRunReader->TakeEvent(theEventN);
                if(TotalNSignals<0)
                        return 1;
                // else
                //cout<<"Total number of signals in this event: "<<TotalNSignals<<endl;
                for(theSignalN=1; theSignalN<=TotalNSignals; theSignalN++)
                {
                        sprintf(foutname,"./%s%d_r%d_segm%d_ev%d%d.dat",DetName.c_str(),DetID, RunN, NSegment,theEventN,theSignalN);
                        printf("Reading and Writing into: %s\n", foutname);
                        fout = fopen(foutname,"w+");
                        //printf("Reading/Writing: Run %d %s %d Event %d Signal %d\n",RunN,DetName.c_str(),DetID,theEventN,theSignalN);
                        theSignalInfo=theACQCInfo->theSignal[theSignalN-1];
                        first_channel=theSignalInfo->TimeStamp-theRunReader->GetPreSamples();
                        channelwidth=1./theRunReader->GetSampleRate()*1000.;//->ns/ch
                        first_time=(first_channel-0.5)*channelwidth;
                        last_time=(first_channel+theSignalInfo->PulseLength-0.5)*channelwidth;
                        //printf( "%lf %lf %lf %lf\n", first_channel, channelwidth, first_time, last_time );
                        for(unsigned int i=0;i<theSignalInfo->PulseLength;i++) {
                                if ( int( i*channelwidth ) % 5 != 0 ) break;
                                if ( int( (double)first_time+i*channelwidth ) % 5 != 0 ) break;
                                fprintf(fout,"%lf %lf\n",(double)first_time+i*channelwidth,(double)theSignalInfo->data[i]);
                        }
                        fclose(fout);
                }
        }
        return 0;

}
