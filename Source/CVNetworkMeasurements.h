//
//  CVNetworkMeasurements.h
//  CVNetwork
//
//  Created by Filipi Nascimento Silva on 10/29/16.
//  Copyright © 2016 Filipi Nascimento Silva. All rights reserved.
//

#ifndef CVNetworkMeasurements_h
#define CVNetworkMeasurements_h

#include <stdio.h>
#include "CVNetwork.h"
#include "CVConcentricStructure.h"


typedef struct _CV_measurementFunction {
	CVDouble (*function)(const CVNetwork*, void*);// network and context
	CVString name;
	void* context;
} CVNetworkMeasurementFunction;


#define CVMakeMeasurementFunction(name,function) {function, name, NULL}

CVDouble CVAverageShortestPathLength(const CVNetwork* theNetwork, void * context);
CVDouble CVNetworkAverageClusteringCoefficient(const CVNetwork* aNetwork, void * context);
CVDouble CVNetworkDegreeAssortativity(const CVNetwork* aNetwork, void * context);
CVDouble CVNetworkDegreeEntropy(const CVNetwork* aNetwork, void * context);

#endif /* CVNetworkMeasurements_h */
