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

void* NStatManagerAddAllRoutes(void);
void* NStatManagerAddAllRoutesWithFilter(void);
void* NStatManagerAddAllTCP(void);
void* NStatManagerAddAllTCPWithFilter(void);
void* NStatManagerAddAllTCPWithOptions(void);
void* NStatManagerAddAllUDP(void);
void* NStatManagerAddAllUDPWithFilter(void);
void* NStatManagerAddAllUDPWithOptions(void);
void* NStatManagerCreate(void);
void* NStatManagerCreateInterfaceSource(void);
void* NStatManagerCreateRouteSource(void);
void* NStatManagerCreateSysinfoSource(void);
void* NStatManagerCreateTCPSource(void);
void* NStatManagerCreateUDPSource(void);
void* NStatManagerDestroy(void);
void* NStatManagerQueryAllSources(void);
void* NStatManagerQueryAllSourcesDescriptions(void);
void* NStatManagerQueryAllSourcesUpdate(void);
void* NStatManagerSetFlags(void);
void* NStatManagerSetInterfaceSourceFD(void);
void* NStatManagerSetInterfaceTraceFD(void);
void* NStatManagerSetQueuePriority(void);
void* NStatRouteValuesForIPv4Host(void);
void* NStatRouteValuesForIPv6Host(void);
void* NStatSourceCopyCounts(void);
void* NStatSourceCopyProperties(void);
void* NStatSourceCopyProperty(void);
void* NStatSourceQueryCounts(void);
void* NStatSourceQueryDescription(void);
void* NStatSourceQueryUpdate(void);
void* NStatSourceRemove(void);
void* NStatSourceSetCountsBlock(void);
void* NStatSourceSetDescriptionBlock(void);
void* NStatSourceSetEventsBlock(void);
void* NStatSourceSetFilter(void);
void* NStatSourceSetRemovedBlock(void);

#import <Foundation/Foundation.h>
#import <NetworkStatistics/NWSnapshotSource.h>
#import <NetworkStatistics/NWSnapshotter.h>
#import <NetworkStatistics/NWRouteSnapshotter.h>
#import <NetworkStatistics/NWSnapshot.h>
#import <NetworkStatistics/NWRouteSnapshot.h>
#import <NetworkStatistics/NWStatisticsManager.h>
#import <NetworkStatistics/NWStatisticsDelegateBlockWrapper.h>
#import <NetworkStatistics/NWStatisticsInterfaceSource.h>
#import <NetworkStatistics/NWStatisticsSource.h>
#import <NetworkStatistics/NWStatisticsTCPSource.h>
#import <NetworkStatistics/NWStatisticsUDPSource.h>
#import <NetworkStatistics/NWStatisticsRouteSource.h>
