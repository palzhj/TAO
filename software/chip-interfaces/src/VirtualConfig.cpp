/*
 * VirtualConfig.cpp
 *
 *  Created on: 29.09.2011
 *      Author: Konrad
 */

//#define DEBUG

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include "../include/VirtualConfig.h"
#include "../include/VirtualInterface.h"

using namespace std;



TVirtualConfig::TVirtualConfig(TVirtualInterface* Interface,
                               /*const members*/const void* pars, const short int nParameters, const short int patternlength):
    parameters((parameter*)pars),
    nParameters(nParameters),
    patternlength(patternlength),
    Iface(Interface)
{

	//if(Iface) ifaceHandID=Iface->AttachHandle(this);
	//else ifaceHandID=255;
	bitpattern_read=NULL;
	bitpattern_write=NULL;
}



TVirtualConfig::~TVirtualConfig()
{
//	fprintf(stderr,"VC destruct\n");

    //if (Iface && Iface->DetachHandle(ifaceHandID)<0)
    //{
    //    fprintf(stderr,"Iface: is Detached\n");
    //}
}




/** @brief Set Parameter in Pattern
  *
  * (documentation goes here)
  */
void TVirtualConfig::SetInPattern(char * const pattern, int N,unsigned long long const& value ) const
{
//	fprintf(stderr,"Parameter %d set to %llu\n",N,value);
    	int pos;
    	unsigned long long mask=1;
	if (parameters[N].endianess==false){
		pos=parameters[N].offset;
		while (pos<parameters[N+1].offset) //loop over bits in parameter
		{
			if ((value & mask)!=0)    //bit is set in parameter
				pattern[pos/8]|= (0x01<<(pos%8));
			else
				pattern[pos/8]&=~(0x01<<(pos%8));
			pos++;
			mask<<=1;
		}
	}else{
		pos=parameters[N+1].offset-1;
		while (pos>=parameters[N].offset) //loop over bits in parameter
		{
			if ((value & mask)!=0)    //bit is set in parameter
				pattern[pos/8]|= (0x01<<(pos%8));
			else
				pattern[pos/8]&=~(0x01<<(pos%8));
			pos--;
			mask<<=1;
		}

	}


};


/** @brief Get Parameter in Pattern
  *
  * (documentation goes here)
  */



void TVirtualConfig::GetFromPattern(char * const pattern, int N, unsigned long long &value) const
{
    unsigned long long temp=0;//temporary value read from pattern
    int pos;
	pos=parameters[N].offset;
	while (pos<parameters[N+1].offset) //loop over bits in parameter
	{
	    if ((pattern[pos/8] & ( 1<< (pos%8))) !=0 ){
		if (parameters[N].endianess==false)
			temp+=(1<<(pos-parameters[N].offset));
		else
			temp+=(1<<(parameters[N+1].offset-1-pos));
	    }
	    pos++;
	}
	value=temp;
};

short int TVirtualConfig::GetParID(const char *name) const
{
    //suche parameter-ID mit namen
    int ID=-1;
    for (int npar=0; npar<nParameters; npar++)
    {
        if(strcmp(name,parameters[npar].name)==0)
        {
            ID=npar;
            break;
        }
    }
    return ID;
}

const char* TVirtualConfig::GetParName(short int n) const
{
    if ((n>=0)&&(n<nParameters))
        return parameters[n].name;
    else	return NULL;
}

const std::string TVirtualConfig::GetParDescription(short int n) const
{
    if ((n>=0)&&(n<nParameters)){
		std::stringstream s;
		s<<"Parameter \""<<parameters[n].name<<"\":"<<std::endl<<parameters[n].description;
		return s.str();
	}
    else	return "";
}

const int TVirtualConfig::ChannelFromID(short int n) const{
	int pos=strlen(parameters[n].name);
	const char* name=parameters[n].name;
	int chan;

        while ((pos>=0)&&(name[pos]!='_')){pos--;};
        if (pos<0){
                return -1;
        }
        if(sscanf(&name[pos],"_CH%d",&chan)!=1){
                return -1;
        }
	return chan;
}

//match parameter name str against pattern
bool TVirtualConfig::Match(const char* pattern, const char* str) const{
	std::string pat=pattern;
	std::string tok;
	std::string par=str;
	bool first=true;
	//check token
	while(par.size()>0){
		tok=pat.substr(0,pat.find_first_of("*"));
		if(!first){
			if(tok.length()==0){
				return true;
			}
			par.erase(0,par.find(tok));
		}
		if(tok.compare(0,tok.length(),par,0,tok.length())==0){
			par.erase(0,tok.length());
			pat.erase(0,tok.length()+1);
			first=false;
		}else{
			return false;
		}
	}
	return true;
}


