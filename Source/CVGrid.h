//
//  CVGrid.h
//  CVNetwork
//
//  Created by Filipi Nascimento Silva on 10/30/16.
//  Copyright © 2016 Filipi Nascimento Silva. All rights reserved.
//

#ifndef CVGrid_h
#define CVGrid_h

#include "CVCommons.h"

typedef struct _CV_Grid {
	CVSize dimension;
	CVSize* sides;
} CVGrid;


CV_INLINE CVGrid* CVNewGrid(const CVSize* sides, CVSize dimension){
	CVGrid* theGrid = NULL;
	if(sides && dimension>0){
		theGrid = calloc(1, sizeof(CVGrid));
		theGrid->sides = calloc(dimension, sizeof(CVSize));
		theGrid->dimension=dimension;
		for (CVIndex d=0; d<dimension; d++) {
			theGrid->sides[d] = sides[d];
			if(sides[d]==0){
				//All sides must be larger than 0 otherwise return NULL;
				free(theGrid->sides);
				free(theGrid);
				theGrid = NULL;
				break;
			}
		}
	}
	return theGrid;
}

CV_INLINE void CVGridDestroy(CVGrid* grid){
	free(grid->sides);
	free(grid);
}

CV_INLINE CVSize CVGridLinearCount(const CVGrid* grid){
	CVSize dimension = grid->dimension;
	const CVSize* gridSize = grid->sides;
	switch (dimension) {
		case 1:{
			return gridSize[0];
		}
		case 2:{
			return gridSize[0]*gridSize[1];
		}
		case 3:{
			return gridSize[0]*gridSize[1]*gridSize[2];
		}
		default:{
			CVSize gridLinearCount = 1;
			for(CVIndex d=0;d<dimension;d++){
				gridLinearCount*=gridSize[d];
			}
			return gridLinearCount;
		}
	}
}

CV_INLINE void* CVGridCalloc(const CVGrid* grid, CVSize elementSize){
	CVSize linearCount = CVGridLinearCount(grid);
	return calloc(linearCount, elementSize);
}

CV_INLINE CVInteger* CVGridNewVector(const CVGrid* grid){
	CVSize dimension = grid->dimension;
	return calloc(dimension, sizeof(CVIndex));
}


CV_INLINE CVFloat CVGridCalculateDistance(const CVGrid* grid,const CVInteger* origin, const CVInteger* vector){
	CVSize dimension = grid->dimension;
	switch (dimension) {
		case 1:{
			return CVABS(origin[0]-vector[0]);
		}
		case 2:{
			CVInteger dx = origin[0]-vector[0];
			CVInteger dy = origin[1]-vector[1];
			return sqrtf(dx*dx + dy*dy);
		}
		case 3:{
			CVInteger dx = origin[0]-vector[0];
			CVInteger dy = origin[1]-vector[1];
			CVInteger dz = origin[2]-vector[2];
			return sqrtf(dx*dx + dy*dy + dz*dz);
		}
		default:{
			CVInteger sqrSum = 0;
			for(CVIndex d=0;d<dimension;d++){
				CVInteger dDiff = vector[d]-origin[d];
				sqrSum+= dDiff*dDiff;
			}
			return sqrtf(sqrSum);
		}
	}
}


CV_INLINE CVFloat CVGridCalculateDistanceFromOrigin(const CVGrid* grid, const CVInteger* vector){
	CVSize dimension = grid->dimension;
	switch (dimension) {
		case 1:{
			return CVABS(vector[0]);
		}
		case 2:{
			CVInteger dx = vector[0];
			CVInteger dy = vector[1];
			return sqrtf(dx*dx + dy*dy);
		}
		case 3:{
			CVInteger dx = vector[0];
			CVInteger dy = vector[1];
			CVInteger dz = vector[2];
			return sqrtf(dx*dx + dy*dy + dz*dz);
		}
		default:{
			CVInteger sqrSum = 0;
			for(CVIndex d=0;d<dimension;d++){
				CVInteger dDiff = vector[d];
				sqrSum+= dDiff*dDiff;
			}
			return sqrtf(sqrSum);
		}
	}
}

