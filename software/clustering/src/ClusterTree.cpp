/*File ClusterTree.cpp:
Use the clustering algorithms to build a new tree of collections of cluster from a tree of aquisitions.
Author: K. Briggl
*/

#include "TFile.h"
#include "TTree.h"

#include "EventType.h"
#include "ClusterTypes.h"
#include "Clustering.h"


int main(int argc, char* argv[]){
	if(argc<2){
		printf("Usage: %s file_basename [entry] [maxdist] [maxdepth] [output_suffix]\n",argv[0]);
		return 0;
	}
	
	int entry=-1;
	if(argc>2){
		sscanf(argv[2],"%d",&entry);
	}
	int maxdist=5;
	if(argc>3){
		sscanf(argv[3],"%d",&maxdist);
	}
	int maxdepth=5;
	if(argc>4){
		sscanf(argv[4],"%d",&maxdepth);
	}


	TFile f(Form("%s.root",argv[1]));
	TTree* tin=(TTree*) f.Get("aqu_dump");
	klaus_aquisition* aqu_in=NULL;
	tin->SetBranchAddress("aquisitions",&aqu_in);

	TFile* fout;
	if(argc>5)
		fout=new TFile(Form("%s_%s.root",argv[1],argv[5]),"RECREATE");
	else
		fout=new TFile(Form("%s_clusters.root",argv[1]),"RECREATE");

	TTree* tout=tin->CloneTree(0);
	klaus_clusterlist* res_clusters=NULL;
	tout->Branch("clusters",&res_clusters);

	if(entry<0){
		printf("Running over tree of %lld entries\n",tin->GetEntries());
		//run over tree
		for(int i=0;i<tin->GetEntries();i++){
			tin->GetEntry(i);
			printf("Entry %d of %lld \n",i,tin->GetEntries());
			klaus_clusterlist clusters=ClusterAquisition(aqu_in,maxdist,maxdepth,0);
			res_clusters=&(clusters);
			tout->Fill();
		}
		tout->Write();
	}else{
		printf("Clustering entry #Entry %d of %lld ",entry,tin->GetEntries());
		tin->GetEntry(entry);
		aqu_in->Print();
		klaus_clusterlist clusters=ClusterAquisition(aqu_in,maxdist,maxdepth,100);
		clusters.Write(Form("clusters_e%d",entry));
	}
	fout->Close();
	printf("END...\n");
}

