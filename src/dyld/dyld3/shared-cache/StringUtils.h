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


#ifndef StringUtils_h
#define StringUtils_h

#include <string>

inline bool startsWith(const std::string& str, const std::string& prefix)
{
    return std::mismatch(prefix.begin(), prefix.end(), str.begin()).first == prefix.end();
}

inline bool startsWith(const std::string_view& str, const std::string_view& prefix)
{
    return std::mismatch(prefix.begin(), prefix.end(), str.begin()).first == prefix.end();
}

inline bool endsWith(const std::string& str, const std::string& suffix)
{
    std::size_t index = str.find(suffix, str.size() - suffix.size());
    return (index != std::string::npos);
}

inline bool contains(const std::string& str, const std::string& search)
{
    std::size_t index = str.find(search);
    return (index != std::string::npos);
}

inline char hexDigit(uint8_t value)
{
    if ( value < 10 )
        return '0' + value;
    else
        return 'a' + value - 10;
}

inline void bytesToHex(const uint8_t* bytes, size_t byteCount, char buffer[])
{
    char* p = buffer;
    for (size_t i=0; i < byteCount; ++i) {
        *p++ = hexDigit(bytes[i] >> 4);
        *p++ = hexDigit(bytes[i] & 0x0F);
    }
    *p++ =  '\0';
}

inline void putHexNibble(uint8_t value, char*& p)
{
    if ( value < 10 )
        *p++ = '0' + value;
    else
        *p++ = 'A' + value - 10;
}

inline void putHexByte(uint8_t value, char*& p)
{
    value &= 0xFF;
    putHexNibble(value >> 4,   p);
    putHexNibble(value & 0x0F, p);
}

inline uint8_t hexCharToUInt(const char hexByte, uint8_t& value) {
    if (hexByte >= '0' && hexByte <= '9') {
        value = hexByte - '0';
        return true;
    } else if (hexByte >= 'A' && hexByte <= 'F') {
        value = hexByte - 'A' + 10;
        return true;
    } else if (hexByte >= 'a' && hexByte <= 'f') {
        value = hexByte - 'a' + 10;
        return true;
    }

    return false;
}

inline uint64_t hexToUInt64(const char* startHexByte, const char** endHexByte) {
    const char* scratch;
    if (endHexByte == nullptr) {
        endHexByte = &scratch;
    }
    if (startHexByte == nullptr)
        return 0;
    uint64_t retval = 0;
    if (startHexByte[0] == '0' &&  startHexByte[1] == 'x') {
        startHexByte +=2;
    }
    *endHexByte = startHexByte + 16;

    //FIXME overrun?
    for (uint32_t i = 0; i < 16; ++i) {
        uint8_t value;
        if (!hexCharToUInt(startHexByte[i], value)) {
            *endHexByte = &startHexByte[i];
            break;
        }
        retval = (retval << 4) + value;
    }
    return retval;
}

inline bool hexToBytes(const char* startHexByte, uint32_t length, uint8_t buffer[]) {
    if (startHexByte == nullptr)
        return false;
    const char *currentHexByte = startHexByte;
    for (uint32_t i = 0; i < length; ++i) {
        uint8_t value;
        if (!hexCharToUInt(currentHexByte[i], value)) {
            return false;
        }
        if (i%2 == 0) {
            buffer[i/2] = value << 4;
        } else {
            buffer[(i-1)/2] |= value;
        }
    }
    return true;
}

#endif // StringUtils_h

