/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#ifndef _EventKit_H_
#define _EventKit_H_

#import <Foundation/Foundation.h>

#import <EventKit/CalDateRangeProtocol.h>
#import <EventKit/CalendarModelProtocol.h>
#import <EventKit/CalendarSourceModelProtocol.h>
#import <EventKit/EKCalendarProtocol.h>
#import <EventKit/EKChangeListenerDelegate.h>
#import <EventKit/EKEditingContextObserver.h>
#import <EventKit/EKEventOccurrenceProtocol.h>
#import <EventKit/EKFrozenMeltedPair.h>
#import <EventKit/EKGEORouteHypothesis.h>
#import <EventKit/EKIdentityProtocol.h>
#import <EventKit/EKJunkInvitationProtocol_Private.h>
#import <EventKit/EKMutableEventOccurrenceProtocol.h>
#import <EventKit/EKMutableOccurrenceProtocol.h>
#import <EventKit/EKOccurrenceProtocol.h>
#import <EventKit/EKProtocolAlarm.h>
#import <EventKit/EKProtocolAttachment.h>
#import <EventKit/EKProtocolCalendarNotification.h>
#import <EventKit/EKProtocolMutableAlarm.h>
#import <EventKit/EKProtocolMutableCalendarNotification.h>
#import <EventKit/EKProtocolMutableObject.h>
#import <EventKit/EKProtocolMutableParticipant.h>
#import <EventKit/EKProtocolMutableReminderOccurrence.h>
#import <EventKit/EKProtocolMutableSharee.h>
#import <EventKit/EKProtocolMutableStructuredLocation.h>
#import <EventKit/EKProtocolObject.h>
#import <EventKit/EKProtocolParticipant.h>
#import <EventKit/EKProtocolRecurrenceRule.h>
#import <EventKit/EKProtocolReminderOccurrence.h>
#import <EventKit/EKProtocolSharee.h>
#import <EventKit/EKProtocolStructuredLocation.h>
#import <EventKit/EKProtocolSuggestedEventInfo.h>
#import <EventKit/EKRecurrenceIdentifierProtocol.h>
#import <EventKit/EKSerializer.h>
#import <EventKit/EKUndoDelegate.h>
#import <EventKit/EventOccurrenceModelProtocol.h>
#import <EventKit/MutableCalendarSourceModelProtocol.h>
#import <EventKit/MutableEventOccurrenceModelProtocol.h>
#import <EventKit/MutableOccurrenceModelProtocol.h>
#import <EventKit/NSManagedObjectContextFaultingDelegate.h>
#import <EventKit/OccurrenceModelProtocol.h>
#import <EventKit/EKEventOccurrenceProtocol_Shared.h>
#import <EventKit/EKAvailabilitySpan.h>
#import <EventKit/EKAvailabilityCache.h>
#import <EventKit/EKUndoManager.h>
#import <EventKit/EKFrozenCalendarResourceChangeNotification.h>
#import <EventKit/EKEWSItemIDPredicate.h>
#import <EventKit/EKSerializableRecurrenceRule.h>
#import <EventKit/EKCalendarItem.h>
#import <EventKit/EKEventRangePredicate.h>
#import <EventKit/EKChangeSet.h>
#import <EventKit/EKUncompleteRecurringReminderOperation.h>
#import <EventKit/EKFrozenCalendarSharedCalendarNotification.h>
#import <EventKit/EKParticipantForSorting.h>
#import <EventKit/EKFrozenParticipant.h>
#import <EventKit/EKSyntheticGEORouteHypothesis.h>
#import <EventKit/EKEditingContextGroup.h>
#import <EventKit/EKRecurrenceSet.h>
#import <EventKit/EKContactEventRangePredicate.h>
#import <EventKit/EKChangeListener.h>
#import <EventKit/EKCalendarEventInvitationNotification.h>
#import <EventKit/EKReminder.h>
#import <EventKit/EKSerializableAlarm.h>
#import <EventKit/EKEventStore.h>
#import <EventKit/EKConflictDetails.h>
#import <EventKit/EKTempEventStoreFactory.h>
#import <EventKit/EKBackingStore.h>
#import <EventKit/EKRecents.h>
#import <EventKit/EKCalendarStoreBackingStore.h>
#import <EventKit/EKManagedObjectContext.h>
#import <EventKit/EKCoreDataFaultDelegate.h>
#import <EventKit/EKCalendar.h>
#import <EventKit/EKChangeTracker.h>
#import <EventKit/EKRelationChange.h>
#import <EventKit/EKChangeLog.h>
#import <EventKit/EKAttachment.h>
#import <EventKit/EKSecureKeyedArchiverSerializer.h>
#import <EventKit/EKObject.h>
#import <EventKit/EKFrozenCalendar.h>
#import <EventKit/EKFrozenCalendarItem.h>
#import <EventKit/EKFrozenCalendarEventInvitationNotification.h>
#import <EventKit/EKEventOccurrenceCache.h>
#import <EventKit/EKFrozenCalendarNotification.h>
#import <EventKit/EKSerializableObject.h>
#import <EventKit/EKCalendarProposeNewTimeNotification.h>
#import <EventKit/EKFrozenReminder.h>
#import <EventKit/EKFeatureSet.h>
#import <EventKit/EKRevertOperation.h>
#import <EventKit/EKFrozenObject.h>
#import <EventKit/EKAlarm.h>
#import <EventKit/EKFrozenCalendarInviteReplyNotification.h>
#import <EventKit/EKUnsliceOperation.h>
#import <EventKit/EKFrozenAlarm.h>
#import <EventKit/EKTravelEngineHypothesis.h>
#import <EventKit/EKObjectPersistentChangesModel.h>
#import <EventKit/EKPersistentHistoryTransactionChangeSummaryModel.h>
#import <EventKit/EKPersistentHistoryTransactionChangeSummary.h>
#import <EventKit/EKSource.h>
#import <EventKit/EKUndoableOperation.h>
#import <EventKit/EKSeriesDetails.h>
#import <EventKit/EKSliceDescription.h>
#import <EventKit/EKPostSliceDescription.h>
#import <EventKit/EKUndoSliceOutcome.h>
#import <EventKit/EKTravelEngineUtilities.h>
#import <EventKit/EKCalendarSharedCalendarNotification.h>
#import <EventKit/EKDataProtectionObserver.h>
#import <EventKit/EKSuggestedEventInfo.h>
#import <EventKit/EKUnrevertOperation.h>
#import <EventKit/EKEditingContext.h>
#import <EventKit/EKUndeleteFutureOperation.h>
#import <EventKit/EKAvailabilityUtilities.h>
#import <EventKit/EKTimedEventStorePurger.h>
#import <EventKit/EKParticipantToContactMatcher.h>
#import <EventKit/EKSerializableEvent.h>
#import <EventKit/EKObjectChangeSummarizer.h>
#import <EventKit/EKConferenceURLDetector.h>
#import <EventKit/EKRecurrenceIdentifier.h>
#import <EventKit/EKCalendarInviteReplyNotification.h>
#import <EventKit/EKFrozenOrganizer.h>
#import <EventKit/EKOccurrenceProtocol_Shared.h>
#import <EventKit/EKFrozenAttachment.h>
#import <EventKit/EKFrozenCalendarSource.h>
#import <EventKit/EKReadWriteLock.h>
#import <EventKit/EKLiveEditingStore.h>
#import <EventKit/EKCalendarPreferences.h>
#import <EventKit/EKPredicate.h>
#import <EventKit/EKDeleteOperation.h>
#import <EventKit/EKCalendarSuggestionNotification.h>
#import <EventKit/EKInviteeAlternativeTimeSearcher.h>
#import <EventKit/EKJunkInvitationProtocol_Shared.h>
#import <EventKit/EKObjectCreationCalendarStore.h>
#import <EventKit/EKCalendarProtocol_Shared.h>
#import <EventKit/EKPredicateSearch.h>
#import <EventKit/EKConflictScanner.h>
#import <EventKit/EKConflictScanJob.h>
#import <EventKit/EKEvent.h>
#import <EventKit/EKObjectCreationInMemorySource.h>
#import <EventKit/EKSaveOperation.h>
#import <EventKit/EKFrozenEvent.h>
#import <EventKit/EKRecurrenceDayOfWeek.h>
#import <EventKit/EKNaturalLanguageSuggestedEventPredicate.h>
#import <EventKit/EKActionsAccounting.h>
#import <EventKit/EKBatchChangeInfo.h>
#import <EventKit/EKEventSliceInfo.h>
#import <EventKit/EKRecurrenceChangeInfo.h>
#import <EventKit/EKRecurringReminderCompletedChangeInfo.h>
#import <EventKit/EKObjectValidationContext.h>
#import <EventKit/EKSerializableAttachment.h>
#import <EventKit/EKGeneralLookupPredicate.h>
#import <EventKit/EKCreateOperation.h>
#import <EventKit/EKFrozenSuggestedEventInfo.h>
#import <EventKit/EKCalendarNotification.h>
#import <EventKit/EKSerializableParticipant.h>
#import <EventKit/EKRecurrenceEnd.h>
#import <EventKit/EKObjectCreationInMemoryObject.h>
#import <EventKit/EKQueryEventPredicate.h>
#import <EventKit/EKAssistantEventPredicate.h>
#import <EventKit/EKRecurrenceRule.h>
#import <EventKit/EKFrozenCalendarSuggestionNotification.h>
#import <EventKit/EKFrozenRecurrenceRule.h>
#import <EventKit/EKDefaultSerializerFactory.h>
#import <EventKit/EKRecurrenceException.h>
#import <EventKit/EKPreferences.h>
#import <EventKit/EKParticipant.h>
#import <EventKit/EKFrozenCalendarProposeNewTimeNotification.h>
#import <EventKit/EKFrozenAttendee.h>
#import <EventKit/EKBackingStoreAccounting.h>
#import <EventKit/EKAttendee.h>
#import <EventKit/EKAvailabilityCachedSpan.h>
#import <EventKit/EKOrganizer.h>
#import <EventKit/EKQueryReminderPredicate.h>
#import <EventKit/EKAvailabilityCachedSpanRange.h>
#import <EventKit/EKObjectCreation.h>
#import <EventKit/EKStructuredLocation.h>
#import <EventKit/EKDiff.h>
#import <EventKit/EKFrozenStructuredLocation.h>
#import <EventKit/EKInviteeTimeSpan.h>
#import <EventKit/EKCalendarChangeHelper.h>
#import <EventKit/EKRepliedEventPredicate.h>
#import <EventKit/EKRecurrenceGenerator.h>
#import <EventKit/EKRecurrenceChangeManager.h>
#import <EventKit/EKObjectCreationInMemoryCalendar.h>
#import <EventKit/EKBackingObjectsTracker.h>
#import <EventKit/EKSuggestedEventPredicate.h>
#import <EventKit/EKRequestAvailabilityOperation.h>
#import <EventKit/EKSharee.h>
#import <EventKit/EKInMemoryBackingStore.h>
#import <EventKit/EKFrozenSharee.h>
#import <EventKit/EKObjectGroup.h>
#import <EventKit/EKCalendarResourceChangeNotification.h>
#import <EventKit/EKQueryPredicate.h>
#import <EventKit/EKInviteeAlternativeTime.h>
#import <EventKit/EKSerializableStructuredLocation.h>
#import <EventKit/EKDebugPreferences.h>
#import <EventKit/EKObjectCreationInMemory.h>
#import <EventKit/EKResliceOperation.h>

void* EKAutoCommentPrefix(void);
void* EKAvailabilitySpanTypeAsString(void);
void* EKBundle(void);
void* EKTravelEngineLogInitialize(void);
void* EKUIAttendeeUtils_AttendeeHasResponded(void);
void* EKUtils_AdjustedAttendeeAddress(void);
void* _bitmaskContainsOption(void);
void* _bitmaskExclusivelyMatchesOption(void);
void* _bitmaskMatchesExactlyOneOptionFromList(void);
void* _boolToString(void);
void* _validMoveEventRequirementsCombination(void);
void* barrierAsyncIfQueueNotNil(void);
void* barrierSyncBool(void);
void* barrierSyncIfQueueNotNil(void);
void* defaultValueForKey(void);
void* dispatch_async_main_if_necessary(void);
void* dispatch_queue_create_recursive_concurrent(void);
void* dispatch_sync_recursive(void);
void* resetStaticObjectCreationVariables(void);
void* stringFromEKEntityType(void);
void* stringFromEKSharingStatus(void);
void* syncBoolIfQueueNotNil(void);
void* syncIfQueueNotNil(void);

#endif
