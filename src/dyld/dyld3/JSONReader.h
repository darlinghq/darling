/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */



#ifndef __JSON_READER_H__
#define __JSON_READER_H__

#include "JSON.h"

class Diagnostics;

namespace dyld3 {
namespace json {

Node readJSON(Diagnostics& diags, const char* filePath);

// Given a map node, returns the node representing the given value.
// If it is missing, returns a sentinel node and sets an error on the diagnostic
const Node& getRequiredValue(Diagnostics& diags, const Node& node, const char* key);

// Given a map node, returns the node representing the given value.
// If it is missing, return nullptr.
const Node* getOptionalValue(Diagnostics& diags, const Node& node, const char* key);

// Parses an int from the given node, or throws an error if its not an integer payload
uint64_t parseRequiredInt(Diagnostics& diags, const Node& node);

// Parses a string from the given node, or throws an error if its not a string payload
const std::string& parseRequiredString(Diagnostics& diags, const Node& node);


} // namespace json
} // namespace dyld3


#endif // __JSON_READER_H__
