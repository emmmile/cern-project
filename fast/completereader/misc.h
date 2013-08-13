#ifndef MISC_H
#define MISC_H
#include <iostream>
#include <typeinfo>
#include <cstdlib>
#include <sstream> // was strstream.h
//#include <stream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>	
#include "DaqType.h"

#define SIZE 200

using namespace std;



Config_V GetConfigVal(string filename,string name,bool dbg = false);
bool rval(unsigned int &arg,string filename);
HVList GetHVVal(string filename);
void dump(char *buf,int size);
void changeItemInFile(string fileName,string add,string remove);
string getFirstItem(string fileName);
int queryMounted(char* aPath);




bool GetDefaultString(string &value,string filename, string name,bool dbg = false);
bool GetDefaultInt(int &value,string filename, string name,bool dbg = false);
bool GetDefaultUnsigned(u_int &value,string filename, string name,bool dbg = false);
bool GetDefaultFloat(float &value,string filename, string name,bool dbg = false);
/*
template <class TYPE> 
bool GetDefaultValue(TYPE &value,string filename, string name,bool dbg = false) {

	bool rstat = false;
	
	char t_str[SIZE];
	ifstream ifs(filename.c_str());
	
	if( !ifs ) { // open failed
		cerr << "cannot open file " << filename << endl;
		return;
		}

	string separators("\t ");
   
	string val;
   	string::size_type pos,pose;
	while ( ifs.getline(t_str,SIZE)) {
		if(dbg) cout << t_str << "  " << name << endl;
		string line = t_str;
		if (line.find_first_of("#") != (string::size_type)0 )
			if(pos = line.find(name) != string::npos ) {
				pos += name.size();
				pose = line.find_first_of(separators,pos);
				val = line.substr(pos,pose);
				if(dbg) cout << "val " << val << endl;
				if( typeid( TYPE ) == typeid( int ) &&
				val.find("x") != string::npos ){
					val.erase(0,2);
					istrstream ist(val.c_str());
					ist >> hex >> value;
					rstat = true;
					}
				else if( typeid( TYPE ) == typeid( bool ) ) {
						if(dbg) cout << "trying bool ... " << val.c_str();
						value = (val == "true") ? true : false;
						rstat = true;
						}
				else {
					istrstream ist(val.c_str());
					ist >>  value;
					rstat = true;
					}
				break;
				}
   	}

return rstat;

}

template <class TYPE>
void GetDefaultValue(TYPE &value,string filename, string name,bool &rstat,bool dbg = false) {
	
	rstat = false;
	
	char t_str[SIZE];
	ifstream ifs(filename.c_str());
	
	if( !ifs ) { // open failed
		cerr << "cannot open file " << filename << endl;
		return;
		}

	string separators("\t ");

	string val;
   	string::size_type pos,pose;
	while ( ifs.getline(t_str,SIZE)) {
		if(dbg) cout << t_str << "  " << name << endl;
		string line = t_str;
		if (line.find_first_of("#") != (string::size_type)0 )
			if(pos = line.find(name) != string::npos ) {
				pos += name.size();
				pose = line.find_first_of(separators,pos);
				val = line.substr(pos,pose);
				if(dbg) cout << "val " << val << endl;
				if( typeid( TYPE ) == typeid( int ) &&
				val.find("x") != string::npos ){
					val.erase(0,2);
					istrstream ist(val.c_str());
					ist >> hex >> value;
					rstat = true;
					}
				else if( typeid( TYPE ) == typeid( bool ) ) {
						if(dbg) cout << "trying bool ... " << val.c_str();
						value = (val == "true") ? true : false;
						rstat = true;
						}
				else {
					istrstream ist(val.c_str());
					ist >>  value;
					rstat = true;
					}
				break;
				}
   	}
}
*/


template <class TYPE> 
TYPE Align(TYPE buf) {
return ( (buf%sizeof(TYPE)) ) ? ((buf/sizeof(TYPE))+1)*sizeof(TYPE) : buf;
}

#endif
