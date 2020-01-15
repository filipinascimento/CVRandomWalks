//
//  CVNetworkAgent.h
//  CVNetwork
//
//  Created by Filipi Nascimento Silva on 8/27/13.
//  Copyright (c) 2013 Filipi Nascimento Silva. All rights reserved.
//

#ifndef CVNetwork_CVNetworkAgent_h
#define CVNetwork_CVNetworkAgent_h
#include "CVNetwork.h"
#include "CVSimpleQueue.h"

CV_INLINE CVAgentPath* CVAgentPathCreate(CVUInteger capacity){
	CVAgentPath* agentPath = calloc(1, sizeof(CVAgentPath));
	agentPath->count=0;
	agentPath->_capacity=capacity;
	if(capacity==0) {
		agentPath->data = NULL;
	}else{
		agentPath->data = calloc(agentPath->_capacity, sizeof(CVNetworkAgent));
	}
	agentPath->visitedNodes = NULL;
	return agentPath;
}

CV_INLINE void CVAgentPathDestroy(CVAgentPath* thePath){
	if(thePath!=NULL){
		if(thePath->data!=NULL){
			free(thePath->data);
		}
		if(thePath->visitedNodes){
			CVBitArrayDestroy(thePath->visitedNodes);
		}
		free(thePath);
	}
}

CV_INLINE void CVAgentPathReallocToCapacity(CVUInteger newCapacity, CVAgentPath* thePath){
	if(thePath->data!=NULL){
		thePath->data = realloc(thePath->data,newCapacity*sizeof(CVNetworkAgent));
	}else{
		thePath->data = calloc(newCapacity, sizeof(CVNetworkAgent));
	}
	thePath->_capacity=newCapacity;
	if(thePath->_capacity<thePath->count) thePath->count = thePath->_capacity;
}

CV_INLINE void CVAgentPathPushAgent(CVNetworkAgent agent, CVAgentPath* thePath){
	if(thePath->_capacity < thePath->count+1){
		CVAgentPathReallocToCapacity(CVCapacityGrow(thePath->count), thePath);
	}
	thePath->data[thePath->count] = agent;
	thePath->count++;
	if(thePath->visitedNodes){
		CVBitArraySet(thePath->visitedNodes,agent.vertex);
	}
}


CV_INLINE CVNetworkAgent CVAgentPathPopAgent(CVAgentPath* path){
	if(path->count>0){
		path->count--;
		CVNetworkAgent agent = path->data[path->count];
		if(path->visitedNodes){
			CVBitArrayClear(path->visitedNodes,agent.vertex);
		}
		return agent;
	}else{
		CVNetworkAgent agent;
		agent.vertex = 0;
		agent.weight = NAN;
		agent.level = 0;
		return agent;
	}
}

CV_INLINE CVNetworkAgent* CVAgentPathLast(CVAgentPath* path){
	if(path->count>0){
		return path->data + (path->count - 1);
	}else{
		return NULL;
	}
}

CV_INLINE CVBool CVAgentPathIsEmpty(CVAgentPath* path){
	if(path->count>0){
		return CVFalse;
	}else{
		return CVTrue;
	}
}

CV_INLINE CVBool CVAgentPathContainsVertex(const CVAgentPath* path,CVIndex aVertex){
	//CVIndex pathIndex;
	//for (pathIndex=0; pathIndex< path->count - 1 ; pathIndex++) {
	//	if(path->data[pathIndex].vertex==aVertex){
	//		return CVTrue;
	//	}
	//}
	return CVBitArrayTest(path->visitedNodes,aVertex);
}



typedef struct{
	CVNetwork mergedNetwork;
	CVIndex* translateMerged;
} CVNetworkPathMergedContext;

CV_INLINE CVBool CVAgentMergedGetMergeddIndex(CVIndex vertexIndex, CVIndex* mergedIndex, CVNetworkPathMergedContext* mergedContext){
	CVIndex translatedIndex = mergedContext->translateMerged[vertexIndex];
	if(translatedIndex){
		*mergedIndex = translatedIndex;
		return CVTrue;
	}else{
		*mergedIndex = vertexIndex;
		return CVFalse;
	}
}


