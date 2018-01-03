/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _SpotlightDaemon_H_
#define _SpotlightDaemon_H_

#import <Foundation/Foundation.h>

#import <SpotlightDaemon/CSSearchableIndexInterface.h>
#import <SpotlightDaemon/CSSearchableIndexServiceInterface.h>
#import <SpotlightDaemon/MDIndexer.h>
#import <SpotlightDaemon/SDEventHandlerProvider.h>
#import <SpotlightDaemon/SPQueryJob.h>
#import <SpotlightDaemon/SPQueryResultsQueue.h>
#import <SpotlightDaemon/SPCoreSpotlightTask.h>
#import <SpotlightDaemon/MDAgent.h>
#import <SpotlightDaemon/SPCoreSpotlightIndexerTask.h>
#import <SpotlightDaemon/_SPBucketSet.h>
#import <SpotlightDaemon/SPConcreteCoreSpotlightIndexer.h>
#import <SpotlightDaemon/SPCoreSpotlightIndexer.h>
#import <SpotlightDaemon/SpotlightReceiverConnection.h>
#import <SpotlightDaemon/SpotlightSender.h>
#import <SpotlightDaemon/SDTraceItem.h>
#import <SpotlightDaemon/SDTrace.h>
#import <SpotlightDaemon/SDEventMonitor.h>
#import <SpotlightDaemon/SDMigrationMonitor.h>
#import <SpotlightDaemon/CSSearchClientConnectionKey.h>
#import <SpotlightDaemon/CSSearchClientConnection.h>
#import <SpotlightDaemon/CSSearchAgent.h>
#import <SpotlightDaemon/CSIndexClientConnectionKey.h>
#import <SpotlightDaemon/CSIndexClientConnection.h>
#import <SpotlightDaemon/CSIndexAgent.h>
#import <SpotlightDaemon/SPCoreSpotlightExtensionsTask.h>
#import <SpotlightDaemon/CSRecieverState.h>
#import <SpotlightDaemon/MDSearchableIndexService.h>

void* BulkDeleteAttributes(void);
void* SDTraceAdd(void);
void* SDTraceDescription(void);
void* SDTransactionCreate(void);
void* SDTransactionDone(void);
void* SISynchedOpWithBlock(void);
void* SISynchedOpWithBlockPropagatingPriority(void);
void* blockRoutine(void);
void* delayedOpBlockCompleteCallback(void);
void* startAgents(void);
void* startAllAgents(void);
void* startIndexAgent(void);
void* syncFinished(void);
void* unpauseCallback(void);

#endif
