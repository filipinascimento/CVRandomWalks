//
//  CVMaleabilityApplication.c
//  CVNetwork
//
//  Created by Filipi Nascimento Silva on 10/29/16.
//  Copyright © 2016 Filipi Nascimento Silva. All rights reserved.
//

#include "CVNetwork.h"
#include "CVNetworkCentrality.h"
#include "CVDistribution.h"
#include "CVConcentricStructure.h"
#include "CVNetworkMeasurements.h"
#include "CVRandomRewiring.h"
#include "CVSet.h"




int main(int argc, char *argv[]){
    CVRandomSeedDev();
    CVString networkPath = NULL;
    CVString outputPath = NULL;
    
    CVFloat q;
    CVFloat p;
    CVIndex w;//sentence size
    CVIndex m;//number of walks per node

    if(argc!=7){
        printf("unexpected number of arguments (%d)\n",(int)argc);
        return EXIT_FAILURE;
    }else{
        //CVRandomWalk q p w m filename.xnet
        q = strtof(argv[1],NULL);
        p = strtof(argv[2],NULL);
        w = strtol(argv[3],NULL,10);
        m = strtol(argv[4],NULL,10);
        networkPath = argv[5];
        outputPath = argv[6];
    }
//    p = 1.0;
//    q = 1.0;
//    w=80;
//    m=20;
    // networkPath = "/Users/filipi/Dropbox/Software/CVRandomWalks/venuesNetworkBackbone.xnet";
    // outputPath = "/Users/filipi/Dropbox/Software/CVRandomWalks/venuesNetworkBackbone_sentences.txt";
    FILE* networkFile = fopen(networkPath,"r");
    if(!networkFile){
        printf("Cannot load file \"%s\". \n",networkPath);
    }
    CVNetwork* network = CVNewNetworkFromXNETFile(networkFile);
    fclose(networkFile);
    
    CVSize verticesCount = network->verticesCount;
    CVSize sentencesCount = network->verticesCount*m;
    CVIndex* sentences = calloc(sentencesCount*w,sizeof(CVIndex)); //all indices are shifted by 1

    unsigned int* seeds = calloc(sentencesCount,sizeof(unsigned int));
    
    unsigned int initialSeed = (unsigned int)time(NULL);
    for(CVIndex sentenceIndex=0; sentenceIndex<sentencesCount;sentenceIndex++){
        seeds[sentenceIndex] = rand_r(&initialSeed)^(unsigned int)sentenceIndex;
    }
    
    CVInteger* currentProgress = calloc(1,sizeof(CVInteger));
    
    CVParallelForStart(distributionsLoop, sentenceIndex, sentencesCount){

        if(CVAtomicIncrementInteger(currentProgress)%1000==0){
            CVParallelLoopCriticalRegionStart(distributionsLoop){
                printf("%"CVIndexScan"/%"CVIndexScan" (%.2f%%)                                                                 \r", (CVIndex)(*currentProgress),sentencesCount,(*currentProgress)/(float)(sentencesCount-1)*100.0);
                fflush(stdout);
            }CVParallelLoopCriticalRegionEnd(distributionsLoop);
        }

        CVIndex currentNode = sentenceIndex%verticesCount;
        CVIndex previousNode = currentNode;
        CVUIntegerSet* previousNeighborsSet = CVNewUIntegerSet();
        unsigned int* seedRef = seeds+sentenceIndex;
        sentences[sentenceIndex*w+0]=currentNode+1; //Always shifted by 1;
        if(q==1.0 && p==1.0){
            for(CVIndex walkStep=1; walkStep<w;walkStep++){//
                CVIndex* neighbors = network->vertexEdgesLists[currentNode];
                CVIndex neighborCount = network->vertexNumOfEdges[currentNode];
                CVIndex* neighEdges = network->vertexEdgesIndices[currentNode];
                if(neighborCount>0){
                    CVFloat* probabilities = calloc(neighborCount,sizeof(CVFloat));
                    for (CVIndex neighIndex = 0; neighIndex < neighborCount; neighIndex++){
                        CVIndex edgeIndex = neighEdges[neighIndex];
                        CVFloat weight = 1.0;
                        
                        if(network->edgeWeighted){
                            weight = network->edgesWeights[edgeIndex];
                        }
                        probabilities[neighIndex] = weight;
                    }
                    
                    CVDouble choice = ((double)rand_r(seedRef) / RAND_MAX);
                    CVDistribution* distribution = CVCreateDistribution(probabilities,NULL,neighborCount);

                    previousNode = currentNode;
                    currentNode = neighbors[CVDistributionIndexForChoice(distribution,choice)];
                    sentences[sentenceIndex*w+walkStep]=currentNode+1; //Always shifted by 1;
                    CVDestroyDistribution(distribution);
                    free(probabilities);
                }else{
                    break;
                }
            }
        }else{
            for(CVIndex walkStep=1; walkStep<w;walkStep++){//
                CVIndex* neighbors = network->vertexEdgesLists[currentNode];
                CVIndex neighborCount = network->vertexNumOfEdges[currentNode];
                CVIndex* neighEdges = network->vertexEdgesIndices[currentNode];
                if(neighborCount>0){
                    CVFloat* probabilities = calloc(neighborCount,sizeof(CVFloat));
                    for (CVIndex neighIndex = 0; neighIndex < neighborCount; neighIndex++){
                        CVIndex edgeIndex = neighEdges[neighIndex];
                        CVIndex candidateIndex = neighbors[neighIndex];
                        CVFloat weight = 1.0;
                        
                        if(network->edgeWeighted){
                            weight = network->edgesWeights[edgeIndex];
                        }

                        if(neighbors[neighIndex]==previousNode){
                            probabilities[neighIndex] = weight*1/p;
                        }else if(CVUIntegerSetHas(previousNeighborsSet,candidateIndex)){
                            probabilities[neighIndex] = weight;
                        }else{
                            probabilities[neighIndex] = weight*1/q;
                        }
                    }
                    
                    CVDouble choice = ((double)rand_r(seedRef) / RAND_MAX);
                    CVDistribution* distribution = CVCreateDistribution(probabilities,NULL,neighborCount);

                    previousNode = currentNode;
                    currentNode = neighbors[CVDistributionIndexForChoice(distribution,choice)];
                    sentences[sentenceIndex*w+walkStep]=currentNode+1; //Always shifted by 1;
                    CVDestroyDistribution(distribution);
                    free(probabilities);

                    CVUIntegerSetClear(previousNeighborsSet);
                    for (CVIndex neighIndex = 0; neighIndex < neighborCount; neighIndex++){
                        CVUIntegerSetAdd(previousNeighborsSet,neighbors[neighIndex]);
                    }
                }else{
                    break;
                }
            }
        }
        CVUIntegerSetDestroy(previousNeighborsSet);
    }CVParallelForEnd(distributionsLoop);
    
    FILE* outputFile = fopen(outputPath,"w");
    if(!outputFile){
        printf("Cannot load file \"%s\". \n",networkPath);
    }

    for(CVIndex sentenceIndex=0; sentenceIndex<sentencesCount;sentenceIndex++){
        for(CVIndex walkStep=0; walkStep<w;walkStep++){
            CVIndex nodeIndexWithOffset = sentences[sentenceIndex*w+walkStep];
            if(nodeIndexWithOffset>0){
                fprintf(outputFile,"%"CVUIntegerScan" ",(nodeIndexWithOffset-1));
            }else{
                break;
            }
        }
        fprintf(outputFile,"\n");
    }
    
    CVNetworkDestroy(network);
    fclose(outputFile);
    return EXIT_SUCCESS;
}


