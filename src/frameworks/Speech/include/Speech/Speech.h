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


#ifndef _Speech_H_
#define _Speech_H_

#import <Foundation/Foundation.h>

#import <Speech/AFDictationDelegate.h>
#import <Speech/SFSpeechRecognitionBufferDelegate.h>
#import <Speech/SFAcousticFeature.h>
#import <Speech/SFVoiceAnalytics.h>
#import <Speech/_SFSearchResult.h>
#import <Speech/SFSpeechRecognitionTask.h>
#import <Speech/_SFSpeechRecognitionBlockTask.h>
#import <Speech/_SFSpeechRecognitionDelegateTask.h>
#import <Speech/SFSpeechRecognitionResult.h>
#import <Speech/SFSpeechRecognizer.h>
#import <Speech/SFTranscriptionSegment.h>
#import <Speech/_SFSearchRequest.h>
#import <Speech/SFSpeechRecognitionRequest.h>
#import <Speech/SFSpeechURLRecognitionRequest.h>
#import <Speech/SFSpeechAudioBufferRecognitionRequest.h>
#import <Speech/SFTranscription.h>

void* SFIsTV(void);

#endif