CV_INLINE CVIndex CVGridLinearIndexFromCoordinates(const CVGrid *grid, const CVInteger* coordinates){
	CVSize dimension = grid->dimension;
	const CVSize* gridSize = grid->sides;
	switch (dimension) {
		case 1:{
			return coordinates[0];
		}
		case 2:{
			return coordinates[0] + gridSize[0]*coordinates[1];
		}
		case 3:{
			return coordinates[0] + gridSize[0]*(coordinates[1]+gridSize[1]*(coordinates[2]));
		}
		case 4:{
			return coordinates[0] + gridSize[0]*(coordinates[1]+gridSize[1]*(coordinates[2]+gridSize[3]*coordinates[3]));
		}
		default:{
			CVInteger linearIndex = 0;
			CVInteger coefficient = 1;
			CVIndex curDim;
			for(curDim=0;curDim<dimension;curDim++){
				linearIndex += coefficient*coordinates[curDim];
				coefficient*=gridSize[curDim];
			}
			return linearIndex;
		}
	}
}

CV_INLINE void CVGridGetCoordinatesFromLinearIndex(const CVGrid *grid, CVInteger linearIndex, CVInteger* coordinates){
	CVSize dimension = grid->dimension;
	const CVSize* gridSize = grid->sides;
	switch (dimension) {
		case 1:{
			coordinates[0] = linearIndex;
			return;
		}
		case 2:{
			coordinates[0] = linearIndex % gridSize[0];
			coordinates[1] = (linearIndex/gridSize[0]) % gridSize[1];
			return;
		}
		case 3:{
			coordinates[0] = linearIndex % gridSize[0];
			linearIndex /= (gridSize[0]);
			coordinates[1] = linearIndex % gridSize[1];
			linearIndex /= (gridSize[1]);
			coordinates[2] = linearIndex % gridSize[2];
			return;
		}
		case 4:{
			coordinates[0] = linearIndex % gridSize[0];
			linearIndex /= (gridSize[0]);
			coordinates[1] = linearIndex % gridSize[1];
			linearIndex /= (gridSize[1]);
			coordinates[2] = linearIndex % gridSize[2];
			linearIndex /= (gridSize[2]);
			coordinates[3] = linearIndex % gridSize[3];
			return;
		}
		default:{
			CVIndex curDim;
			for(curDim=0;curDim<dimension;curDim++){
				coordinates[curDim] = linearIndex % gridSize[curDim];
				linearIndex /= (gridSize[curDim]);
			}
		}
	}
}


CV_INLINE CVBool CVGridCheckCoordinateBounds(CVInteger coordinateValue, CVSize bound){
	if(coordinateValue<0 || coordinateValue>=bound){
		return CVFalse;
	}else{
		return CVTrue;
	}
}


CV_INLINE CVBool CVGridGetDisplacedCoordinate(const CVGrid *grid, const CVInteger* originalCoordinates, const  CVInteger* displacement, const  CVInteger* center, CVInteger* dstCoordinates){
	CVSize dimension = grid->dimension;
	const CVSize* gridSize = grid->sides;
	switch (dimension) {
		case 1:{
			CVInteger newCoordinate = originalCoordinates[0] + displacement[0] - center[0];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[0])){
				dstCoordinates[0] = newCoordinate;
			}else{
				return CVFalse;
			}
			break;
		}
		case 2:{
			CVInteger newCoordinate = originalCoordinates[0] + displacement[0] - center[0];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[0])){
				dstCoordinates[0] = newCoordinate;
			}else{
				return CVFalse;
			}
			newCoordinate = originalCoordinates[1] + displacement[1] - center[1];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[1])){
				dstCoordinates[1] = newCoordinate;
			}else{
				return CVFalse;
			}
			break;
		}
		case 3:{
			CVInteger newCoordinate = originalCoordinates[0] + displacement[0] - center[0];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[0])){
				dstCoordinates[0] = newCoordinate;
			}else{
				return CVFalse;
			}
			newCoordinate = originalCoordinates[1] + displacement[1] - center[1];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[1])){
				dstCoordinates[1] = newCoordinate;
			}else{
				return CVFalse;
			}
			newCoordinate = originalCoordinates[2] + displacement[2] - center[2];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[2])){
				dstCoordinates[2] = newCoordinate;
			}else{
				return CVFalse;
			}
			break;
		}
		case 4:{
			CVInteger newCoordinate = originalCoordinates[0] + displacement[0] - center[0];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[0])){
				dstCoordinates[0] = newCoordinate;
			}else{
				return CVFalse;
			}
			newCoordinate = originalCoordinates[1] + displacement[1] - center[1];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[1])){
				dstCoordinates[1] = newCoordinate;
			}else{
				return CVFalse;
			}
			newCoordinate = originalCoordinates[2] + displacement[2] - center[2];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[2])){
				dstCoordinates[2] = newCoordinate;
			}else{
				return CVFalse;
			}
			newCoordinate = originalCoordinates[3] + displacement[3] - center[3];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[3])){
				dstCoordinates[3] = newCoordinate;
			}else{
				return CVFalse;
			}
			break;
		}
		default:{
			CVIndex i;
			for(i=0;i<dimension;i++){
				CVInteger newCoordinate = originalCoordinates[i]+displacement[i] - center[i];
				if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[i])){
					dstCoordinates[i] = newCoordinate;
				}else{
					return CVFalse;
				}
			}
			break;
		}
	}
	return CVTrue;
}



