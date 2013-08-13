#ifndef FIO_H
#define FIO_H
#include <iostream>
#include <string>
#include <fstream>
#include <string.h>
#include <stdlib.h>
//#include <strstream.h>
//#include <stream>

#define BUFFERSIZE 1024

using namespace std;

class f_i {
private:
        ifstream inFile;
public:
        f_i(string fname);
        unsigned int read(unsigned char *buffer,streamsize size = BUFFERSIZE);
        unsigned int read(char *buffer,streamsize size = BUFFERSIZE);
        unsigned int readFrame(unsigned char *&buffer,int start,int bufSize);

        ~f_i();
        void close();
        unsigned int size();
        void resetFilePos();
        void goToPos(u_int pos);
        void step(u_int pos);
        std::streampos fPos();
        u_int curPos();
        bool isBad() {return (inFile==NULL);};
};

class fileReader {
public: 
        fileReader(string file);
        ~fileReader();

        int readFrame(unsigned char *&buffer,int start, int bufSize);
        int size();

protected: // Protected attributes

        int fileSize;
        ifstream inFile;
        string fname;
};



class f_o {
private:
        ofstream outFile;
public:
        f_o() { }
        f_o(string fname);
        void open(string fname);
        void close();
        void write(char *buffer,streamsize size = BUFFERSIZE);
        ~f_o();
};	


class f_ox {
private:
        int fdesc;
public:
        f_ox();
        ~f_ox();
        void open(char* fname);
        void close();
        void write(char *buffer,unsigned int size);
        int getFilePos();
        void setFilePos(unsigned int fpos);
        bool operator ()();
};	


#endif
