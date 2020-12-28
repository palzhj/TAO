
#include "ClusterTypes.h"


//********************************
//Implementation of klaus_cluster_event
//********************************
ClassImp(klaus_cluster_event);

float klaus_cluster_event::layer_offset=0;
float klaus_cluster_event::layer_spacing=20;

//GetPosition functions: map channel to position. 
//T0 is mapped to -1000,-1000 on xy-plane.
//Undefined channels mapped to -10000,-10000
float klaus_cluster_event::GetPosition(int axis){
	//printf("klaus_cluster_event::GetPosition(%d), layer=%d channel=%d\n",axis,layer,channel);
	if(axis==0){
		const float map[]={-45,45,30,0,-30,45,-45,-1000};
		//printf("--> %f\n",map[channel]);
		if(channel<8) return map[channel];
		else return -10000;
	}
	else if(axis==1){
		const float map[]={-30,-30,0,0,0,30,30,-1000};
		//printf("--> %f\n",map[channel]);
		if(channel<8) return map[channel];
		else return -10000;
	}
	else if(axis==2){
		return layer*layer_spacing + layer_offset;
	}else throw;
}

float klaus_cluster_event::GetTime(){ return AbsTime(); };

void klaus_cluster_event::DumpObject(int depth, FILE* fd){
//	fprintf(fd,"Layer %d "); this->PrintHeader(fd);
	fprintf(fd,"Layer %d ",layer); this->Print(fd);
}


//********************************
//Implementation of klaus_cluster
//********************************

ClassImp(klaus_cluster);
klaus_cluster_event::klaus_cluster_event():layer(-1){};
klaus_cluster_event::klaus_cluster_event(klaus_event& ev,short int layerID):layer(layerID),klaus_event(ev){};

//Get average position
float klaus_cluster::GetPosition(int axis){
	int n=0;
	float pos=0;
	//printf("klaus_cluster::GetPosition(%d)\n",axis);
	for(klaus_cluster::iterator it=this->begin();it!=this->end();++it){
		pos+=it->GetPosition(axis);
		n++;
	}
	//printf("--> %f\n",pos/n);
	return pos/n;
}

//Get average time (in case of overrun during cluster, add 1<<12 to these hits)
float klaus_cluster::GetTime(){
	int n=0;
	float time=0;
	for(klaus_cluster::iterator it=this->begin();it!=this->end();++it){
		float t_it=it->GetTime();
		//detect overflow
		if((time/n > 4090)&&(t_it<6)) t_it+=4096;
		if((time/n < 6)&&(t_it>4090)) t_it-=4096;
		time+=t_it;
		n++;
	}
	return time/n;
};

void klaus_cluster::DumpObject(int depth, FILE* fd){
	fprintf(fd,"  Cluster of %zu hits: <T>=%4.4f\n",this->size(),GetTime());
	if(depth>0) for(klaus_cluster::iterator it=this->begin();it!=this->end();++it){
		it->DumpObject(depth-1,fd);
	}
}

void klaus_cluster::Merge(klaus_cluster& src){
	this->splice(this->end(),src);
}

//********************************
//Implementation of klaus_clusterlist
//********************************

ClassImp(klaus_clusterlist);

void klaus_clusterlist::DumpObject(int depth, FILE* fd){
	printf("%zu Clusters\n",this->data.size());
	int c=0;
	if(depth>0) for(klaus_clusterlist::iterator it=this->data.begin();it!=this->data.end();++it){
		printf("  C#%3.3d: ",c++); it->DumpObject(depth-1,fd);
	}
}

//***********************************************
//Implementation of klaus_clusterlist_collection
//***********************************************

