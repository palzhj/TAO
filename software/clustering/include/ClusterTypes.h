/*
	File ClusterTypes.h:
	Definitions of types to cluster klaus events:
	basic_cluster_object: defines interface for printing & getting basic observables (time,coordinate)
	klaus_cluster_event: inherited from klaus_event, extends the layer information
	klaus_cluster	    : collection of klaus_cluster_events
	klaus_clusterlist  : collection of clusters

	All classes derive from basic_cluster_object
*/

#ifndef CLUSTER_TYPES_H__
#define CLUSTER_TYPES_H__

#include <map>
#include <list>
#include <vector>

#include "EventType.h"

//Class definition of a KLauS4 hit within the system, consisting ov several layers.
//Adds layer information to the event
class klaus_cluster_event: public klaus_event{
public:
	//constructors: from basic layer event + layerID
	klaus_cluster_event();
	klaus_cluster_event(klaus_event& ev,short int layerID=-1);

	//members
	short int layer;

	//static members
	static float layer_spacing; //!
	static float layer_offset; //!

	//member functions
	//Get Position of hit in mm.
	// channel 3 is at 0,0
	// z-axis is calculated as layerID*klaus_cluster_event::layer_spacing + klaus_cluster_event::layer_offset
	float GetPosition(int axis=0); //Axis starting from 0==X

	float GetTime(); //returns AbsTime()

	void DumpObject(int depth, FILE* fd=stdout);

	ClassDef(klaus_cluster_event,1);
};

//KLauS4 cluster type: collection of hits with layer information, same observable interface using basic_cluster_object
class klaus_cluster: public std::list<klaus_cluster_event>{
public:
	float GetPosition(int axis=0); //Get average position of this

	float GetTime(); //Get average time (in case of overrun during cluster, add 1<<12 to these hits)

	void DumpObject(int depth, FILE* fd=stdout);
	
	//(no copy) move hits of cluster src to this, appending at the end.
	//Iterators pointing to hits of src will be kept valid, but point to this.
	//src will be empty after
	void Merge(klaus_cluster& src);

	ClassDef(klaus_cluster,1);
};


//KLauS4 cluster list type: collection of clusters, e.g. the clusters collected within one readout cycle
class klaus_clusterlist: public TObject{
public:
	typedef std::list<klaus_cluster>::iterator iterator;
	std::list<klaus_cluster> data;

	void DumpObject(int depth, FILE* fd=stdout);	

	ClassDef(klaus_clusterlist,1);
};

#endif