//FIXME: unweighted
CV_INLINE void CVAgentGetDistancesFromVertex(const CVNetwork* network, CVIndex referenceVertex, CVSize* distances, CVBitArray existingBitArray, CVSize maximumDistance,CVNetworkPathMergedContext* mergedContext){
	CVBitArray visitedNodes = existingBitArray;
	if(!existingBitArray){
		visitedNodes = CVNewBitArray(network->verticesCount);
	}
	
	if(mergedContext){
		if(mergedContext->translateMerged){
			memset(mergedContext->translateMerged,0, network->verticesCount*sizeof(CVIndex));
		}else{
			mergedContext->translateMerged = calloc(network->verticesCount, sizeof(CVIndex));
		}
	}
	
	CVQueue toVisitQueue = CVQueueCreate();
	CVInteger currentVertex = referenceVertex;
	CVQueuePush(&toVisitQueue, currentVertex);
	CVBitArraySet(visitedNodes, currentVertex);
	while (CVQueueDequeue(&toVisitQueue,&currentVertex)) {
		CVSize currentDistance = distances[currentVertex];
		if(currentDistance<maximumDistance){
			CVSize vertexEdgesCount = network->vertexNumOfEdges[currentVertex];
			CVIndex* vertexEdgesList = network->vertexEdgesLists[currentVertex];
			CVIndex ni;
			for(ni=0;ni<vertexEdgesCount;ni++){
				CVIndex neighborVertex = vertexEdgesList[ni];
				if(!CVBitArrayTest(visitedNodes, neighborVertex)){
					distances[neighborVertex] = currentDistance+1;
					CVBitArraySet(visitedNodes, neighborVertex);
					CVQueuePush(&toVisitQueue, neighborVertex);
				}else if(distances[neighborVertex]==currentDistance){
					//at same distance
					
				}
			}
		}
	}
	CVQueueDestroy(&toVisitQueue);
	if(!existingBitArray){
		CVBitArrayDestroy(visitedNodes);
	}else{
		CVBitArrayClearAll(existingBitArray, network->verticesCount);
	}
}


CV_INLINE CVInteger CVNetworkPathSelfAvoidingNextBranch(const CVNetwork* network, const CVNetworkAgent* currentAgent, const CVAgentPath* path, CVFloat* choiceProbability, CVNetworkAgent* nextAgent){
	CVInteger currentLevel = currentAgent->level;
	CVIndex currentVertex = currentAgent->vertex;
	CVIndex branchIndex = currentAgent->branchIndex;
	CVFloat currentWeight = currentAgent->weight;
	CVFloat* edgesWeights = network->edgesWeights;
	CVSize vertexEdgesCount = network->vertexNumOfEdges[currentVertex];
	CVIndex* vertexEdgesList = network->vertexEdgesLists[currentVertex];
	CVIndex nextVertex = 0;
	
	
	//=CVFalse;
	CVFloat sumProbability = 0.0f;
	
	if(choiceProbability && branchIndex < vertexEdgesCount){
		//if(network->edgeWeighted){
		CVIndex curBranch=0;;
		while(curBranch < vertexEdgesCount){
			CVFloat probability = 1.0;
			nextVertex = vertexEdgesList[curBranch++];
			if(network->edgeWeighted){
				probability = edgesWeights[network->vertexEdgesIndices[currentVertex][curBranch-1]];
			}else{
				probability = 1.0;
			}
			if(!CVAgentPathContainsVertex(path,nextVertex)){
				sumProbability+=probability;
			}
		}
		//}else{
		//	sumProbability = vertexEdgesCount - branchIndex;
		//}
		*choiceProbability = 1.0/sumProbability;
	}
	
	
	while(branchIndex < vertexEdgesCount){
		nextVertex = vertexEdgesList[branchIndex++];
		if(CVUnlikely(!network->verticesEnabled[nextVertex])){
			continue;
		}
		
		if(!CVAgentPathContainsVertex(path,nextVertex)){
			if(network->edgeWeighted){
				currentWeight *= edgesWeights[network->vertexEdgesIndices[currentVertex][branchIndex-1]];
			}
			nextAgent->vertex = nextVertex;
			nextAgent->level = currentLevel+1;
			nextAgent->weight = currentWeight;
			nextAgent->branchIndex=0;
			break;
		}
	}
	return branchIndex;
}

CV_INLINE CVInteger CVNetworkPathNextBranch(const CVNetwork* network, const CVNetworkAgent* currentAgent, const CVAgentPath* path, CVFloat* choiceProbability, CVNetworkAgent* nextAgent){
	CVInteger currentLevel = currentAgent->level;
	CVIndex currentVertex = currentAgent->vertex;
	CVIndex branchIndex = currentAgent->branchIndex;
	CVFloat currentWeight = currentAgent->weight;
	CVFloat* edgesWeights = network->edgesWeights;

	CVSize vertexEdgesCount = network->vertexNumOfEdges[currentVertex];
	CVIndex* vertexEdgesList = network->vertexEdgesLists[currentVertex];
	CVIndex nextVertex = 0;

	//=CVFalse;
	
	if(choiceProbability && branchIndex < vertexEdgesCount){
		CVFloat sumProbability = 0.0f;
		if(network->edgeWeighted){
			CVIndex curBranch=0;;
			while(curBranch < vertexEdgesCount){
				sumProbability+= edgesWeights[network->vertexEdgesIndices[currentVertex][curBranch-1]];
			}
		}else{
			sumProbability=vertexEdgesCount;
		}
		*choiceProbability = 1.0/sumProbability;
	}
	while(branchIndex < vertexEdgesCount){
		nextVertex = vertexEdgesList[branchIndex++];
		if(CVUnlikely(!network->verticesEnabled[nextVertex])){
			break;
		}

		if(network->edgeWeighted){
			currentWeight *= network->edgesWeights[network->vertexEdgesIndices[currentVertex][branchIndex-1]];
		}
		nextAgent->vertex = nextVertex;
		nextAgent->level = currentLevel+1;
		nextAgent->weight = currentWeight;
		nextAgent->branchIndex=0;
		break;
	}

	return branchIndex;
}





