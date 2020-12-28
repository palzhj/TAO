/*File Clustering.cpp:
Methods to build time-clusters of hits within one aquisition cycle (klaus_aquisition object)
Author: K. Briggl
*/

#include "TMath.h"
#include <assert.h>

#include "ClusterTypes.h"
#include "Clustering.h"

//#define DEBUG_MERGE0
//#define DEBUG_MERGE1


//helper types used for merging
//collection of uniquely identified cluster sets for merging individual layers
void clusters_collection_t::DumpObject(int depth, FILE* fd){
	fprintf(fd,"Collection of %zu Cluster sets.\n",this->size());
	if(depth>0) for(clusters_collection_t::iterator itcoll=this->begin();itcoll!=this->end();++itcoll){
		fprintf(fd," CollID=%d : ",itcoll->first);
		itcoll->second.DumpObject(depth-1,fd);
	}
}



//cluster event by layer
int ClusterLayers(klaus_aquisition* aqu_in, clusters_collection_t& clusters, int maxdist, int maxdepth, layer_list_t* layerIDs_found){
	int n=0;
	if(layerIDs_found!=NULL) layerIDs_found->clear();
	for (std::map<unsigned char, std::list<klaus_event> >::iterator it=aqu_in->data.begin();it!=aqu_in->data.end();it++){
		if(layerIDs_found!=NULL) layerIDs_found->push_back(it->first);

		std::list<klaus_event>::iterator istart=it->second.begin();
		while(istart!=it->second.end()){

			//create empty cluster object
			clusters[it->first].data.emplace_back();
			n--;
#ifdef DEBUG_MERGE0
			printf("A%d, C#%d: New cluster created\n",it->first,clusters[it->first].size());
#endif

			//try forward: find hits for current cluster, searching for hits up to maxdepth forward in the list, starting from start.
			//if found, move hits from aquisition to cluster
			int nf=0;
			std::list<klaus_event>::iterator iwalk=istart;
			while(iwalk!=it->second.end() && nf<=maxdepth){
				//check current hit *iwalk, compare with current cluster
				if(
					(clusters[it->first].data.back().size()==0) //empty cluster
				      ||(TMath::Abs(iwalk->AbsTime()-clusters[it->first].data.back().GetTime())<maxdist) //hit fits current cluster
				){
#ifdef DEBUG_MERGE0
					printf("A%d, C#%d: Add hit to cluster, forward=%d\n",it->first,clusters[it->first].size(),nf);
#endif
					// Fill, no increment of depth-counter
					clusters[it->first].data.back().emplace_back(*iwalk,it->first);
					//erase hit from input list
					if(istart!=iwalk){ //walker not at start, can erase hit from input list
						iwalk=it->second.erase(iwalk);
					}else{ //walker at start, also update the start iterator (would become invalid)
						istart=iwalk=it->second.erase(iwalk);
					}
					n++;

				}else{ //hit does not fit current cluster, check next (increment depth-counter)
					iwalk++;
					nf++;
				}
			}
			//finished searching hits for this cluster, next run
		}
#ifdef DEBUG_MERGE0
	clusters.DumpObject(3);
#endif
	}
	return n;
}


