/*
 Beschreibungsklasse fuer Interface mit einem SiPM-Chip (KlAus, StiC)
 rein virtuell, Bitpattern einlese, auslese je nach typ
 */

#ifndef __VIRTUAL_CONFIG_H
#define __VIRTUAL_CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>

#include "VirtualInterface.h"

#define MAX_PARLENGTH 50

class TVirtualConfig
{

protected:
    struct parameter
    {
        const char *name;
        int offset;
	bool endianess; //0: little endian
			//1: big endian
	const char *description;
        parameter(const char * const n,int o,int e):name(n),offset(o),endianess(e),description(""){};
        parameter(const char * const n,int o,int e,const char* desc):name(n),offset(o),endianess(e),description(desc){};
    };

    //static data members
    parameter * parameters;
    const short int nParameters;
    const short int patternlength;	//Length of the bit pattern in bytes
    //pattern read / to write
    char *bitpattern_read;
    char *bitpattern_write;


private:

    int fChipID;
//verbindung zu bitpattern
    void GetFromPattern(char * const pattern , int N,unsigned long long& value) const;
    void SetInPattern(char * const pattern , int N,unsigned long long const& value) const;
    TVirtualInterface *Iface;

protected:
    //Interface
    unsigned int ifaceHandID;

    //instanziieren mit der richtigen anzahl an parametern, *_conf sind erzeugt.
    //ist protected um instanziieren zu verhindern.
    TVirtualConfig(TVirtualInterface* Interface,
                   const void* pars, const short int nParameters, const short int patternlength);


public:

    virtual ~TVirtualConfig();

//Interface Access & Validation
///////////////////////////
    virtual int UpdateConfig();
    /*
	Possible returns:
	0:  all good
	-1: communication timeout
	-2: communication error
	<=-10:  command ok, but returned bitpattern not correct

    */
    virtual int ValidatePattern();
    /*
	check bitpattern: send and received the same?
	0:  all good
	<=-10:  returned bitpattern not correct: - 10*{NUMBER_OF_WRONG_BYTES}
    */

    virtual int IssueCommand(const char cmd,  int len, char* data, int reply_len=-1, char* reply=0);
    /*
	Connection to interface command method, to be used by derived classes to access the interface
    */

    std::string change(char c);
    void writebitcode(char* ofilename=NULL);

    void SetChipID(int i);

//Pattern Access
///////////////////////////
    virtual short int GetPatternBitLength()const {return parameters[nParameters].offset;}; //get length of pattern in bits
    virtual short int GetPatternByteLength()const {return patternlength;}; //get length of pattern in bytes
    virtual char* GetPatternRD() const{return bitpattern_read;};
    virtual char* GetPatternWR() const{return bitpattern_write;};
//Parameter finding
///////////////////////////
    const char* GetParName(short int n) const;
    const char* GetParTooltip(short int n) const;
    const std::string GetParDescription(short int n) const;
    const short int GetNParameters()const {return nParameters;};
    short int GetParID(const char* name) const;

    const int ChannelFromID(short int n) const;

    bool Match(const char* pattern, const char* str) const;
    std::list<short int> MatchedParamSet(const char* pattern) const;
    std::list<short int> SectionParamSet(const char* prefix) const;


    /** @brief Check if Parameter is filler w.o. connection in the ASIC
      * Return: true if first char of parameter is '_'
      */
    const bool ParIsFiller(short int n) const;


//Parameter Setters+Getters
///////////////////////////


    /** @brief Get Parameter (read)
      * Return: number of parameter, -1 if not in range/found
      */
    virtual int GetParValueRD(short int npar,unsigned long long &value);
    /** @brief Get Parameter (read)
      * Return: number of parameter, -1 if not in range/found
      */
    int GetParValueRD(const char *name,unsigned long long &value);

    /** @brief Get Parameter (to be written)
      * Return: number of parameter, -1 if not in range/found
      */
    virtual int GetParValueWR(short int npar,unsigned long long &value);
    /** @brief Get Parameter (to be written)
      * Return: number of parameter, -1 if not in range/found
      */
    int GetParValueWR(const char *name,unsigned long long &value);




    /** @brief Set Parameter (read)
      * Return: number of parameter, -1 if not in range/found
      */
    virtual int SetParValue(short int npar,unsigned long long value);
    /** @brief Set Parameter in current_conf (read)
      * Return: number of parameter, -1 if not in range/found
      */
    int SetParValue(const char *name,unsigned long long value);

    //lese datei, schreibe in new_conf
    int ReadFromFile(std::istream& istream);
    int ReadFromFile(const char *fname);

    virtual void Print(bool batch,std::ostream& ostream);
    virtual void Print(bool batch,const char* ofilename=NULL);

    void PrintPattern() const;
};

#endif
