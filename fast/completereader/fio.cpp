#include "fio.h"

#include <stdio.h>
//#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <string.h>
//#include <netinet/in.h>
//#include <stream.h>

//#include "header.h"

//#include "DaqDefs.h"	//	testing

f_i::f_i(string fname) {
		inFile.open(fname.c_str());

		if( !inFile ) { // open failed
                        cout << "cannot open file " << fname << "\n";
			}
		}
			
unsigned int f_i::read(unsigned char *buffer,streamsize size) {
		inFile.read((char*)buffer,size);
		unsigned int rsize = inFile.gcount();
		//cout << form("fio read:  buffer %x size %x rsize %x filepos %x \n",buffer,size,rsize,inFile.tellg()) << flush;
		return rsize;
		}		

unsigned int f_i::read(char *buffer,streamsize size) {
		inFile.read(buffer,size);
		unsigned int rsize = inFile.gcount();
		//cout << form("fio read:  buffer %x size %x rsize %x filepos %x \n",buffer,size,rsize,inFile.tellg()) << flush;
		return rsize;
		}		
		
unsigned int f_i::readFrame(unsigned char *&buffer,int start,int bufSize){

		int get_size =0;
	
		inFile.seekg(start);

		inFile.read((char*)buffer,bufSize);
		get_size = inFile.gcount();
		inFile.close();		

		return 	get_size;
		}

			
f_i::~f_i() {
		inFile.close();
		}
			
void f_i::close(){
		inFile.close();
		}

unsigned int f_i::size() {
		inFile.seekg(0,std::ios::end);
        	std::streampos fSize = inFile.tellg();
		inFile.seekg(0);
		return (unsigned int)fSize;
		}
		
void f_i::resetFilePos() {
		inFile.clear();
		inFile.seekg(0);
		}
			
void f_i::goToPos(u_int pos) {
		inFile.seekg(pos,std::ios::beg);
		}
			
void f_i::step(u_int pos) {
		inFile.seekg(pos,std::ios::cur);
		}

u_int f_i::curPos() {
    return 0;
		//return inFile.seekg(0,std::ios::cur);
		}
		
std::streampos f_i::fPos() {
		return inFile.tellg();
		}

fileReader::fileReader(string file) {

		fname = file;
		fileSize = size();	
		}
			
int fileReader::readFrame(unsigned char *&buffer,int start, int bufSize) {
		int get_size =0;

		inFile.open(fname.c_str());
	
	 	if( !inFile ) { 					// open failed
			cerr << "readFrame cannot open file " << fname << "\n";
			return 0;
			}
	
		inFile.seekg(start);

		inFile.read((char*)buffer,bufSize);
		get_size = inFile.gcount();
		inFile.close();		

		return 	get_size;
		}
		
int fileReader::size() {
		inFile.open(fname.c_str());
		inFile.seekg(0,std::ios::end);
        	std::streampos fSize = inFile.tellg();

		inFile.close();
		return (unsigned int)fSize;
		}	


f_o::f_o(string fname) {
	outFile.open(fname.c_str());

	if( !outFile ) { // open failed
		cerr << "cannot open file " << fname << "\n";
		exit(-1);
		}
	}
		
void f_o::open(string fname) {
	outFile.open(fname.c_str());
	
	if( !outFile ) { // open failed
		cerr << "cannot open file " << fname << "\n";
		exit(-1);
		}
	}
		
void f_o::write(char *buffer,streamsize size) {
		outFile.write(buffer,size);
		}
			
f_o::~f_o() {
	outFile.close();
	}
			
void f_o::close(){
	outFile.close();
	}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
f_ox::f_ox()
{
fdesc=-1;
}
//-----------------------------------------------------------------------------
f_ox::~f_ox()
{
if (fdesc>=0) ::close(fdesc);
fdesc=-1;
}
//-----------------------------------------------------------------------------
void f_ox::open(char* fname)
{
if (fdesc>=0) ::close(fdesc);
if ((fdesc = ::open(fname, O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0) {
      fprintf(stderr, "Failed to open file %s: %s\n",fname, strerror(errno));
	    }
}
//-----------------------------------------------------------------------------
void f_ox::close()
{
::close(fdesc);
fdesc=-1;
}
//-----------------------------------------------------------------------------
void f_ox::write(char *buffer,u_int size)
{
if ((u_int(::write(fdesc,buffer,size ))) !=  size) {
		fprintf(stderr, "Writing to disk failed: %s\n", strerror(errno));
		}                                                                       		
}
//-----------------------------------------------------------------------------
int f_ox::getFilePos()
{
return lseek(fdesc,0,SEEK_CUR);	
}
//-----------------------------------------------------------------------------
void f_ox::setFilePos(unsigned int fpos)
{
  if (fpos==0xffffffff) {
     // go to the file end
     lseek(fdesc,0,SEEK_END);	
     return;
     }
   lseek(fdesc,fpos,SEEK_SET);	
   return;
}
//-----------------------------------------------------------------------------
bool f_ox::operator ()()
{
return (fdesc>=0);
}
//-----------------------------------------------------------------------------
