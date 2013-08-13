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
        correlator*  data;
public:
        ///
        /// \brief Construct a \ref castor object, starting reading the data of the detectors.
        ///
        /// \param path the location of the experiment directory on CASTOR.
        /// \param run the run number.
        /// \param rs the run start time.
        /// \param us the start time specified by the user.
        /// \param d pointer to the correlator, where the final data is stored.
        ///
        castor( const string& path, const int& run, const time_t rs, const time_t us, correlator* d )
                : runstart( rs ), userstart( us ), data( d ) {

                // open the file and get some informations, for example exact date and detectors numbers and names
                theRunReader=new RunReader();
                theRunReader->SetDirName(path);
                theRunReader->SetRunNumber(run);
                theRunReader->SetSegmentNumber(0); // the first segment is enough for us
                info = theRunReader->GetStructure();

                if ( !info ) throw exception( );

                // finally reads the data, one detector after the other
                for ( int i = 0; i < info->NChannels; ++i ) {
                        readDetector( info->theChannelInfo[i].DetType, info->theChannelInfo[i].DetID );
                }
        }


        /// \brief Read the data for a specific detector of the current run. After reading it runs the
        /// peak analyzer saving the results into \ref out.
        ///
        /// \param detector the detector name.
        /// \param id the detector id.
        /// \param out a stream where to write the (analyzed) data.
        ///
        void readDetector( const string& detector, const int& id ) {
                cout << "[fast] Reading " << detector << id << "..." << endl;

                theRunReader->SetDetName(detector);
                theRunReader->SetDetID(id);

                int TotalNEvents,TotalNSignals, firstevent, theEventN,theSignalN;
                firstevent = 1;

                // Check number of events in the file
                TotalNEvents=theRunReader->Init();
                if(TotalNEvents<0) return;


                theACQCInfo=theRunReader->GetACQCInfo();

                // Determine number of signals in each event, loop over events and save all signals into a file
                double first_channel, first_time, last_time, channelwidth;
                int totalcount = 0;


                // Loop over events
                for(theEventN=firstevent; theEventN<=TotalNEvents; theEventN++) {
                        // just for diagnostic purposes
                        string name = detector + std::to_string((long long int)id) + " event " + std::to_string((long long int)theEventN);

                        //  Loop over signals
                        TotalNSignals=theRunReader->TakeEvent(theEventN);
                        if(TotalNSignals<0) return;

                        for(theSignalN=1; theSignalN<=TotalNSignals; theSignalN++) {

                                peakanalyzer<double> p( name );

                                // printf("Reading/Writing: Run %d %s %d Event %d Signal %d\n",RunN,DetName.c_str(),DetID,theEventN,theSignalN);
                                SignalInfo* theSignalInfo=theACQCInfo->theSignal[theSignalN-1];
                                first_channel=theSignalInfo->TimeStamp-theRunReader->GetPreSamples();
                                channelwidth=1./theRunReader->GetSampleRate()*1000.;//->ns/ch
                                first_time=(first_channel-0.5)*channelwidth;
                                last_time=(first_channel+theSignalInfo->PulseLength-0.5)*channelwidth;


                                for(unsigned int i=0;i<theSignalInfo->PulseLength;i++) {
                                        double time = (double)first_time+i*channelwidth;
                                        double value = (double)theSignalInfo->data[i];
                                        p.push( time, value );

                                        //fprintf(fout,"%f %f\n",(double)first_time+i*channelwidth,(double)theSignalInfo->data[i]);
                                }

                                totalcount += p.peaks();
                        }
                }

                string detectorname = detector + std::to_string((long long int)id);

                // send a value to the correlator (consequently also the GUI)
                data->lock();
                data->addPoint( detectorname, runstart - userstart, totalcount );
                data->unlock();

                cout << "[fast] " << detector << id << " has " << totalcount << " peaks." << endl;
        }
};

#endif // CASTOR_HPP
