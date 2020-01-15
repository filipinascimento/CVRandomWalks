//
//  CVRandomRewiring.c
//  CVNetwork
//
//  Created by Filipi Nascimento Silva on 10/30/16.
//  Copyright © 2016 Filipi Nascimento Silva. All rights reserved.
//

#include "CVRandomRewiring.h"


CVNetworkRewireEntry CVNetworkRandomRewireEntry(const CVNetwork* theNetwork){
	CVSize verticesCount = theNetwork->verticesCount;
	CVSize edgesCount = theNetwork->edgesCount;
	CVIndex selectedEdge = CVRandomInRange(0, edgesCount);
	CVIndex currentFrom = theNetwork->edgeFromList[selectedEdge];
	CVIndex currentTo = theNetwork->edgeToList[selectedEdge];
	CVIndex newFrom = 0;
	CVIndex newTo = 0;
	do{
		newFrom = CVRandomInRange(0, verticesCount);
		newTo = CVRandomInRange(0, verticesCount);
	}while(CVUnlikely(newFrom==newTo)||
		   CVUnlikely(newFrom==currentFrom && newTo==currentTo)||
		   CVUnlikely(CVNetworkAreAdjacent(theNetwork, newFrom, newTo))
		   );
	CVNetworkRewireEntry entry;
	entry.edgeIndex = selectedEdge;
	entry.previousTo = currentTo;
	entry.previousFrom = currentFrom;
	entry.newFrom = newFrom;
	entry.newTo = newTo;
	return entry;
}

void CVNetworkDoRewire(CVNetwork* theNetwork, CVNetworkRewireEntry entry){
	CVIndex currentFrom = theNetwork->edgeFromList[entry.edgeIndex];
	CVIndex currentTo = theNetwork->edgeToList[entry.edgeIndex];
	
	//Removing edges from adjacency lists
	CVIndex* fromEdgesIndices = theNetwork->vertexEdgesIndices[currentFrom];
	CVIndex* fromEdgesToList = theNetwork->vertexEdgesLists[currentFrom];
	CVSize fromEdgesCount = theNetwork->vertexNumOfEdges[currentFrom];
	CVIndex fromNeighIndex = CVUIntegerMAX;
	for (fromNeighIndex=0; fromNeighIndex<fromEdgesCount; fromNeighIndex++) {
		if(CVUnlikely(fromEdgesIndices[fromNeighIndex]==entry.edgeIndex)){
			break;
		}
	}
	
	if(fromNeighIndex<fromEdgesCount-1){
		memmove(fromEdgesIndices+fromNeighIndex, fromEdgesIndices+fromNeighIndex+1, sizeof(CVIndex)*(fromEdgesCount-fromNeighIndex-1));
		memmove(fromEdgesToList+fromNeighIndex, fromEdgesToList+fromNeighIndex+1, sizeof(CVIndex)*(fromEdgesCount-fromNeighIndex-1));
	}
	
	if(fromEdgesCount>0){
		theNetwork->vertexNumOfEdges[currentFrom]--;
	}
	
	if(theNetwork->directed){
		if(!theNetwork->directed){
			//If not directed, also remove its reflection from the In adjacency lists
			CVIndex* toEdgesIndices = theNetwork->vertexInEdgesIndices[currentTo];
			CVIndex* toEdgesToList = theNetwork->vertexInEdgesLists[currentTo];
			CVSize toEdgesCount = theNetwork->vertexNumOfInEdges[currentTo];
			CVIndex toNeighIndex = CVUIntegerMAX;
			for (toNeighIndex=0; toNeighIndex<toEdgesCount; toNeighIndex++) {
				if(CVUnlikely(toEdgesIndices[toNeighIndex]==entry.edgeIndex)){
					break;
				}
			}
			
			if(toNeighIndex<toEdgesCount-1){
				memmove(toEdgesIndices+toNeighIndex, toEdgesIndices+toNeighIndex+1, sizeof(CVIndex)*(toEdgesCount-toNeighIndex-1));
				memmove(toEdgesToList+toNeighIndex, toEdgesToList+toNeighIndex+1, sizeof(CVIndex)*(toEdgesCount-toNeighIndex-1));
			}
			if(toEdgesCount>0){
				theNetwork->vertexNumOfInEdges[currentTo]--;
			}
		}
	}else{
		//Aalso remove its reflection from the adjacency lists
		CVIndex* toEdgesIndices = theNetwork->vertexEdgesIndices[currentTo];
		CVIndex* toEdgesToList = theNetwork->vertexEdgesLists[currentTo];
		CVSize toEdgesCount = theNetwork->vertexNumOfEdges[currentTo];
		CVIndex toNeighIndex = CVUIntegerMAX;
		for (toNeighIndex=0; toNeighIndex<toEdgesCount; toNeighIndex++) {
			if(CVUnlikely(toEdgesIndices[toNeighIndex]==entry.edgeIndex)){
				break;
			}
		}
		
		if(toNeighIndex<toEdgesCount-1){
			memmove(toEdgesIndices+toNeighIndex, toEdgesIndices+toNeighIndex+1, sizeof(CVIndex)*(toEdgesCount-toNeighIndex-1));
			memmove(toEdgesToList+toNeighIndex, toEdgesToList+toNeighIndex+1, sizeof(CVIndex)*(toEdgesCount-toNeighIndex-1));
		}
		if(toEdgesCount>0){
			theNetwork->vertexNumOfEdges[currentTo]--;
		}
	}
	
	
	//Adding the new edges to the adjacency lists
	CVNetworkGrowVertexSetEdgeForVertex(theNetwork,entry.edgeIndex,entry.newFrom,entry.newTo);
	if(theNetwork->directed){
		CVNetworkGrowVertexSetInEdgeForVertex(theNetwork,entry.edgeIndex,entry.newTo,entry.newFrom);
	}else{
		CVNetworkGrowVertexSetEdgeForVertex(theNetwork,entry.edgeIndex,entry.newTo,entry.newFrom);
	}
	theNetwork->edgeFromList[entry.edgeIndex] = entry.newFrom;
	theNetwork->edgeToList[entry.edgeIndex] = entry.newTo;
}