CV_INLINE CVInteger CVNetworkPathBackboneSelfAvoidingNextBranch(const CVNetwork* network, const CVNetworkAgent* currentAgent, const CVAgentPath* path, CVFloat* choiceProbability, CVNetworkAgent* nextAgent, CVSize* distances){
	CVInteger currentLevel = currentAgent->level;
	CVIndex currentVertex = currentAgent->vertex;
	CVIndex branchIndex = currentAgent->branchIndex;
	CVFloat currentWeight = currentAgent->weight;
	CVFloat* edgesWeights = network->edgesWeights;
	CVSize vertexEdgesCount = network->vertexNumOfEdges[currentVertex];
	CVIndex* vertexEdgesList = network->vertexEdgesLists[currentVertex];
	CVIndex nextVertex = 0;
	
	//=CVFalse;
	CVFloat sumProbability = 0.0f;
	
	if(choiceProbability && branchIndex < vertexEdgesCount){
		//if(network->edgeWeighted){
		CVIndex curBranch=0;
		while(curBranch < vertexEdgesCount){
			CVFloat probability = 1.0;
			nextVertex = vertexEdgesList[curBranch++];
			if(network->edgeWeighted){
				probability = edgesWeights[network->vertexEdgesIndices[currentVertex][curBranch-1]];
			}else{
				probability = 1.0;
			}
			if(distances[currentVertex]<distances[nextVertex]){
				sumProbability+=probability;
			}
		}
		//}else{
		//	sumProbability = vertexEdgesCount - branchIndex;
		//}
		*choiceProbability = 1.0/sumProbability;
	}
	
	
	while(branchIndex < vertexEdgesCount){
		nextVertex = vertexEdgesList[branchIndex++];
		if(CVUnlikely(!network->verticesEnabled[nextVertex])){
			continue;
		}
		
		if(distances[currentVertex]<distances[nextVertex]){
			if(network->edgeWeighted){
				currentWeight *= edgesWeights[network->vertexEdgesIndices[currentVertex][branchIndex-1]];
			}
			nextAgent->vertex = nextVertex;
			nextAgent->level = currentLevel+1;
			nextAgent->weight = currentWeight;
			nextAgent->branchIndex=0;
			break;
		}
	}
	return branchIndex;
}

CV_INLINE CVInteger CVNetworkPathMergedSelfAvoidingNextBranch(const CVNetwork* network, const CVNetworkAgent* currentAgent, const CVAgentPath* path, CVFloat* choiceProbability, CVNetworkAgent* nextAgent, CVSize* distances,CVNetworkPathMergedContext* context){
	CVInteger currentLevel = currentAgent->level;
	CVIndex currentVertex = currentAgent->vertex;
	CVIndex branchIndex = currentAgent->branchIndex;
	CVFloat currentWeight = currentAgent->weight;
	CVFloat* edgesWeights = network->edgesWeights;
	CVSize vertexEdgesCount = network->vertexNumOfEdges[currentVertex];
	CVIndex* vertexEdgesList = network->vertexEdgesLists[currentVertex];
	CVIndex nextVertex = 0;
	
	//=CVFalse;
	CVFloat sumProbability = 0.0f;
	
	if(branchIndex < vertexEdgesCount){
		CVIndex curBranch=0;
		while(curBranch < vertexEdgesCount){
			CVFloat probability = 1.0;
			nextVertex = vertexEdgesList[curBranch++];
			if(network->edgeWeighted){
				probability = edgesWeights[network->vertexEdgesIndices[currentVertex][curBranch-1]];
			}else{
				probability = 1.0;
			}
			if(distances[currentVertex]<distances[nextVertex]){
				sumProbability+=probability;
			}
		}
		
		if(choiceProbability){
			*choiceProbability = 1.0/sumProbability;
		}
	}
	
	
	while(branchIndex < vertexEdgesCount){
		nextVertex = vertexEdgesList[branchIndex++];
		if(CVUnlikely(!network->verticesEnabled[nextVertex])){
			continue;
		}
		
		if(distances[currentVertex]<distances[nextVertex]){
			if(network->edgeWeighted){
				currentWeight *= edgesWeights[network->vertexEdgesIndices[currentVertex][branchIndex-1]];
			}
			nextAgent->vertex = nextVertex;
			nextAgent->level = currentLevel+1;
			nextAgent->weight = currentWeight;
			nextAgent->branchIndex=0;
			break;
		}else if(distances[currentVertex]==distances[nextVertex] &&  !CVAgentPathContainsVertex(path,nextVertex)){
			if(network->edgeWeighted){
				currentWeight *= edgesWeights[network->vertexEdgesIndices[currentVertex][branchIndex-1]];
			}
			nextAgent->vertex = nextVertex;
			nextAgent->level = currentLevel;
			nextAgent->weight = currentWeight;
			nextAgent->branchIndex=0;
			if(choiceProbability){
				*choiceProbability = 1.0;
			}
			break;
		}
	}
	return branchIndex;
}



#endif