CV_INLINE CVBool CVGridGetDisplacedToroidalCoordinate(const CVGrid *grid, const CVInteger* originalCoordinates, const  CVInteger* displacement,  const  CVInteger* center, CVInteger* dstCoordinates){
	CVSize dimension = grid->dimension;
	const CVSize* gridSize = grid->sides;
	switch (dimension) {
		case 1:{
			CVInteger newCoordinate = (gridSize[0] + originalCoordinates[0] + displacement[0] - center[0]) % gridSize[0];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[0])){
				dstCoordinates[0] = newCoordinate;
			}else{
				return CVFalse;
			}
			break;
		}
		case 2:{
			CVInteger newCoordinate = (gridSize[0] + originalCoordinates[0] + displacement[0] - center[0]) % gridSize[0];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[0])){
				dstCoordinates[0] = newCoordinate;
			}else{
				return CVFalse;
			}
			newCoordinate = (gridSize[1] + originalCoordinates[1] + displacement[1] - center[1]) % gridSize[1];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[1])){
				dstCoordinates[1] = newCoordinate;
			}else{
				return CVFalse;
			}
			break;
		}
		case 3:{
			CVInteger newCoordinate = (gridSize[0] + originalCoordinates[0] + displacement[0] - center[0]) % gridSize[0];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[0])){
				dstCoordinates[0] = newCoordinate;
			}else{
				return CVFalse;
			}
			newCoordinate = (gridSize[1] + originalCoordinates[1] + displacement[1] - center[1]) % gridSize[1];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[1])){
				dstCoordinates[1] = newCoordinate;
			}else{
				return CVFalse;
			}
			newCoordinate = (gridSize[2] + originalCoordinates[2] + displacement[2] - center[2]) % gridSize[2];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[2])){
				dstCoordinates[2] = newCoordinate;
			}else{
				return CVFalse;
			}
			break;
		}
		case 4:{
			CVInteger newCoordinate = (gridSize[0] + originalCoordinates[0] + displacement[0] - center[0]) % gridSize[0];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[0])){
				dstCoordinates[0] = newCoordinate;
			}else{
				return CVFalse;
			}
			newCoordinate = (gridSize[1] + originalCoordinates[1] + displacement[1] - center[1]) % gridSize[1];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[1])){
				dstCoordinates[1] = newCoordinate;
			}else{
				return CVFalse;
			}
			newCoordinate = (gridSize[2] + originalCoordinates[2] + displacement[2] - center[2]) % gridSize[2];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[2])){
				dstCoordinates[2] = newCoordinate;
			}else{
				return CVFalse;
			}
			newCoordinate = (gridSize[3] + originalCoordinates[3] + displacement[3] - center[3]) % gridSize[3];
			if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[3])){
				dstCoordinates[3] = newCoordinate;
			}else{
				return CVFalse;
			}
			break;
		}
		default:{
			CVIndex i;
			for(i=0;i<dimension;i++){
				CVInteger newCoordinate = (gridSize[i] + originalCoordinates[i]+displacement[i] - center[i]) % gridSize[i];
				if(CVGridCheckCoordinateBounds(newCoordinate, gridSize[i])){
					dstCoordinates[i] = newCoordinate;
				}else{
					return CVFalse;
				}
			}
			break;
		}
	}
	return CVTrue;
}


#endif /* CVGrid_h */
