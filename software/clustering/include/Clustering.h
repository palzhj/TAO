#ifndef CLUSTERING_H__
#define CLUSTERING_H__

/*File Clustering.h:
Methods to build time-clusters of hits within one aquisition cycle (klaus_aquisition object)
Author: K. Briggl
*/

#include "TMath.h"
#include "TTree.h"
#include "TFile.h"
#include <assert.h>

#include "ClusterTypes.h"

//helper types used for merging
//list of layers
typedef std::list<unsigned char>						layer_list_t;
//collection of uniquely identified cluster sets for merging individual layers
class clusters_collection_t : public std::map<int           /*ID*/, klaus_clusterlist /*collection of clusters*/> {
public:
	void DumpObject(int depth, FILE* fd=stdout);
};

//cluster hits of aquisition for each layer individually
int ClusterLayers(klaus_aquisition* aqu_in, clusters_collection_t& clusters, int maxdist, int maxdepth, layer_list_t* layerIDs_found=NULL);

//combined pair of previously clustered set of clusterlists (set of asics/layers)
int ClusterLayerPair(clusters_collection_t& clusters_in, int merged_set_ID,int maxdist, int maxdepth);

//Clustering of everything! First cluster layers, then cluster pairs of layers until only one list of clusters remains
klaus_clusterlist ClusterAquisition(klaus_aquisition* aqu_in, int maxdist, int maxdepth, int loglevel=0);


#endif
