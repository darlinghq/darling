/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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

/*
 Local-node implementation of the OpenDirectory entry points that
 pam_opendirectory imports. The generated stubs in CFOpenDirectory.c return
 NULL/false for all of them, so ODNodeCreateWithNodeType fails, pam returns
 PAM_SERVICE_ERR, and login can never succeed whatever the password hash says.

 ODRecordRef/ODNodeRef/ODSessionRef are opaque (struct __ODRecord and friends),
 so they are backed here by the structures below. The node reads the same local
 passwd database a login would consult, and verification delegates to the
 platform crypt(3) with the hash's own salt. Nothing here can succeed without a
 real password match: a missing record, a missing hash, or a crypt failure all
 return false.
*/

#include "../include/generated-stubs.h"
#include <CFOpenDirectory/CFOpenDirectoryPriv.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CFOpenDirectory/CFOpenDirectoryConstants.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The SDK shipped with this tree has no <crypt.h>, so the compiler would fall
// through to the host's glibc one and fail on its own prototypes. libSystem
// exports crypt(3) (verified: `T _crypt` in libsystem_c.dylib), so declare the
// one function needed here rather than pulling in a foreign header.
extern char *crypt(const char *key, const char *salt);

// Set by the constructor in CFOpenDirectory.c, shared so STUB_VERBOSE traces both.
extern int verbose;

// The passwd databases a local node reads, in priority order. The first that
// parses wins, matching how a local node resolves its backing store.
static const char * const passwdPaths[] = {
    "/private/etc/master.passwd",
    "/private/etc/passwd",
};

// A record is one passwd entry. The AuthenticationAuthority value keeps the
// full "scheme$salt$hash" string, because crypt(3) needs the scheme and salt
// and a verifier has nothing else to go on.
struct __ODRecord {
    CFStringRef name;          // primary name, e.g. "cristi"
    CFStringRef authAuthority; // full hash string from the passwd file
};

struct __ODNode {
    CFStringRef name;
};

struct __ODSession {
    int placeholder;
};

static CFStringRef copyCFString(const char *s)
{
    return s ? CFStringCreateWithCString(kCFAllocatorDefault, s,
                                         kCFStringEncodingUTF8) : NULL;
}

