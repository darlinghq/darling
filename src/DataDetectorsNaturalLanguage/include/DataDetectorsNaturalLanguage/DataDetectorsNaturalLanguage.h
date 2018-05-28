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


#ifndef _DataDetectorsNaturalLanguage_H_
#define _DataDetectorsNaturalLanguage_H_

#import <Foundation/Foundation.h>

#import <DataDetectorsNaturalLanguage/IPFeatureScanner.h>
#import <DataDetectorsNaturalLanguage/IPFeatureMailScanner.h>
#import <DataDetectorsNaturalLanguage/IPFeatureTextMessageScanner.h>
#import <DataDetectorsNaturalLanguage/IPDataDetectorsFeatureExtractor.h>
#import <DataDetectorsNaturalLanguage/IPFeatureExtractor.h>
#import <DataDetectorsNaturalLanguage/IPFeatureManager.h>
#import <DataDetectorsNaturalLanguage/IPTenseDetector.h>
#import <DataDetectorsNaturalLanguage/IPFeatureData.h>
#import <DataDetectorsNaturalLanguage/IPKeywordFeatureExtractor.h>
#import <DataDetectorsNaturalLanguage/IPCircularBufferArray.h>
#import <DataDetectorsNaturalLanguage/IPRegexToolbox.h>
#import <DataDetectorsNaturalLanguage/IPPerson.h>
#import <DataDetectorsNaturalLanguage/IPTextMessageConversation.h>
#import <DataDetectorsNaturalLanguage/IPSentenceFeatureExtractor.h>
#import <DataDetectorsNaturalLanguage/IPQuoteParser.h>
#import <DataDetectorsNaturalLanguage/IPFeature.h>
#import <DataDetectorsNaturalLanguage/IPMessageXMLParser.h>
#import <DataDetectorsNaturalLanguage/IPMessageUnit.h>
#import <DataDetectorsNaturalLanguage/IPMessage.h>
#import <DataDetectorsNaturalLanguage/IPFeatureSentenceFragment.h>
#import <DataDetectorsNaturalLanguage/IPFeatureSentence.h>
#import <DataDetectorsNaturalLanguage/IPFeatureKeyword.h>
#import <DataDetectorsNaturalLanguage/IPEventClassificationType.h>

void* IPInitLogging(void);
void* humanReadableClusterType(void);
void* humanReadableFeatureType(void);

#endif
