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


#ifndef _OSAKit_H_
#define _OSAKit_H_

#import <Foundation/Foundation.h>

#import <OSAKit/NSBrowserDelegate.h>
#import <OSAKit/NSControlTextEditingDelegate.h>
#import <OSAKit/NSOutlineViewDataSource.h>
#import <OSAKit/NSOutlineViewDelegate.h>
#import <OSAKit/NSSplitViewDelegate.h>
#import <OSAKit/NSTableViewDataSource.h>
#import <OSAKit/NSTableViewDelegate.h>
#import <OSAKit/NSTextStorageDelegate.h>
#import <OSAKit/NSToolbarDelegate.h>
#import <OSAKit/OSACompletionItem.h>
#import <OSAKit/OSAMergedSortedArray.h>
#import <OSAKit/OSATextStorage.h>
#import <OSAKit/OSAScriptTextStorage.h>
#import <OSAKit/OSAScriptAssistant.h>
#import <OSAKit/OSAScriptAssistantView.h>
#import <OSAKit/OSAScriptAssistantWindow.h>
#import <OSAKit/OSAScriptElement.h>
#import <OSAKit/OSAGlobalElement.h>
#import <OSAKit/OSAPropertyElement.h>
#import <OSAKit/OSAScriptObjectElement.h>
#import <OSAKit/OSALongCommentElement.h>
#import <OSAKit/OSAShortCommentElement.h>
#import <OSAKit/OSATextElement.h>
#import <OSAKit/OSAUnknownElement.h>
#import <OSAKit/OSAScriptGenerator.h>
#import <OSAKit/OSAScriptParser.h>
#import <OSAKit/OSAControlElement.h>
#import <OSAKit/OSAConsideringElement.h>
#import <OSAKit/OSAIgnoringElement.h>
#import <OSAKit/OSAIfElement.h>
#import <OSAKit/OSAElseIfElement.h>
#import <OSAKit/OSAElseElement.h>
#import <OSAKit/OSAEndIfElement.h>
#import <OSAKit/OSARepeatElement.h>
#import <OSAKit/OSATellElement.h>
#import <OSAKit/OSATryElement.h>
#import <OSAKit/OSAOnErrorElement.h>
#import <OSAKit/OSAEndTryElement.h>
#import <OSAKit/OSAUsingTermsFromElement.h>
#import <OSAKit/OSAWithTimeoutElement.h>
#import <OSAKit/OSAWithTransactionElement.h>
#import <OSAKit/OSAHandlerElement.h>
#import <OSAKit/OSACommandHandlerElement.h>
#import <OSAKit/OSAEventHandlerElement.h>
#import <OSAKit/OSATerminologyManager.h>
#import <OSAKit/OSAActiveProc.h>
#import <OSAKit/OSASendProc.h>
#import <OSAKit/OSARecordingController.h>
#import <OSAKit/OSAScriptAssistantDataSource.h>
#import <OSAKit/OSAScriptAssistantWordBasedDataSource.h>
#import <OSAKit/OSAScriptAssistantStrategy.h>
#import <OSAKit/OSAScriptAssistantKeywordStrategy.h>
#import <OSAKit/OSAScriptAssistantTerminologyStrategy.h>
#import <OSAKit/OSAScriptAssistantParserStrategy.h>
#import <OSAKit/OSAScriptAssistantScannerStrategy.h>
#import <OSAKit/OSANameCompletionItem.h>
#import <OSAKit/OSAKeywordCompletionItem.h>
#import <OSAKit/OSATerminologyCompletionItem.h>
#import <OSAKit/OSAComponentInstance.h>
#import <OSAKit/OSALanguagePrivate.h>
#import <OSAKit/OSALanguage.h>
#import <OSAKit/OSAScript.h>
#import <OSAKit/OSAScriptPrivate.h>
#import <OSAKit/OSAScriptController.h>
#import <OSAKit/OSAScriptControllerPrivate.h>
#import <OSAKit/OSAScriptView.h>
#import <OSAKit/OSAScriptViewPrivate.h>
#import <OSAKit/OSADictionaryController.h>
#import <OSAKit/OSADictionaryControllerPrivate.h>
#import <OSAKit/OSADictionaryHistoryItem.h>
#import <OSAKit/OSADictionaryHistory.h>
#import <OSAKit/WebKitSoftLinking.h>
#import <OSAKit/OSADictionaryView.h>
#import <OSAKit/OSADictionaryViewPrivate.h>
#import <OSAKit/OSASplitView.h>
#import <OSAKit/OSADictionary.h>
#import <OSAKit/OSADictionaryPrivate.h>
#import <OSAKit/OSADefinition.h>
#import <OSAKit/OSAAccessorDefinition.h>
#import <OSAKit/OSAClassDefinition.h>
#import <OSAKit/OSACocoaDefinition.h>
#import <OSAKit/OSAVerbDefinition.h>
#import <OSAKit/OSACommandDefinition.h>
#import <OSAKit/OSAContentsDefinition.h>
#import <OSAKit/OSADirectParameterDefinition.h>
#import <OSAKit/OSADocumentationDefinition.h>
#import <OSAKit/OSAElementDefinition.h>
#import <OSAKit/OSAEnumerationDefinition.h>
#import <OSAKit/OSAEnumeratorDefinition.h>
#import <OSAKit/OSAEventDefinition.h>
#import <OSAKit/OSAParameterDefinition.h>
#import <OSAKit/OSAPropertyDefinition.h>
#import <OSAKit/OSARespondsToDefinition.h>
#import <OSAKit/OSASuiteDefinition.h>
#import <OSAKit/OSASynonymDefinition.h>
#import <OSAKit/OSADictionaryWindowController.h>
#import <OSAKit/OSAImageTextCell.h>
#import <OSAKit/OSABrowserCell.h>
#import <OSAKit/CPDelayedUpdaterInfo.h>
#import <OSAKit/CPDelayedUpdateManager.h>
#import <OSAKit/OSALanguageInstancePrivate.h>
#import <OSAKit/OSALanguageInstance.h>

void* ActiveProc(void);
void* FilterProc(void);
void* IdleProc(void);
void* OSA_IsHardLineBreakUnichar(void);
void* OSA_attributedStringFromDescriptor(void);
void* OSA_attributedStringWithCarbonTEData(void);
void* OSA_carbonTEDataFromAttributedString(void);
void* OSA_findMatchingBraceForRangeInString(void);
void* OSA_isClosingBrace(void);
void* OSA_isOpeningBrace(void);
void* OSA_matchingDelimiter(void);
void* OSA_numberOfLeadingSpacesFromRangeInString(void);
void* OSA_tabStopArrayForFontAndTabWidth(void);
void* OSA_tabbifiedStringWithNumberOfSpaces(void);
void* SendProc(void);
void* SetSegmentDescriptions(void);
void* _OSA_identifierChars(void);
void* _osa_fileRefFromPath(void);
void* _osa_pathFromFSRef(void);
void* scriptAssistantCaseCompareItems(void);

#endif
