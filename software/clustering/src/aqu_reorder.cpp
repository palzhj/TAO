/*File aqu_reorder.C
Script to reorder aquisition objects from an input tree by T0 timestamps
Author: K. Briggl
The input data should consist of ordinary tree-data, where for every aquisition a trigger was send to all ASIC's T0 channels
The reordering is done based on the timestamps of these synchronization signals, output is a file with the same aquisition objects, where the first entry is the T0 hit.
If a T0 reference hit is missing for one or any number of ASICs, all consecutive hits will be added to the previous T0 reference hit.

run the script as :
root -l -e ".L ~/git-repos/klaus-daq/software/daq-common/klaus_storagetypes.so" aqu_reorder.C\(\"RunXX\"\) -q
where the input file has the name RunXX.root (.root is appended). The output file will have the name RunXX_reordered.root.
The path to the shared library might need adaptations
*/


#include "TFile.h"
#include "TTree.h"
#include "EventType.h"


klaus_aquisition* getRefObj(std::map<unsigned char, std::list<klaus_aquisition*>::iterator> refs,unsigned char ID){
	if(refs.find(ID)==refs.end()) return NULL;
	else return *(refs.find(ID)->second);
}

#define THIS_REF(asic) (aqu_ref.find(asic->first)!=aqu_ref.end()?aqu_ref[asic->first]:aqu_queue.end())
#define REPORT_REF(asic) ((THIS_REF(asic)!=aqu_queue.end())?THIS_REF(asic)->obj:NULL)
struct combined_aquisition{
	klaus_aquisition* obj;
	int nrefs;
	int nchecks;
	combined_aquisition(int ID){
		obj=new klaus_aquisition;
		obj->aqu_ID=ID;
		nrefs=1;
		nchecks=1;
	}
};

struct problematic_info{
	int ID_start;
	int ID_occur;
	int entry_out;
};

