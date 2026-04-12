/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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


#ifndef _PubSub_H_
#define _PubSub_H_

#import <Foundation/Foundation.h>

#import <PubSub/PSMessageQueueDelegate.h>
#import <PubSub/PSEnclosureInternal.h>
#import <PubSub/PSEnclosure.h>
#import <PubSub/PSFeedSettingsInternal.h>
#import <PubSub/PSFeedSettings.h>
#import <PubSub/PSClientInternal.h>
#import <PubSub/PSClient.h>
#import <PubSub/PSContentInternal.h>
#import <PubSub/PSContent.h>
#import <PubSub/PSFeedInternal.h>
#import <PubSub/PSFeed.h>
#import <PubSub/PSEntryInternal.h>
#import <PubSub/PSEntry.h>
#import <PubSub/PSAuthorInternal.h>
#import <PubSub/PSAuthor.h>
#import <PubSub/PSUtils.h>
#import <PubSub/PSLink.h>
#import <PubSub/PSRegularExpression.h>
#import <PubSub/PSInternal.h>
#import <PubSub/PSSCGIProtocol.h>
#import <PubSub/PSFeedURLUtils.h>
#import <PubSub/PSSyncManager.h>
#import <PubSub/PSMessage.h>
#import <PubSub/PSMessageQueue.h>
#import <PubSub/PSTempDelegate.h>
#import <PubSub/PSDotMacOperation.h>
#import <PubSub/PSDotMacGetOperation.h>
#import <PubSub/PSDotMacPutOperation.h>
#import <PubSub/PSOperation.h>
#import <PubSub/PSGetOperation.h>
#import <PubSub/PSPutOperation.h>
#import <PubSub/PSAggregate.h>

void* PSAgentFrameworkMain(void);

#endif