void CVNetworkUndoRewire(CVNetwork* theNetwork, CVNetworkRewireEntry entry){
	CVNetworkRewireEntry reverseEntry;
	reverseEntry.edgeIndex = entry.edgeIndex;
	reverseEntry.newTo = entry.previousTo;
	reverseEntry.newFrom = entry.previousFrom;
	reverseEntry.previousTo = entry.newTo;
	reverseEntry.previousFrom = entry.newFrom;
	CVNetworkDoRewire(theNetwork, reverseEntry);
}


CVNetwork* CVNewNetworkWithNetworkAndRewire(const CVNetwork* originalNetwork, CVNetworkRewireEntry rewireEntry){
	if(originalNetwork){
		CVNetwork * theNetwork = NULL;
		theNetwork = CV_NewAllocationNetwork(originalNetwork->verticesCount);
		theNetwork->vertexWeighted = CVFalse;
		theNetwork->edgeWeighted = originalNetwork->edgeWeighted;
		theNetwork->directed = originalNetwork->directed;
		CVIndex i;
		for(i=0;i<originalNetwork->edgesCount;i++){
			CVIndex from,to;
			from = originalNetwork->edgeFromList[i];
			to = originalNetwork->edgeToList[i];
			if(CVUnlikely(rewireEntry.edgeIndex==i)){
				from = rewireEntry.newFrom;
				to = rewireEntry.newTo;
			}else{//REMOVE ELSE FOR DEFAULT REWIRE
			CVFloat weight = 1.0f;
			if(originalNetwork->edgeWeighted){
				weight = originalNetwork->edgesWeights[i];
			}
			CVNetworkAddNewEdge(theNetwork, from, to, weight);
			}
		}
		CVIndex propertyIndex;
		for(propertyIndex=0;propertyIndex<originalNetwork->propertiesCount;propertyIndex++){
			CVPropertyType type = originalNetwork->propertiesTypes[propertyIndex];
			void* data = originalNetwork->propertiesData[propertyIndex];
			CVString name = originalNetwork->propertiesNames[propertyIndex];
			CVNetworkAppendProperty(theNetwork, name, type, data);
		}
		return theNetwork;
	}else{
		return NULL;
	}
}





