//
//  CVNetworkMeasurements.c
//  CVNetwork
//
//  Created by Filipi Nascimento Silva on 10/29/16.
//  Copyright © 2016 Filipi Nascimento Silva. All rights reserved.
//

#include "CVNetworkMeasurements.h"



CVDouble CVAverageShortestPathLength(const CVNetwork* theNetwork, void * context){
	CVConcentricStructure* concentricStructure = CVNewConcentricStructureForNetwork(theNetwork, CVFalse);
	
	CVDouble pathHarmonicSum = 0.0;
	CVUInteger pathCount = 0;
	for (CVIndex vertexIndex=0; vertexIndex<theNetwork->verticesCount; vertexIndex++) {
		CVConcentricStructureSetReferenceVertex(vertexIndex, 50, concentricStructure);
		for (CVIndex level=1; level<concentricStructure->levelsCount;level++){
			CVUInteger levelVertices = CVConcentricCountVerticesAtLevel(level, concentricStructure);
			if(levelVertices==0){
				break;
			}else{
				pathHarmonicSum += levelVertices*(CVDouble)level;
				pathCount+=levelVertices;
			}
		}
	}
	CVConcentricStructureDestroy(concentricStructure);
	return pathHarmonicSum/pathCount;
}





CVDouble CVNetworkAverageClusteringCoefficient(const CVNetwork* aNetwork, void * context){
	CVSize verticesCount = aNetwork->verticesCount;
	CVDouble averageClustering = 0;
	for(CVIndex vertexIndex=0;vertexIndex<verticesCount;vertexIndex++){
		CVSize vertexEdgesCount = aNetwork->vertexNumOfEdges[vertexIndex];
		CVIndex* vertexEdgesList = aNetwork->vertexEdgesLists[vertexIndex];
		CVSize inLevelConnections = 0;
		CVIndex ni;
		CVBitArrayStatic(isNeighbor,verticesCount);
		CVBitArrayClearAll(isNeighbor, verticesCount);
		for(ni=0;ni<vertexEdgesCount;ni++){
			CVBitArraySet(isNeighbor, vertexEdgesList[ni]);
		}
		for(ni=0;ni<vertexEdgesCount;ni++){
			CVIndex neighborVertex = vertexEdgesList[ni];
			CVSize neighborEdgesCount = aNetwork->vertexNumOfEdges[neighborVertex];
			CVIndex* neighborEdgesList = aNetwork->vertexEdgesLists[neighborVertex];
			CVIndex nni;
			for(nni=0;nni<neighborEdgesCount;nni++){
				if(CVBitArrayTest(isNeighbor,neighborEdgesList[nni])){
					inLevelConnections++;
				}
			}
		}
		if((vertexEdgesCount-1.0) > 0.0){
			averageClustering+= (inLevelConnections)/(CVFloat)(vertexEdgesCount*(vertexEdgesCount-1.0f));
		}
	}
	averageClustering/=verticesCount;
	return averageClustering;
}


CVDouble CVNetworkDegreeAssortativity(const CVNetwork* aNetwork, void * context){
	CVSize edgesCount = aNetwork->edgesCount;
	
	CVDouble jkSum = 0;
	CVDouble jpkSum = 0;
	CVDouble j2pk2Sum = 0;
	
	for (CVIndex edgeIndex=0; edgeIndex<edgesCount; edgeIndex++) {
		CVIndex fromIndex = aNetwork->edgeFromList[edgeIndex];
		CVIndex toIndex = aNetwork->edgeToList[edgeIndex];
		CVDouble j = aNetwork->vertexNumOfEdges[fromIndex];
		CVDouble k = aNetwork->vertexNumOfEdges[toIndex];
		jkSum+=j*k;
		jpkSum+=j+k;
		j2pk2Sum+=j*j+k*k;
	}
	CVDouble Minv = 1.0/edgesCount;
	
	CVDouble rUp = Minv*jkSum - (Minv * 0.5 * jpkSum)*(Minv * 0.5 * jpkSum);
	
	CVDouble rDown = Minv*j2pk2Sum - (Minv * 0.5 * jpkSum)*(Minv * 0.5 * jpkSum);
	
	if(rDown!=0.0){
		return rUp/rDown;
	}else{
		return 0.0;
	}
}

CVDouble CVNetworkDegreeEntropy(const CVNetwork* aNetwork, void * context){
	CVSize verticesCount = aNetwork->verticesCount;
	CVDouble degreeEntropy = 0.0;
	CVSize maximumDegree = 0;

#define k_CVNetworkDegreeEntropyStaticChunkSize 128

	for(CVIndex vertexIndex=0;vertexIndex<verticesCount;vertexIndex++){
		CVSize vertexDegree = aNetwork->vertexNumOfEdges[vertexIndex];
		maximumDegree = CVMAX(maximumDegree, vertexDegree);
	}
	
	CVSize degreeFrequenciesStaticChunck[k_CVNetworkDegreeEntropyStaticChunkSize];
	CVSize* degreeFrequencies = degreeFrequenciesStaticChunck;
	
	if(maximumDegree+1>k_CVNetworkDegreeEntropyStaticChunkSize){
		degreeFrequencies = calloc(maximumDegree+1, sizeof(CVSize));
	}
	
	for (CVIndex degreeIndex=0; degreeIndex<=maximumDegree; degreeIndex++) {
		degreeFrequencies[degreeIndex] = 0;
	}
	
	for(CVIndex vertexIndex=0;vertexIndex<verticesCount;vertexIndex++){
		CVSize vertexDegree = aNetwork->vertexNumOfEdges[vertexIndex];
		degreeFrequencies[vertexDegree]+=1;
	}
	
	for (CVIndex degreeIndex=0; degreeIndex<=maximumDegree; degreeIndex++) {
		CVSize frequency = degreeFrequencies[degreeIndex];
		if(frequency>0){
			CVDouble p = frequency/(CVDouble)verticesCount;
			degreeEntropy += -p*log(p);
		}
	}
	
	if(maximumDegree+1>k_CVNetworkDegreeEntropyStaticChunkSize){
		free(degreeFrequencies);
	}
#undef k_CVNetworkDegreeEntropyStaticChunkSize
	
	return degreeEntropy;
}








