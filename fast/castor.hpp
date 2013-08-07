#ifndef CASTOR_HPP
#define CASTOR_HPP

#include "fast/reader/RawReader.hh"
#include "fast/peakanalyzer.hpp"
#include "gui.hpp"
#include <iostream>
using namespace std;



///
/// \brief The castor class reads a single run from CASTOR, processing it with a peak analyzer.
///
class castor {

        RunReader*   theRunReader;
        ACQCInfo*    theACQCInfo;
        ModuleInfo*  info;

        time_t       runstart;
        time_t       userstart;
        gui*         window;
public:
        ///
        /// \brief Construct a \ref castor object, starting reading the data of the detectors.
        ///
        /// \param path the location of the experiment directory on CASTOR.
        /// \param run the run number.
        /// \param out a stream where to write the (analyzed) data.
        /// \param s starting time, it is actually used only for viewing on the GUI.
        /// \param w pointer to the GUI window.
        ///
        castor( const string& path, const int& run, fstream& out, time_t rs, time_t us, gui* w )
                : runstart( rs ), userstart( us ), window( w ) {
                // open the file and get some informations, for example exact date and detectors numbers and names
                theRunReader=new RunReader();
                theRunReader->SetDirName(path);
                theRunReader->SetRunNumber(run);
                theRunReader->SetSegmentNumber(0); // the first segment is enough for us
                info = theRunReader->GetStructure();

                if ( !info ) {
                        throw exception( );
                }

                //cerr << "[fast] Run " << run << " has " << info->NChannels << " channels:" << endl;
                //cout << *info << endl;


                // now I should have all the information needed for the future
                for ( int i = 0; i < info->NChannels; ++i ) {
                        get( info->theChannelInfo[i].DetType, info->theChannelInfo[i].DetID, out );
                        getchar();

                        //delete info;
                        //theRunReader=new RunReader();
                        /*theRunReader->SetDirName(path);
            theRunReader->SetRunNumber(run);
            theRunReader->SetSegmentNumber(0); // the first segment is enough for us
            info = theRunReader->GetStructure();*/
                }
        }

        ///
        /// \brief Read the data for a specific detector of the current run. After reading it runs the
        /// peak analyzer saving the results into \ref out.
        ///
        /// \param detector the detector name.
        /// \param id the detector id.
        /// \param out a stream where to write the (analyzed) data.
        /// \param start starting time, it is actually used only for viewing on the GUI.
        ///
        void get( const string& detector, const int& id, fstream& out ) {
                cout << "[fast] Reading " << detector << id << "..." << endl;
                // TODO check if detector and id are in info
                theRunReader->SetDetName(detector);
                theRunReader->SetDetID(id);

                int TotalNEvents,TotalNSignals, firstevent, theEventN,theSignalN;
                firstevent = 1;

                // Check number of events in the file
                TotalNEvents=theRunReader->Init();
                if(TotalNEvents<0) return;
                //else cout<<"Total number of events in this file: "<<TotalNEvents<<endl;
                theACQCInfo=theRunReader->GetACQCInfo();

                // Determine number of signals in each event, loop over events and save all signals into a file
                int first_channel, first_time, last_time;
                float channelwidth;

                //        FILE *fout;
                int totalcount = 0;
                // Loop over events
                for(theEventN=firstevent; theEventN<=TotalNEvents; theEventN++) {


                        //            if ( theEventN == 1 ) {
                        //                char foutname[200];
                        //                printf( "./%s%d_r%d_segm%d_ev%d.dat\n", detector.c_str(),id, theRunReader->RunN, theRunReader->SegmentN, theEventN);
                        //                sprintf(foutname,"./%s%d_r%d_segm%d_ev%d.dat",detector.c_str(),id, theRunReader->RunN, theRunReader->SegmentN,theEventN);

                        //                fout = fopen(foutname,"w+");
                        //            }
                        //  Loop over signals
                        TotalNSignals=theRunReader->TakeEvent(theEventN);
                        if(TotalNSignals<0)
                                return;
                        // else
                        //printf("Reading and Writing into: %s\n", foutname);
                        for(theSignalN=1; theSignalN<=TotalNSignals; theSignalN++) {

                                peakanalyzer<double> p( detector + std::to_string((long long int)id) + " event " + std::to_string((long long int)theEventN) );



                                // printf("Reading/Writing: Run %d %s %d Event %d Signal %d\n",RunN,DetName.c_str(),DetID,theEventN,theSignalN);
                                SignalInfo* theSignalInfo=theACQCInfo->theSignal[theSignalN-1];
                                first_channel=theSignalInfo->TimeStamp-theRunReader->GetPreSamples();
                                channelwidth=1./theRunReader->GetSampleRate()*1000.;//->ns/ch
                                first_time=(first_channel-0.5)*channelwidth;
                                last_time=(first_channel+theSignalInfo->PulseLength-0.5)*channelwidth;



                                //if ( theEventN == 1 ) {
                                printf( "%d %f %d %f\n", first_channel, channelwidth,theRunReader->GetSampleRate(), 1./theRunReader->GetSampleRate()*1000. );
                                for(unsigned int i=0;i<theSignalInfo->PulseLength;i++) {
                                        float time = (float)first_time+i*channelwidth;
                                        float value = (float)theSignalInfo->data[i];
                                        p.push( time, value );

                                        //fprintf(fout,"%f %f\n",(float)first_time+i*channelwidth,(float)theSignalInfo->data[i]);
                                }
                                //}

                                totalcount += p.peaks();
                        }
                }


                string detectorname = detector + std::to_string((long long int)id);
                // prints a pair <DETECTOR, value>
                out << "<" << detectorname << ", " << totalcount << ">\t";
                out.flush();


                // push point to the gui
                window->addPoint( detectorname, runstart - userstart, totalcount );


                cout << "[fast] " << detector << id << " has " << totalcount << " peaks." << endl;


                // after one detector has been read I start analyzing
                //if ( __analyzer.joinable() ) __analyzer.join();

                //peakanalyzer p( detector + std::to_string((long long int)id) );
                //__analyzer = thread( &peakanalyzer::analyze, p, std::ref( __stream ), std::ref( __stream_mutex ) );
        }
};

#endif // CASTOR_HPP
