//
//  CVRandomRewiring.h
//  CVNetwork
//
//  Created by Filipi Nascimento Silva on 10/30/16.
//  Copyright © 2016 Filipi Nascimento Silva. All rights reserved.
//

#ifndef CVRandomRewiring_h
#define CVRandomRewiring_h

#include "CVNetwork.h"

typedef struct {
	CVIndex previousFrom;
	CVIndex previousTo;
	CVIndex newFrom;
	CVIndex newTo;
	CVIndex edgeIndex;
} CVNetworkRewireEntry;

CVNetworkRewireEntry CVNetworkRandomRewireEntry(const CVNetwork* theNetwork);
void CVNetworkDoRewire(CVNetwork* theNetwork, CVNetworkRewireEntry entry);
void CVNetworkUndoRewire(CVNetwork* theNetwork, CVNetworkRewireEntry entry);
CVNetwork* CVNewNetworkWithNetworkAndRewire(const CVNetwork* originalNetwork, CVNetworkRewireEntry rewireEntry);

CVNetwork* CVNewNetworkByRemovingEdgeFast(const CVNetwork* originalNetwork, CVIndex edgeIndex);
CVNetwork* CVNewNetworkByAddingEdgeFast(const CVNetwork* originalNetwork, CVIndex fromIndex,CVIndex toIndex);




#endif /* CVRandomRewiring_h */
