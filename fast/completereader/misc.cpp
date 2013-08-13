#include "misc.h"
#include <string.h>
#include <stdlib.h>

#define INTC_as_int 0x43544e49
#define EXTC_as_int 0x43545845
#define ACQD_as_int 0x44514341
#define NONE_as_int 0x454e4f4e

// ------------------------------------------------------------------------------------------
bool GetDefaultString(string &value,string filename, string name,bool dbg) 
{
	
	char t_str[SIZE];
	ifstream ifs(filename.c_str());
	
	if( !ifs ) { // open failed
		cerr << "cannot open file " << filename << endl;
		return false;
		}

	string separators("\t ");

   	string::size_type pos,pose;
	while ( ifs.getline(t_str,SIZE)) {
		if(dbg) cout << t_str << "  " << name << endl;
		string line = t_str;
		if (line.find_first_of("#") != (string::size_type)0 )
			if( (pos = line.find(name)) != string::npos ) {
				pos += name.size()+1;
				pose = line.find_first_of(separators,pos);
				value = line.substr(pos,pose);
				if(dbg) printf("Key: %s Value: '%s'\n",name.c_str(),value.c_str());
				return true;
				}
   	}
return false;
}
// ------------------------------------------------------------------------------------------
bool GetDefaultInt(int &value,string filename, string name,bool dbg) 
{
	string val;
	if (!GetDefaultString(val,filename,name,dbg)) return false;
		
	if(val.find("x") != string::npos )
		sscanf(val.c_str(),"%x",&value);
	else 
		sscanf(val.c_str(),"%d",&value);
	return true;
}
// ------------------------------------------------------------------------------------------
bool GetDefaultUnsigned(u_int &value,string filename, string name,bool dbg) 
{
	string val;
	if (!GetDefaultString(val,filename,name,dbg)) return false;
		
	if(val.find("x") != string::npos )
		sscanf(val.c_str(),"%x",&value);
	else 
		sscanf(val.c_str(),"%u",&value);
	return true;
}
// ------------------------------------------------------------------------------------------
bool GetDefaultFloat(float &value,string filename, string name,bool dbg) 
{
	string val;
	if (!GetDefaultString(val,filename,name,dbg)) return false;
	sscanf(val.c_str(),"%f",&value);
	return true;
}
// ------------------------------------------------------------------------------------------
string getFirstItem(string fileName)
{
static string item;
item="";
ifstream ifs(fileName.c_str());	
if( ifs ) {
  ifs >> item;	
  ifs.close();	
  }
return item;
}
// ------------------------------------------------------------------------------------------
void changeItemInFile(string fileName,string add,string remove)
{
vector <string> stringList;
ifstream ifs(fileName.c_str());	
if( ifs ) {
  string line;
  ifs >> line;	
  while (!ifs.eof()) {
    cout << line << endl;
    if (line!=remove) stringList.push_back(line);
    ifs >> line;	
    }
  ifs.close();	
  }
if (add!="") stringList.push_back(add);

ofstream ofs(fileName.c_str());
if (!ofs) cout << "cannot open file for writing !" << endl;
else {
  for (u_int i=0;i<stringList.size();i++)
    ofs << stringList[i] << endl;
  ofs.close();
  }
}

// ------------------------------------------------------------------------------------------
bool wval(unsigned int arg,string filename) {	
	ofstream ofs(filename.c_str());	
	if( !ofs ) { // open failed
		cerr << "cannot open file " << filename << endl;
		return false;
		}
	ofs << arg;	
	ofs.close();	
	return true;
	}
	
// ------------------------------------------------------------------------------------------
bool rval(unsigned int &arg,string filename) {	
	char t_str[SIZE];
	ifstream ifs(filename.c_str());	
	if( !ifs ) { // open failed
		cerr << "cannot open file " << filename << endl;
		return false;
		}
	ifs.getline(t_str,SIZE);
	
	sscanf(t_str,"%d",&arg);
	/*
	istrstream ist(t_str);
	ist >> arg;
	*/
	//cout << " read " << arg << endl;
	ifs.close();	
	return true;
	}
