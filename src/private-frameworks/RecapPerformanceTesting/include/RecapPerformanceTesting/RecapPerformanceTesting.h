/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _RecapPerformanceTesting_H_
#define _RecapPerformanceTesting_H_

#import <Foundation/Foundation.h>

// Apple's private performance-test harness. Apps link it for their
// -application:runTest:options: self-test hooks; those tests are never run on Darling.

@protocol RPTComposer <NSObject>
@end

@protocol RPTTestParameters <NSObject>
@property (nonatomic, copy) NSString *testName;
@end

@interface RPTTestRunner : NSObject
+ (BOOL)runTestWithParameters:(id<RPTTestParameters>)parameters;
- (BOOL)runTestWithParameters:(id<RPTTestParameters>)parameters;
@end

@interface RPTResizeTestParameters : NSObject <RPTTestParameters>
@property (nonatomic, copy) NSString *testName;
@property (nonatomic, readonly) id window;
- (instancetype)initWithTestName:(NSString *)testName window:(id)window completionHandler:(void (^)(void))completionHandler;
@end

@interface RPTScrollViewTestParameters : NSObject <RPTTestParameters>
@property (nonatomic, copy) NSString *testName;
@property (nonatomic, readonly) id scrollView;
- (instancetype)initWithTestName:(NSString *)testName scrollView:(id)scrollView completionHandler:(void (^)(void))completionHandler;
@end

#endif