//return a list of parameters matching pattern, only wildcarts can be used there, e.g. channelname/* or parname/*
std::list<short int> TVirtualConfig::MatchedParamSet(const char* pattern) const{
	std::list<short int> res;
	//try to match all parameters
	for (int npar=0; npar<nParameters; npar++){
		if(Match(pattern,GetParName(npar))) res.push_back(npar);
	}
	return res;
}

std::list<short int> TVirtualConfig::SectionParamSet(const char* prefix) const{
	std::list<short int> res;
	for (int npar=0; npar<nParameters; npar++){
		if(strstr(GetParName(npar),prefix)==0) res.push_back(npar);
	}
	return res;
}

const bool TVirtualConfig::ParIsFiller(short int n) const{
//checks if the current parameter ID refers to a filler with no connections in the ASIC.
//returns true if first character is a '_'
	return GetParName(n)[0]=='_';
}

int TVirtualConfig::ValidatePattern(){
    /*check bitpattern*/
    //take care of the MSB wich might not be aligned
	unsigned char bitmask= (0xff<< ((GetPatternBitLength()-1)%8+1));
	bitmask = ~bitmask;
	#ifdef DEBUG
	printf("bitmask for MSB: 0x%02x, ofs: %i\n", bitmask, GetPatternBitLength()-1);
	#endif
    int nch=GetPatternByteLength()-1;
    int returnval=0;
    if( (bitmask&bitpattern_write[nch]) != (bitmask&bitpattern_read[nch])){
		    printf("ValidatePattern(HandleID=%d): Difference found for byte %d: %02X recv: %02X \n",ifaceHandID,nch,bitmask&bitpattern_write[nch],bitmask&bitpattern_read[nch]);
		    returnval -= 10;
    }

    for (int nch=GetPatternByteLength()-2; nch>=0; nch--)
    {
	    if( (0xff&bitpattern_write[nch]) != (0xff&bitpattern_read[nch])){
		    printf("ValidatePattern(HandleID=%d): Difference found for byte %d: %02X recv: %02X \n",ifaceHandID,nch,0xff&bitpattern_write[nch],0xff&bitpattern_read[nch]);
		    returnval -= 10;
	    }
    }
    return returnval;
}

int TVirtualConfig::UpdateConfig()
{
    int status;
    int nch;
#ifdef DEBUG
    printf("Bitpattern wr: ");
    for (nch=GetPatternByteLength()-1; nch>=0; nch--)
    {
        printf("%02X ",0xFF&bitpattern_write[nch]);
    }
    printf("\n");
#endif
    status =IssueCommand('c', GetPatternByteLength(),bitpattern_write,  GetPatternByteLength(),bitpattern_read);
    if (status<0)
	return status;

#ifdef DEBUG
    printf("Bitpattern rd: ");
    for (nch=GetPatternByteLength()-1; nch>=0; nch--)
    {
        printf("%02X ",0xFF&bitpattern_read[nch]);
    }
    printf("\n");
#endif

	return ValidatePattern();
}


int TVirtualConfig::IssueCommand(const char cmd,  int len, char* data, int reply_len, char* reply){
        writebitcode("./test.txt");
	//if(Iface!=NULL) return Iface->Command(ifaceHandID,cmd,len,data,reply_len,reply,this);
	//else{
//		printf("TVirtualConfig::IssueCommand(): No interface bound.\n");
//		return -1;
//	}
}


/** @brief Get Parameter (read)
  * Return: number of parameter, -1 if not in range/found/RW-error
  */
int TVirtualConfig::GetParValueRD(const char *name, unsigned long long &value)
{
    //printf("TVirtualConfig::GetParValueRD(%s)\n",name);
    return GetParValueRD(GetParID(name), value);
}
/** @brief Get Parameter (read)
  * Return: number of parameter, -1 if not in range/found/RW-error
  */
int TVirtualConfig::GetParValueRD(short int npar, unsigned long long &value)
{
    if ( (npar<0) | (npar>=nParameters) )
        return -1;
    //printf("TVirtualConfig::GetParValueRD(%d)\n",npar);
    GetFromPattern(bitpattern_read, npar, value);
    return npar;
}



/** @brief Get Parameter (to be written)
  * Return: number of parameter, -1 if not in range/found
  */
int TVirtualConfig::GetParValueWR(short int npar, unsigned long long &value)
{
    if ( (npar<0) | (npar>=nParameters) )
        return -1;
    //printf("TVirtualConfig::GetParValueWR(%d)\n",npar);
    GetFromPattern(bitpattern_write, npar, value);
    return npar;
}



/** @brief Get Parameter (to be written)
  * Return: number of parameter, -1 if not in range/found
  */
int TVirtualConfig::GetParValueWR(const char *name, unsigned long long &value)
{
    //printf("TVirtualConfig::GetParValueWR(%s)\n",name);
    return GetParValueWR(GetParID(name), value);
}