CVNetwork* CVNewNetworkWithNetworkAndRemoveEntry(const CVNetwork* originalNetwork, CVNetworkRewireEntry rewireEntry){
	if(originalNetwork){
		CVNetwork * theNetwork = NULL;
		theNetwork = CV_NewAllocationNetwork(originalNetwork->verticesCount);
		theNetwork->vertexWeighted = CVFalse;
		theNetwork->edgeWeighted = originalNetwork->edgeWeighted;
		theNetwork->directed = originalNetwork->directed;
		CVIndex i;
		for(i=0;i<originalNetwork->edgesCount;i++){
			CVIndex from,to;
			from = originalNetwork->edgeFromList[i];
			to = originalNetwork->edgeToList[i];
			if(CVUnlikely(rewireEntry.edgeIndex==i)){
				from = rewireEntry.newFrom;
				to = rewireEntry.newTo;
			}
			CVFloat weight = 1.0f;
			if(originalNetwork->edgeWeighted){
				weight = originalNetwork->edgesWeights[i];
			}
			CVNetworkAddNewEdge(theNetwork, from, to, weight);
		}
		CVIndex propertyIndex;
		for(propertyIndex=0;propertyIndex<originalNetwork->propertiesCount;propertyIndex++){
			CVPropertyType type = originalNetwork->propertiesTypes[propertyIndex];
			void* data = originalNetwork->propertiesData[propertyIndex];
			CVString name = originalNetwork->propertiesNames[propertyIndex];
			CVNetworkAppendProperty(theNetwork, name, type, data);
		}
		return theNetwork;
	}else{
		return NULL;
	}
}





CVNetwork* CVNewNetworkByRemovingEdgeFast(const CVNetwork* originalNetwork, CVIndex edgeIndex){
	if(originalNetwork){
		CVNetwork * theNetwork = NULL;
		theNetwork = CV_NewAllocationNetwork(originalNetwork->verticesCount);
		theNetwork->vertexWeighted = CVFalse;
		theNetwork->edgeWeighted = CVFalse;
		theNetwork->directed = originalNetwork->directed;
		CVIndex i;

		for(i=0;i<originalNetwork->edgesCount;i++){
			CVIndex from, to;
			from = originalNetwork->edgeFromList[i];
			to = originalNetwork->edgeToList[i];
			if(CVLikely(edgeIndex!=i)){
				CVNetworkAddNewEdge(theNetwork, from, to, 1.0);
			}
		}
		return theNetwork;
	}else{
		return NULL;
	}
}


CVNetwork* CVNewNetworkByAddingEdgeFast(const CVNetwork* originalNetwork, CVIndex fromIndex, CVIndex toIndex){
	CVNetwork * theNetwork = NULL;
	theNetwork = CV_NewAllocationNetwork(originalNetwork->verticesCount);
	theNetwork->vertexWeighted = CVFalse;
	theNetwork->edgeWeighted = CVFalse;
	theNetwork->directed = originalNetwork->directed;
	CVIndex i;
	for(i=0;i<originalNetwork->edgesCount;i++){
		CVIndex from,to;
		from = originalNetwork->edgeFromList[i];
		to = originalNetwork->edgeToList[i];
		CVNetworkAddNewEdge(theNetwork, from, to, 1.0);
	}
	CVNetworkAddNewEdge(theNetwork, fromIndex, toIndex, 1.0);
	return theNetwork;
}