void aqu_reorder(const char* fname){
	TFile f(Form("%s.root",fname));
	TTree* tin=(TTree*) f.Get("aqu_dump");
	TFile fout(Form("%s_reorder.root",fname),"RECREATE");
	TTree* tout=new TTree("aqu_dump","Reordered Aquisitions");

	klaus_aquisition* aqu_in=NULL;
	klaus_aquisition* aqu_out=NULL;
	//list of combined aquisitions currently filled
	std::list<combined_aquisition> aqu_queue;
	//references to combined aquisitions in aqu_queue
	std::map<unsigned char, std::list<combined_aquisition>::iterator> aqu_ref;
	std::map<unsigned char, int> nhits;
	tin->SetBranchAddress("aquisitions",&aqu_in);
	tout->Branch("aquisitions",&aqu_out);
	unsigned long last_aqu_ID;
	int errcnt=0;
	std::list<problematic_info> problematic_aquisitions;
	int stop_at=-1;
	for(int i=0;i<tin->GetEntries();i++){
		tin->GetEntry(i);
		if(stop_at==i) return;	
		//run over ASICs
		std::map<unsigned char, std::list<klaus_event> >::iterator asic;
		for(asic=aqu_in->data.begin();asic!=aqu_in->data.end();++asic){
			//loop over hits of this
			std::list<klaus_event>::iterator hit; 
			for(hit=asic->second.begin();hit!=asic->second.end();++hit){
				if(hit->channel==7){ //time-reference hit: potentially commit old active and create new aquisition object or hook to most recent
					//finished filling previous aquisition. Remove reference, report number of hits, set-up again

					//previously referenced?
					if(THIS_REF(asic)!=aqu_queue.end()){
						printf("Aquisition %6.6ld chip %d: %3.3d hits follow | ref=(---)%p nref=%d ncheck=%d\n",aqu_in->aqu_ID,asic->first,nhits[asic->first],REPORT_REF(asic),aqu_ref[asic->first]->nrefs-1,aqu_ref[asic->first]->nchecks);
						//decrement references counter will not use it anymore after we are finished here
						aqu_ref[asic->first]->nrefs--;
						nhits[asic->first]=0;
					}

					//hook to new ref
					if( aqu_queue.empty() || (THIS_REF(asic)!=aqu_queue.end() && (THIS_REF(asic)->obj==aqu_queue.back().obj))){
						//no current reference or current reference is set and points to last in queue. Create new aquisition and reference to it
						//generate new combined aquisition, reference to it
						aqu_queue.push_back(aqu_in->aqu_ID);
						aqu_ref[asic->first]=--aqu_queue.end();

						printf("Aquisition %6.6ld chip %d: time-ref @%4.4x ",aqu_in->aqu_ID,asic->first,hit->AbsTime());
						printf(" | ref=(new)%p\n",REPORT_REF(asic));
					}else{
						//hook to existing object, aquisition after the current one
						if(THIS_REF(asic)!=aqu_queue.end())
							aqu_ref[asic->first]++;
						else
							aqu_ref[asic->first]=aqu_queue.begin();


						//hook, try later aquisitions if delta delay is too large
						int dt;
						while(aqu_ref[asic->first]!=aqu_queue.end()){
							aqu_ref[asic->first]->nrefs++;
							aqu_ref[asic->first]->nchecks++;

							dt= aqu_ref[asic->first]->obj->data.begin()->second.front().AbsTime() - hit->AbsTime();
							printf("Aquisition %6.6ld chip %d: time-ref @%4.4x ",aqu_in->aqu_ID,asic->first,hit->AbsTime());
							printf(" | ref=(hok)%p n=%d",REPORT_REF(asic),aqu_ref[asic->first]->nrefs);

							//check if timestamp fits
							if(!((TMath::Abs(dt)>2) && (TMath::Abs(dt+4096)>2))){ //matched timestamp
								printf(" | delta=%d\n",dt);
								break; //timestamp fits, can stop here
							}else{ //unmatched timestamp!
								printf(" | DELTA=%d -------------- !!!\n",dt);
								problematic_info info;
								info.ID_occur=aqu_in->aqu_ID;
								info.ID_start=aqu_ref[asic->first]->obj->aqu_ID;
								info.entry_out=tout->GetEntries()-1;
								problematic_aquisitions.push_back(info);
								errcnt++;
								//stop_at=i+10;

								//try later aquisitions, drop reference to this one
								aqu_ref[asic->first]->nrefs--;
								aqu_ref[asic->first]++;
							}
						}
					}

					if(THIS_REF(asic)==aqu_queue.end()){
						//the previous effort failed to find a suitable reference. Create a new one, a trigger got lost.
						//Be aware that any hits after the lost trigger will be associated with the previous trigger
						aqu_queue.push_back(aqu_in->aqu_ID);
						aqu_ref[asic->first]=--aqu_queue.end();
						printf("Aquisition %6.6ld chip %d: time-ref @%4.4x ",aqu_in->aqu_ID,asic->first,hit->AbsTime());
						printf(" | ref=(new)%p\n",REPORT_REF(asic));
					}

					//add hit to referenced object
					aqu_ref[asic->first]->obj->data[asic->first].push_back(*(klaus_event*)hit->Clone());
					aqu_ref[asic->first]->obj->nEvents++;
					//last_aqu_ID=aqu_in->aqu_ID;

				}else{ //hit from any channel: fill to reference
					if(THIS_REF(asic)!=aqu_queue.end()){ // has reference
						aqu_ref[asic->first]->obj->data[asic->first].push_back(*(klaus_event*)hit->Clone());
						aqu_ref[asic->first]->obj->nEvents++;

						if(nhits[asic->first]<0){
						}
						nhits[asic->first]++;
					}else{//no reference
						printf("Aquisition %6.6ld chip %d: hit ch%d  @%4.4x  | ref=!!!NONE!!!\n",aqu_in->aqu_ID,asic->first,hit->channel,hit->AbsTime());
						errcnt++;
					}
				}
			}//hits
		}//asics
		last_aqu_ID=aqu_in->aqu_ID;

		//commit aquisition if it is full
		//check if aquisition object was checked by all (a requirement for the number of triggers can also be put here)
		std::list<combined_aquisition>::iterator it=aqu_queue.begin();
		for(;it!=aqu_queue.end();it++){
			if((it->nrefs==0) && (it->nchecks==3)){//checked by all
				aqu_out=it->obj;

				printf("COMMIT of aquisition started with %6.6ld: Triggers: %lu, Left uncommitted: %lu, Times:\n",aqu_out->aqu_ID,aqu_out->data.size(),aqu_queue.size()-1);
				printf("COMMIT Times: ");
				for(std::map<unsigned char, std::list<klaus_event> >::iterator ic=aqu_out->data.begin();ic!=aqu_out->data.end();ic++){
					printf("\t%u:%4.4x",ic->first,ic->second.front().AbsTime());
				}
				printf("\n");
				tout->Fill();

				//remove from queue. Can be done if full, since no other list will reference to this object anymore
				it=--aqu_queue.erase(it);
			}
		}



	}//entries(aquisitions-in)


	//commit aquisition at end
	//check if aquisition object was checked by all (a requirement for the number of triggers can also be put here)
	std::list<combined_aquisition>::iterator it=aqu_queue.begin();
	printf("After reordering, %lu entries left in queue:\n",aqu_queue.size());
	for(;it!=aqu_queue.end();it++){
			aqu_out=it->obj;
			printf("COMMIT of aquisition started with %6.6ld: Triggers: %lu, Left uncommitted: %lu, Times:\n",aqu_out->aqu_ID,aqu_out->data.size(),aqu_queue.size()-1);
			printf("COMMIT of aquisition started with %6.6ld: Stray references: %d, Checked by: %d\n",aqu_out->aqu_ID,it->nrefs,it->nchecks);
			printf("COMMIT Times: ");
			for(std::map<unsigned char, std::list<klaus_event> >::iterator ic=aqu_out->data.begin();ic!=aqu_out->data.end();ic++){
				printf("\t%u:%4.4x",ic->first,ic->second.front().AbsTime());
			}
			printf("\n");
			tout->Fill();

			//remove from queue. Can be done if full, since no other list will reference to this object anymore
			it=--aqu_queue.erase(it);
	}



	tout->Write();
	printf("Finished reordering. In: %lld Aquisitions. Out: %lld Aquisitions. Errors: %d\n",tin->GetEntries(),tout->GetEntries(),errcnt);

	for(std::list<problematic_info>::iterator it=problematic_aquisitions.begin(); it!=problematic_aquisitions.end();it++){
		printf("Start@ #%d; Occured #%d; TreeEntry: #%d\n",it->ID_start, it->ID_occur, it->entry_out);

	}
}



int main(int argc, char* argv[]){
	if(argc<2){
		printf("Usage: %s file_basename [entry]\n",argv[0]);
		return 0;
	}
	aqu_reorder(argv[1]);
}