int ClusterLayerPair(clusters_collection_t& clusterlists_in, int merged_set_ID,int maxdist, int maxdepth){
	int nmerged=0;
	//try further clustering between a pair of layers, pick the first two of the collection
	//the merged pair of clusters in the input set are removed from the clusters object when finished, 
	//the resulting merged set of clusters is appended to clusters[merged_set_ID] (or overwritten after merging if the ID already existed)
	//start with iterators in the front of the cluster sets.
	//At each step, check the time differences between the clusters. If exceeded, continue with next
	
	//temporary storage of the merged result
	klaus_clusterlist clusterlist_out; 

	//iterators to walk along the pair of cluster sets
	int IDs[2];
	//iterators on clusterlist: point to cluster
	klaus_clusterlist::iterator walk_start[2];
	klaus_clusterlist::iterator walk_runner[2];
	klaus_clusterlist::iterator walk_end[2];

	//check if there are cluster sets to merge
	if(clusterlists_in.size()<1){
		printf("Collection size<1, nothing to do...\n");
		return 0;
	}
	if(clusterlists_in.size()==1){
		//move to merged_set_ID. Very inefficient, but no need to deal with this now
		clusterlist_out=clusterlists_in.begin()->second;
		clusterlists_in.clear();
		clusterlists_in[merged_set_ID]=clusterlist_out;
		printf("Collection size==1, nothing to do...\n");
		return 0;
	}

	//select the first available pair for the search
	IDs[0]=(clusterlists_in.begin())->first;
	IDs[1]=(++clusterlists_in.begin())->first;
	walk_start[0]=clusterlists_in[IDs[0]].data.begin();
	walk_start[1]=clusterlists_in[IDs[1]].data.begin();
	walk_end[0]=clusterlists_in[IDs[0]].data.end();
	walk_end[1]=clusterlists_in[IDs[1]].data.end();

//	printf("==Merging  layers %d and %d ===============================\n",IDs[0],IDs[1]);

	while(walk_start[0]!=walk_end[0] || walk_start[1]!=walk_end[1]){
	//while(walk_start[0]!=walk_end[0] && walk_start[1]!=walk_end[1]){
		walk_runner[0]=walk_start[0];
		walk_runner[1]=walk_start[1];
		//walk along A, find first hit at B position
		int nA=0;
		int nB=0;
#ifdef DEBUG_MERGE1
		printf("---------------\n");
		if(walk_runner[0]!=walk_end[0]) 
			printf("A: %zu Entries, T=%d\n",walk_runner[0]->size(),GetClusterTime(walk_runner[0]));
		else
			printf("A: end\n");

		if(walk_runner[1]!=walk_end[1]) 
			printf("B: %zu Entries, T=%d\n",walk_runner[1]->size(),GetClusterTime(walk_runner[1]));
		else
			printf("B: end\n");
		printf("--------\n");
#endif
		//walk along A, keep B at current position. See if we find a candidate in A
		while((nA<=maxdepth) && walk_start[1]!=walk_end[1]){
			if(walk_runner[0]==walk_end[0]){
				nA=maxdepth+1;
				break;
			}
			int dt=walk_runner[0]->GetTime()-walk_start[1]->GetTime();
			if(TMath::Abs(dt)<maxdist) break;
			walk_runner[0]++;
			nA++;
		}
		//walk along B, keep A at current position. See if we find a candidate in B
		while((nB<=maxdepth)&& walk_start[0]!=walk_end[0]){
			if(walk_runner[1]==walk_end[1]){
				nB=maxdepth+1;
				break;
			}
			int dt=walk_runner[1]->GetTime()-walk_start[0]->GetTime();
			if(TMath::Abs(dt)<maxdist) break;
			walk_runner[1]++;
			nB++;
		}

		//found any reasonable hit for A/B?
		//which one is better?
#ifdef DEBUG_MERGE1
		printf("A: %d, B: %d\n",nA,nB);
#endif
		if(nA==nB){ //direct fit!
			if(nA>0){
				//No candidate for A nor B, put both to a separate cluster in the output collection
#ifdef DEBUG_MERGE0
				printf("A|B: %3.3x %3.3x\n",walk_start[0]->GetTime(),walk_start[1]->GetTime());
#endif
				clusterlist_out.data.emplace_back(*walk_start[0]);
				clusterlist_out.data.emplace_back(*walk_start[1]);
			}else{
				//A and B match, put both to common cluster in the output collection
#ifdef DEBUG_MERGE0
				printf("A+B: %3.3x %3.3x\n",walk_start[0]->GetTime(),walk_start[1]->GetTime());
#endif
				//add A (easy,because it does not exist anyway)
				clusterlist_out.data.emplace_back(*walk_start[0]);
				//append B by std::list::merge (removes the cluster from the *walk_start[1] list)
				clusterlist_out.data.back().Merge(*walk_start[1]);
				nmerged++;
			}
			if(walk_start[0]!=walk_end[0]) walk_start[0]++;
			if(walk_start[1]!=walk_end[1]) walk_start[1]++;
			continue;
		}
		if(nA<nB){
			//No candidate for A, put this to the output collection
#ifdef DEBUG_MERGE0
			printf("A  : %3.3x\n",walk_start[0]->GetTime());
#endif
			clusterlist_out.data.emplace_back(*walk_start[0]);

			if(walk_start[0]!=walk_end[0]) walk_start[0]++;
		}
		if(nA>nB){
			//No candidate for B, put this to the output collection
#ifdef DEBUG_MERGE0
			printf("B  : %3.3x\n",walk_start[1]->GetTime());
#endif
			clusterlist_out.data.emplace_back(*walk_start[1]);

			if(walk_start[1]!=walk_end[1]) walk_start[1]++;
		}
	}

	//merging is finished, now remove the merged sets and add the result set
	clusterlists_in.erase(clusterlists_in.find(IDs[0]));
	clusterlists_in.erase(clusterlists_in.find(IDs[1]));
	clusterlists_in[merged_set_ID]=clusterlist_out;
	return nmerged;
}


klaus_clusterlist ClusterAquisition(klaus_aquisition* aqu_in, int maxdist, int maxdepth, int loglevel){
	layer_list_t	layers;
	clusters_collection_t clusters_coll;

	int combined=ClusterLayers(aqu_in,clusters_coll,maxdist,0,&layers);
	if(loglevel>0) printf("after merging individual -> %d combined from %zu layers\n",combined,layers.size());
	//first pair
	combined=ClusterLayerPair(clusters_coll,-1,maxdist,maxdepth);
	if(loglevel>0) printf("after merging iteration : %d combined\n",combined);
	//second pair
	combined=ClusterLayerPair(clusters_coll,-1,maxdist,maxdepth);
	if(loglevel>0) printf("after merging iteration : %d combined\n",combined);

	//clusters_coll.DumpObject(loglevel-1);
	assert(clusters_coll.size()==1);
	if(loglevel>0) printf(" -> %zu clusters\n",clusters_coll.begin()->second.data.size());
	return clusters_coll[-1];
}


