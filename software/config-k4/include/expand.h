#ifndef _EXPAND_H__
#define _EXPAND_H__


#include <stdio.h>
#include "string.h"
#include <set>

class PattExp{
public:
	class copyentr{
	public:
		copyentr(int chi=0,int cha=0):chip(chi),channel(cha){};
		bool operator() (const copyentr& lhs, const copyentr& rhs){
			if(lhs.chip==rhs.chip)
				return lhs.channel < rhs.channel;
			else	return lhs.chip<rhs.chip;
		}
		int chip;
		int channel;
	};

	typedef std::set<copyentr,copyentr> copyset;


private:

	static int subpat(char* str ,int s, int e, int curr_chip, copyset &cset){
		int cstart;
		int cend;
		int ccnt;
		copyentr entry(curr_chip,0);
		if(sscanf(&str[s],"%d-%d%n",&cstart,&cend,&ccnt)==2){//from-to pattern
			if( (ccnt!=e-s) || (cstart>cend) )
				return -1;
			for (int c=cstart;c<=cend;c++){
				entry.channel=c;
				cset.insert(entry);
			}
			return 2;
		}else 
		if(sscanf(&str[s],"%d%n",&cstart,&ccnt)==1){//single channel pattern
			if( ccnt!=e-s )
				return -1;
			entry.channel=cstart;
			cset.insert(entry);
			return 1;
		}else
		return -1;
	}

public:
	/*
	expand a pattern desribing a range of chip&channels to a set
	e.g. 0:1-12,1:2,3 will be expanded to chip0{1,2,..,12} and chip1{2,3}
	*/
	static int Expand(char* s,copyset &cset,  int current_chip=0 ){
		size_t last=-1;
		size_t curr=-1;
		size_t pos=0;
		while ( pos < strlen(s) ){
			if(s[pos] == ',' || s[pos] == ':'){//next delimiter
				last=curr+1;
				curr=pos;
				if (s[curr] == ','){
					if(subpat(s,last, curr,current_chip,cset)<0)
						return -last;
				}
				if (s[curr] == ':'){
					if (sscanf(&s[last],"%d:",&current_chip)!=1){
						return -last;
					}else{
					}
				}
			}
			pos++;
		}
		if(subpat(s, curr+1,strlen(s),current_chip,cset)<0)
			return -(curr+1);
		else return 1;
	}
};

#endif