int TVirtualConfig::SetParValue(const char *name, unsigned long long value)
{
    //printf("TVirtualConfig::SetParValue(%s)\n",name);
    return SetParValue(GetParID(name), value);
}


int TVirtualConfig::SetParValue(short int npar,unsigned long long value)
{
    if ( (npar<0) | (npar>=nParameters) )
        return -1;
    //printf("TVirtualConfig::SetParValue(%d)\n",npar);
    SetInPattern(bitpattern_write,npar,value);

    return npar;
}



int TVirtualConfig::ReadFromFile(std::istream& istream){

    short int nRead=0; //Anzahl gelesener Parameter
    char parname[MAX_PARLENGTH];
    unsigned long long value;
    string line;
    while (getline(istream,line,'\n').good())
    {
	if(line.c_str()[0]=='#')
		continue;
        if(sscanf(line.c_str(),"%s = %llX",parname,&value)!=2)
        {
            fprintf(stderr,"TVirtualConfig::ReadFromFile() - Error in syntax.\n\t Line: %s\n",line.c_str());
            continue;
        }
        if (SetParValue(parname,value)>=0)
            nRead++;
        else
        {
            fprintf(stderr,"TVirtualConfig::ReadFromFile() - Parameter \"%s\" not recognized.\n",parname);
        }

#ifdef DEBUG
        fprintf(stderr,"\t parname %s value=%llX\n%s\n",parname,value,line.c_str());
#endif
    }
    return 1;
}

int TVirtualConfig::ReadFromFile(const char *fname)
{
    if (fname==NULL)
	return 0;
    ifstream ifile(fname);
    if (!ifile.is_open())
    {
        fprintf(stderr,"Configuration File \"%s\" could not be opened. Leaving Standard Values.\n",fname);
        return -1;
    }
    ReadFromFile(ifile);
    return 1;
}
void TVirtualConfig::PrintPattern() const
{
    for (int npar=0; npar<nParameters; npar++)
	printf("#%d: %s [%d:%d]\n",npar,GetParName(npar),parameters[npar].offset,parameters[npar+1].offset-1);
}

//Print to some ostream
void TVirtualConfig::Print(bool batch, ostream& ostream){
    unsigned long long value_rd,value_wr;
    if(!batch)
    {
        ostream<<"Configuration values: \n\n";
        ostream<<"parname\t sent\tread\n";
    }
    for (int npar=0; npar<nParameters; npar++)
    {
	//check if current parameter is a filler
	if(ParIsFiller(npar)){
#ifndef DEBUG
		continue;
#else
		ostream<<"#--filler:\n";
#endif
	}

        GetParValueRD(npar,value_rd);
        GetParValueWR(npar,value_wr);
        if(batch)
            ostream<<GetParName(npar)<<" = "<<std::hex<<value_wr<<std::endl;
        else
            ostream<<GetParName(npar)<<" =\t"<<std::hex<<value_wr<<"\t"<<std::hex<<value_rd<<std::endl;
    }
    
}
//Print to file
void TVirtualConfig::Print(bool batch,const char* ofilename)
{

	std::ostream* ofile; 
	if (ofilename!=NULL){
  		ofile=new ofstream(ofilename);
		if (!((std::ofstream*)ofile)->is_open())
		{
			fprintf(stderr,"Configuration File \"%s\" could not be opened for writing.\n",ofilename);
			return;
		}
	}
	else
		ofile=&std::cout;
	Print(batch,*ofile);

	if (ofilename!=NULL){
		((std::ofstream*)ofile)->close();
		delete ofile;
	}
}

void TVirtualConfig::writebitcode(const char* ofilename)
{

        std::ostream* ofile;
        if (ofilename!=NULL){
                ofile=new ofstream(ofilename);
                if (!((std::ofstream*)ofile)->is_open())
                {
                        fprintf(stderr,"Bit coding configuration File \"%s\" could not be opened for writing.\n",ofilename);
                        return;
                }
        }
        else
                ofile=&std::cout;

	int len = GetPatternByteLength();

        for (int nch=len-1; nch>=0; nch--)
        {
            //(*ofile) << hex << (0xff & (((const char*)(bitpattern_write))[0?len-nch-1:nch]));
            (*ofile)<<change(bitpattern_write[nch]);
        }
        (*ofile)<<std::endl;

        if (ofilename!=NULL){
                ((std::ofstream*)ofile)->close();
                delete ofile;
        }
}

std::string TVirtualConfig::change(char c)
{
    std::string data;
    for(int i=0;i<8;i++)
    {
        if ( ( c >>(i-1) ) & 0x01 == 1 )
        {
          data+="1";
        }
        else
        {
          data+="0";
        }
     }

     for(int a=1;a<5;a++)
     {
       char x=data[a];
       data[a]=data[8-a];
       data[8-a]=x;
     }

    return data;
}