// ------------------------------------------------------------------------------------------
Config_V GetConfigVal(string filename,string name,bool dbg) {
  char t_str[SIZE];
  char did[10];
  char moduleType[10];
  int mod,channel;
  int  pre,post;
  int threshold,sampleRate,sampleSize,streamNumber,detectorChannel,
			fullscale,delay,thresholdSign,used;
  float offset;


  // calculate a crate number
  char crateString[25];
  strcpy(crateString,name.c_str());
  if (dbg) cout << "crate name: " << crateString;
  int length=strlen(crateString);
  if (crateString[length-1]==':') { // delete ':'
      crateString[length-1]=0;
      length--;
      }
  if (crateString[length-1]=='g') { // the hosts may have the ending 'g', e.g. 'pcntof19g' g=gigabit
      crateString[length-1]=0;
      length--;
      }
  int crate;
  crate=atoi(&crateString[length-2]);
  if ((crateString[length-4]!='p') && (crateString[length-4]!='P')) crate+=100;
  if (dbg) cout << " number: " << crate << endl;


	Channel_Config	readoutConfig;
	Config_V rconf_v;

	ifstream ifs(filename.c_str());

	if( !ifs ) { // open failed
		cerr << "cannot open file " << filename << endl;
		exit(-1);
		}

	string nameOff,detectoridOff;
   	string::size_type pos;

	while ( ifs.getline(t_str,SIZE)) {
		if(dbg) cout << "-->" << t_str << endl;
		string line = t_str;
		if (line.find_first_of("#") != (string::size_type)0 ) {
			if((pos = line.find(name)) != string::npos ) {	// fist we hunt the name identifier
				nameOff = line.substr(pos + name.size(),line.size());
				sscanf(nameOff.c_str(),"%s%d%d%d%d%d%d%d%d%s%d%d%d%d%f%d",
          did,&streamNumber,&mod,&channel,&pre,&post,
	  &threshold,&sampleRate,&sampleSize,moduleType,
          &fullscale,&detectorChannel,&delay,
          &thresholdSign,&offset,&used );
				if(dbg) printf("nameOff %s\n",nameOff.c_str());
				if(dbg) printf("got %s m: %d ch: %d pre: %d post: %d thr: %d dch: %d\n",did,mod,channel,pre,post,threshold,detectorChannel);
				memcpy(&readoutConfig.id,did,4);
				memcpy(&readoutConfig.moduleType,moduleType,4);
				readoutConfig.clockState=(readoutConfig.moduleType==ACQD_as_int) ? INTC_as_int : EXTC_as_int;
				readoutConfig.str_crate_mod_ch 	= (streamNumber<<24) + (crate<<16) + (mod << 8) + channel;
				readoutConfig.detectorChannel = detectorChannel;
				readoutConfig.fullScale =  fullscale;
				readoutConfig.threshold = threshold;
				readoutConfig.thresholdSign = thresholdSign;
				readoutConfig.delayTime = delay;
				readoutConfig.offset= offset;
				readoutConfig.pre 	= pre;
				readoutConfig.post 	= post;
 				readoutConfig.sampleRate = sampleRate;
				readoutConfig.sampleSize = sampleSize;
				if ((readoutConfig.moduleType!=NONE_as_int) &&
				    (strcmp(did,"*NO*")) &&
				    (used>0))   rconf_v.push_back( readoutConfig );
				}
      } // if
		} // while
  if (rconf_v.size()==0)
	cout << "No config data found for pc " << name.c_str() << endl;
  return rconf_v;
}

// ------------------------------------------------------------------------------------------
HVList GetHVVal(string filename) {
char t_str[SIZE];
char did[10];
HV_Config	hvConfig;
HVList hvList;

ifstream ifs(filename.c_str());
	
if( !ifs ) { // open failed
	cerr << "cannot open file " << filename << endl;
	exit(-1);
	}
	
while ( ifs.getline(t_str,SIZE)) {
  char *textStart=t_str;
  while (textStart[0]==' ') textStart++;   // skip leading blanks

	if ((textStart[0]!='#') && (textStart[0]!=0)) {
			sscanf(t_str,"%s%d%d%d%d%f%f%f%f%f",
                did,
                &hvConfig.detectorChannel,
                &hvConfig.hvCrate,
                &hvConfig.hvBoard,
                &hvConfig.hvChannel,
                &hvConfig.vSet,
                &hvConfig.vMax,
                &hvConfig.iMax,
                &hvConfig.deltaV,
                &hvConfig.deltaI);
            if (strcmp(did,"****"))                      // type = **** means: no detector specified
    			memcpy(&hvConfig.detectorType,did,4);
            else
                hvConfig.detectorType=0;
			hvList.push_back( hvConfig );				
      } // if
	} // while

if (!hvList.size()) cout << "No HV config data found in " << filename.c_str() << endl;

return hvList;
}
// ------------------------------------------------------------------------------------------
void dump(char *buf,int size) {

int tot_size=1;

	for(int x=0;x<size;x++) {
		if(!((tot_size-1)%16)) printf("0x%08x ",tot_size-1);
		printf("%02x",0xff&buf[x]);
		if(!(tot_size%4)) printf(" ");
		if(!(tot_size%16)) {
				for(int cc = x - 15;cc<=x;cc++) {
					char c = isprint(buf[cc]) ? buf[cc] : '.';
					printf("%c",c);
					}
				cout << endl;
				//exit(0);							
			}
		tot_size ++;
	}
	cout << endl << endl;
}
// ------------------------------------------------------------------------------------------
// This function checks if a given directory is mounted on the system.
// Be carefull: if /tdata is mounted, the check for /tdata/data01 will fail
// because /tdata/data01 is not listed in /etc/mtab.
// return values
// -1 : an error occured
//  0 : path not mounted
//  1 : path mounted
int queryMounted(char* aPath)
{
// remove trailing '/'
char path[256];
strncpy(path,aPath,256);
path[255]=0;
while ((strlen(path)>0) && (path[strlen(path)-1]=='/')) path[strlen(path)-1]=0;
// check if path still valid
if (strlen(path)==0) {
  printf("queryMounted: No path given!\n");
  return -1;
  };
// read mtab file
FILE* file=fopen("/etc/mtab","rb");
if (!file) {
  printf("queryMounted: Cannot read /etc/mtab!\n");
  return -1;
  }
char line[256];
while (!feof(file)) {
  fgets(line,256,file);
  line[255]=0;
  if (strstr(line,path)) {
    fclose(file);
	return 1;
    };
  }
fclose(file);
return 0;
}
// ------------------------------------------------------------------------------------------