static char *cStringOf(CFStringRef s)
{
    if (s == NULL)
        return NULL;
    const char *utf8 = CFStringGetCStringPtr(s, kCFStringEncodingUTF8);
    if (utf8 != NULL)
        return strdup(utf8);
    CFIndex length = CFStringGetLength(s);
    CFIndex max = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    char *buffer = malloc((size_t)max);
    if (buffer == NULL)
        return NULL;
    if (!CFStringGetCString(s, buffer, max, kCFStringEncodingUTF8)) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

// Reports a POSIX-domain error to the caller. The reason is only written to the
// trace, because CFErrorCreate here takes a userInfo dictionary and building a
// throwaway one per failure would only to be discarded unread.
static void setODError(CFErrorRef *error, const char *reason)
{
    if (verbose)
        fprintf(stderr, "ODLocal: %s\n", reason);
    if (error != NULL)
        *error = CFErrorCreate(kCFAllocatorDefault, kCFErrorDomainPOSIX, EINVAL, NULL);
}

// Splits one colon-delimited passwd line into fields, returning how many were
// found. A comment or malformed line yields 0.
static int splitPasswdLine(char *line, char **fields, int maxFields)
{
    int index = 0;
    char *cursor = line;

    while (index < maxFields) {
        fields[index++] = cursor;
        char *separator = strchr(cursor, ':');
        if (separator == NULL)
            break;
        *separator = '\0';
        cursor = separator + 1;
    }

    // A real entry has at least name and password, and the name is never empty.
    if (index < 2 || fields[0][0] == '\0')
        return 0;

    return index;
}

// Reads the AuthenticationAuthority for recordName, or NULL when there is no
// such record or it carries no hash.
//
// Two layouts are in play and both are accepted, because a record can be written
// by either tool: BSD master.passwd carries the hash in an eleventh field with
// field 1 left as "*", while the passwd file shipped in a Darling prefix has ten
// fields with the hash directly in field 1.
static CFStringRef copyAuthAuthorityForRecord(const char *recordName)
{
    for (size_t i = 0; i < sizeof(passwdPaths) / sizeof(passwdPaths[0]); i++) {
        FILE *f = fopen(passwdPaths[i], "r");
        if (f == NULL)
            continue;

        char line[4096];
        CFStringRef result = NULL;

        while (fgets(line, sizeof(line), f) != NULL) {
            char *newline = strchr(line, '\n');
            if (newline != NULL)
                *newline = '\0';
            if (line[0] == '#' || line[0] == '\0')
                continue;

            char *fields[11];
            int count = splitPasswdLine(line, fields, 11);
            if (count == 0)
                continue;

            if (strcmp(fields[0], recordName) != 0)
                continue;

            // Prefer the dedicated hash field when the line is long enough to
            // have one, otherwise fall back to the password field itself.
            const char *hash = NULL;
            if (count >= 11)
                hash = fields[10];
            else
                hash = fields[1];

            if (hash == NULL || hash[0] == '\0' || hash[0] == '*' ||
                hash[0] == '!')
                break;   // account exists but cannot authenticate

            result = copyCFString(hash);
            break;
        }

        fclose(f);

        if (result != NULL)
            return result;
    }

    return NULL;
}

// Builds a record for recordName, or NULL when the passwd database has no such
// entry with a usable hash.
static ODRecordRef createLocalRecord(const char *recordName)
{
    if (recordName == NULL || recordName[0] == '\0')
        return NULL;

    CFStringRef authority = copyAuthAuthorityForRecord(recordName);
    if (authority == NULL)
        return NULL;

    struct __ODRecord *record = calloc(1, sizeof(*record));
    if (record == NULL) {
        CFRelease(authority);
        return NULL;
    }

    record->name = copyCFString(recordName);
    record->authAuthority = authority;

    if (record->name == NULL) {
        CFRelease(authority);
        free(record);
        return NULL;
    }

    return record;
}

static struct __ODRecord *recordOf(ODRecordRef record)
{
    return (struct __ODRecord *) record;
}

static struct __ODNode *nodeOf(ODNodeRef node)
{
    return (struct __ODNode *) node;
}

#pragma mark - Session

ODSessionRef ODSessionCreate(CFAllocatorRef allocator, CFDictionaryRef options, CFErrorRef *error)
{
    (void) allocator;
    (void) options;

    struct __ODSession *session = calloc(1, sizeof(*session));
    if (session == NULL) {
        setODError(error, "ODSessionCreate: out of memory");
        return NULL;
    }

    if (error != NULL)
        *error = NULL;
    return session;
}

#pragma mark - Node

ODNodeRef ODNodeCreateWithNodeType(CFAllocatorRef allocator, ODSessionRef session,
                                   ODNodeType nodeType, CFErrorRef *error)
{
    (void) allocator;
    (void) session;

    // pam_opendirectory asks for kODNodeTypeAuthentication (0x2201) and passes a
    // NULL session; both are legitimate. kODNodeTypeLocalNodes (0x2200) names the
    // same local directory. Any other type names a service this node does not
    // have, so refuse rather than answer from passwd.
    if (nodeType != kODNodeTypeAuthentication && nodeType != kODNodeTypeLocalNodes) {
        setODError(error, "ODNodeCreateWithNodeType: unsupported node type");
        return NULL;
    }

    struct __ODNode *node = calloc(1, sizeof(*node));
    if (node == NULL) {
        setODError(error, "ODNodeCreateWithNodeType: out of memory");
        return NULL;
    }

    node->name = copyCFString("local");
    if (node->name == NULL) {
        free(node);
        setODError(error, "ODNodeCreateWithNodeType: out of memory");
        return NULL;
    }

    if (error != NULL)
        *error = NULL;
    return node;
}

#pragma mark - Record

ODRecordRef ODNodeCopyRecord(ODNodeRef node, ODRecordType recordType, CFStringRef recordName,
                             CFTypeRef attributes, CFErrorRef *error)
{
    (void) attributes;

    if (nodeOf(node) == NULL) {
        setODError(error, "ODNodeCopyRecord: invalid node");
        return NULL;
    }

    // Only user records are backed by the passwd database, so refusing other
    // types here keeps the node from claiming to hold data it cannot serve.
    // Compare by value, not by pointer: pam_opendirectory brings its own
    // CFString for "dsRecTypeStandard:Users", so it is a different object from
    // kODRecordTypeUsers even though the contents match.
    // kODRecordTypeUsers is a `const ODRecordType`, and ODRecordType is
    // CFStringRef, so the comparison is against the string it holds rather than
    // the address of the variable. pam_opendirectory brings its own equal
    // string, so identity would never match.
    CFStringRef usersType = kODRecordTypeUsers;
    if (recordType == NULL || usersType == NULL ||
        CFGetTypeID((CFTypeRef) recordType) != CFStringGetTypeID() ||
        CFStringCompare((CFStringRef) recordType, usersType, 0) !=
            kCFCompareEqualTo) {
        setODError(error, "ODNodeCopyRecord: unsupported record type");
        return NULL;
    }

    char *name = cStringOf(recordName);
    if (name == NULL) {
        setODError(error, "ODNodeCopyRecord: invalid record name");
        return NULL;
    }

    ODRecordRef record = createLocalRecord(name);
    free(name);

    if (record == NULL) {
        setODError(error, "ODNodeCopyRecord: no such record");
        return NULL;
    }

    if (error != NULL)
        *error = NULL;
    return record;
}

#pragma mark - Authentication

bool ODRecordAuthenticationAllowed(ODRecordRef record, CFErrorRef *error)
{
    if (error != NULL)
        *error = NULL;
    // The passwd database is readable by the guest's own users, so a record read
    // from it is a record the caller was allowed to read.
    return recordOf(record) != NULL;
}

bool ODRecordVerifyPassword(ODRecordRef record, CFStringRef password, CFErrorRef *error)
{
    struct __ODRecord *self = recordOf(record);

    if (error != NULL)
        *error = NULL;

    if (self == NULL || self->authAuthority == NULL || password == NULL) {
        setODError(error, "ODRecordVerifyPassword: no record or password");
        return false;
    }

    char *stored = cStringOf(self->authAuthority);
    char *given = cStringOf(password);

    if (stored == NULL || given == NULL) {
        free(stored);
        free(given);
        setODError(error, "ODRecordVerifyPassword: out of memory");
        return false;
    }

    // crypt(3) takes the salt from the beginning of the stored hash, so the
    // scheme and iterations travel with the comparison and no part of the
    // configuration is assumed here.
    char *computed = crypt(given, stored);

    bool matches = false;
    if (computed != NULL && stored[0] != '\0')
        matches = strcmp(computed, stored) == 0;

    free(stored);
    free(given);

    return matches;
}

#pragma mark - Node queries

CFArrayRef ODNodeCopyUnreachableSubnodeNames(ODNodeRef node, CFErrorRef *error)
{
    if (nodeOf(node) == NULL) {
        setODError(error, "ODNodeCopyUnreachableSubnodeNames: invalid node");
        return NULL;
    }

    if (error != NULL)
        *error = NULL;

    // A passwd-backed node has no subnodes. Returning an empty array rather than
    // NULL matters: the caller walks the result without checking, so NULL is a
    // crash, while "none" is the truthful answer.
    return CFArrayCreate(kCFAllocatorDefault, NULL, 0, &kCFTypeArrayCallBacks);
}
