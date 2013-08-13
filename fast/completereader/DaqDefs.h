/***************************************************************************
                          DaqDefs.h  -  description
                             -------------------
    begin                : Mon Jul 10 2000
    copyright            : (C) 2000-2002 by Ralf Plag & Erich Schaefer
    email                : Ralf.Plag@cern.ch
 ***************************************************************************/
#ifndef DAQDEFS_H
#define DAQDEFS_H

const unsigned int MaxChannels = 8;

/* a port list in the web lists the following ports
   (ref: http://keir.net/portlist.html)
       TCP  4011  Alternate Service Boot
       TCP  4012  PDA Gate
       TCP  4013  ACL Manager
       TCP  4014  TAICLOCK
       TCP  4015  Talarian Mcast
       TCP  4016  Talarian Mcast
       TCP  4017  Talarian Mcast
       TCP  4018  Talarian Mcast
       TCP  4019  Talarian Mcast
       TCP  4045  nfs-lockd
so we better use ports from 4020 to 4044 */

#define RUNCONTROL_REC_PORT 4040
#define PROD_REC_PORT       4041
#define PACQ_REC_PORT       4042
#define HV_REC_PORT         4043

/*
#define configFile "/daq/NTOFDaq/Config/ETEP.conf"
#define lastRunInfoFile "/daq/NTOFDaq/Config/ElastRun.info"
#define RUNNUMBERFILE "/NTOFDaq/Config/ErunNumber"
*/
#define configFile         "config/READOUT.conf"
#define lastRunInfoFile    "/daq/NTOFDaq/Config/lastRun.info"
#define RUNNUMBERFILE      "/NTOFDaq/Config/runNumber"

#define daqStatusFile      "/daq/NTOFDaq/Config/daqStatus.info"
#define sampleListFile     "/daq/NTOFDaq/Config/samples.info"
#define sampleInBeamFile   "/daq/NTOFDaq/Config/sampleInBeam.info"

#define runInfoFile        "/daq/NTOFDaq/Log/run.info"
#define edisplayConfigFile "/daq/NTOFDaq/Config/DISPLAY.conf"
#define beamConfigFile     "/daq/NTOFDaq/Config/beam.conf"
#define hvFile             "/daq/NTOFDaq/Config/VImon.cfg"
#define totalNumberOfProtonsFile "/daq/NTOFDaq/Config/totalNumberOfProtons"
#define filterInfoFile      "/daq/NTOFDaq/Config/filters.info"

#define fileFinished       ".finished"
#define maxNumberOfStreams 20

/*\
 *  Sample rate Table, in Mega samples per second
\*/

//int sampleRateTable[] = {100,200,250,400,500,1000,2000};

#define SILI "SILI"
#define C6D6 "C6D6"
#define BAF2 "BAF2"
#define PPAC "PPAC"


#endif
