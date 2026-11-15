// Copyright (C) 2026 Lubos Dolezel
//
// The legacy DirectoryService API (dsOpenDirService, dsFindDirNodes, dsOpenDirNode,
// dsDoAttributeValueSearchWithData, dsGetRecordList, ...) answers from the system's
// directory: the search node is found and opened, and the root user's record is
// returned by a UniqueID search and by a name search. These are the calls a
// membership daemon makes at start-up. Also covers the authentication buffer helpers
// that the framework exports and the error statuses for bad references.
//
// Expected results come from the public DirectoryService headers; the same program
// passes on macOS 26.7.3 and 10.14.4.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <DirectoryService/DirectoryService.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Exported by the framework but not declared in the SDK headers
extern tDirStatus dsAuthBufferGetDataListPriv(tDataBufferPtr inAuthBuff, tDataListPtr inOutDataList);
extern tDataListPtr dsAuthBufferGetDataListAllocPriv(tDataBufferPtr inAuthBuff);
extern char *dsDataListGetNodeStringPriv(tDataListPtr inDataList, UInt32 inNodeIndex);

static int failures;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

// Runs an attribute search for one user record and reports the values of the record's
// RecordName and UniqueID attributes as the search returned them.
static int fetch_user(tDirReference dir, tDirNodeReference node, int by_name, const char *value,
	char *name_out, size_t name_size, char *uid_out, size_t uid_size, tDirStatus *status_out)
{
	UInt32 buffer_size = 4096;
	tDataBufferPtr buffer = dsDataBufferAllocate(dir, buffer_size);
	tDataListPtr types = dsBuildListFromStrings(dir, kDSStdRecordTypeUsers, NULL);
	tDataListPtr attrs = dsBuildListFromStrings(dir, kDS1AttrUniqueID, kDSNAttrRecordName,
		kDS1AttrGeneratedUID, NULL);
	tDataNodePtr attr_type = dsDataNodeAllocateString(dir, kDS1AttrUniqueID);
	tDataNodePtr pattern = dsDataNodeAllocateString(dir, value);
	tDataListPtr names = dsBuildListFromStrings(dir, value, NULL);
	tContextData context = 0;
	UInt32 count = 0;
	tDirStatus status;
	int found = 0;

	name_out[0] = uid_out[0] = '\0';

	do {
		count = 0;
		if (by_name)
			status = dsGetRecordList(node, buffer, names, eDSExact, types, attrs, false, &count, &context);
		else
			status = dsDoAttributeValueSearchWithData(node, buffer, types, attr_type, eDSExact,
				pattern, attrs, 0, &count, &context);
		if (status == eDSBufferTooSmall) {
			dsDataBufferDeAllocate(dir, buffer);
			buffer_size *= 2;
			buffer = dsDataBufferAllocate(dir, buffer_size);
			context = 0;
		}
	} while (status == eDSBufferTooSmall && buffer_size <= 1024 * 1024);

	*status_out = status;

	if (status == eDSNoErr && count > 0) {
		tAttributeListRef attr_list = 0;
		tRecordEntryPtr record = NULL;

		if (dsGetRecordEntry(node, buffer, 1, &attr_list, &record) == eDSNoErr && record != NULL) {
			for (UInt32 i = 1; i <= record->fRecordAttributeCount; i++) {
				tAttributeValueListRef value_list = 0;
				tAttributeEntryPtr info = NULL;
				tAttributeValueEntryPtr first = NULL;

				if (dsGetAttributeEntry(node, buffer, attr_list, i, &value_list, &info) != eDSNoErr)
					continue;
				if (dsGetAttributeValue(node, buffer, 1, value_list, &first) == eDSNoErr && first != NULL) {
					const char *attr = info->fAttributeSignature.fBufferData;
					const char *data = first->fAttributeValueData.fBufferData;

					if (strcmp(attr, kDSNAttrRecordName) == 0)
						snprintf(name_out, name_size, "%s", data);
					else if (strcmp(attr, kDS1AttrUniqueID) == 0)
						snprintf(uid_out, uid_size, "%s", data);
					dsDeallocAttributeValueEntry(dir, first);
				}
				dsCloseAttributeValueList(value_list);
				dsDeallocAttributeEntry(dir, info);
			}
			found = 1;
			dsCloseAttributeList(attr_list);
			dsDeallocRecordEntry(dir, record);
		}
	}

	if (context != 0)
		dsReleaseContinueData(dir, context);
	dsDataBufferDeAllocate(dir, buffer);
	dsDataListDeallocate(dir, types);
	free(types);
	dsDataListDeallocate(dir, attrs);
	free(attrs);
	dsDataListDeallocate(dir, names);
	free(names);
	dsDataNodeDeAllocate(dir, attr_type);
	dsDataNodeDeAllocate(dir, pattern);
	return found;
}

int main(void)
{
	tDirReference dir = 0;
	tDirStatus status = dsOpenDirService(&dir);

	check("dsOpenDirService succeeds", status == eDSNoErr && dir != 0);
	if (dir == 0)
		return 1;

	// The search node is found with the default node pattern
	tDataBufferPtr node_buffer = dsDataBufferAllocate(dir, 4096);
	UInt32 node_count = 0;
	tContextData context = 0;

	status = dsFindDirNodes(dir, node_buffer, NULL, eDSSearchNodeName, &node_count, &context);
	check("dsFindDirNodes(search node) succeeds", status == eDSNoErr);
	check("dsFindDirNodes(search node) finds a node", node_count >= 1);

	tDataListPtr node_name = NULL;
	status = dsGetDirNodeName(dir, node_buffer, 1, &node_name);
	check("dsGetDirNodeName(1) succeeds", status == eDSNoErr && node_name != NULL);

	char *path = node_name != NULL ? dsGetPathFromList(dir, node_name, "/") : NULL;
	check("the search node's path is /Search", path != NULL && strcmp(path, "/Search") == 0);
	free(path);

	tDataListPtr none = NULL;
	status = dsGetDirNodeName(dir, node_buffer, node_count + 1, &none);
	check("dsGetDirNodeName past the last node fails", status != eDSNoErr);

	tDirNodeReference node = 0;
	status = node_name != NULL ? dsOpenDirNode(dir, node_name, &node) : eDSNullNodeName;
	check("dsOpenDirNode(search node) succeeds", status == eDSNoErr && node != 0);

	if (node_name != NULL) {
		dsDataListDeallocate(dir, node_name);
		free(node_name);
	}
	dsDataBufferDeAllocate(dir, node_buffer);

	// The root user's record, by UniqueID and by name
	char name[128], uid[32];

	if (node != 0) {
		int found = fetch_user(dir, node, 0, "0", name, sizeof(name), uid, sizeof(uid), &status);
		check("UniqueID search for 0 returns without error", status == eDSNoErr);
		check("UniqueID search for 0 finds a record", found);
		check("the record's name is root", strcmp(name, "root") == 0);
		check("the record's UniqueID is 0", strcmp(uid, "0") == 0);

		found = fetch_user(dir, node, 1, "root", name, sizeof(name), uid, sizeof(uid), &status);
		check("name search for root returns without error", status == eDSNoErr);
		check("name search for root finds a record", found);
		check("the found record's UniqueID is 0", strcmp(uid, "0") == 0);

		found = fetch_user(dir, node, 1, "no-such-user-directoryservice-test", name, sizeof(name),
			uid, sizeof(uid), &status);
		check("a name that does not exist returns no record", status == eDSNoErr && !found);

		check("dsCloseDirNode succeeds", dsCloseDirNode(node) == eDSNoErr);
	}

	// Bad references are rejected
	node_buffer = dsDataBufferAllocate(dir, 4096);
	status = dsFindDirNodes(0x7ffffff0, node_buffer, NULL, eDSSearchNodeName, &node_count, &context);
	check("dsFindDirNodes on a bad directory reference fails", status != eDSNoErr);
	tDirNodeReference bad_node = 0;
	tDataListPtr bad_name = dsBuildListFromStrings(dir, "Search", NULL);
	status = dsOpenDirNode(0x7ffffff0, bad_name, &bad_node);
	check("dsOpenDirNode on a bad directory reference fails", status != eDSNoErr);
	dsDataListDeallocate(dir, bad_name);
	free(bad_name);
	dsDataBufferDeAllocate(dir, node_buffer);

	// Authentication buffers: length-prefixed items turn into a data list
	tDataBufferPtr auth = dsDataBufferAllocate(dir, 256);
	status = dsAppendAuthBuffer(auth, 2, 4, "user", 8, "password");
	check("dsAppendAuthBuffer appends two items", status == eDSNoErr);

	tDataListPtr items = dsAuthBufferGetDataListAllocPriv(auth);
	check("auth buffer converts to a data list", items != NULL && items->fDataNodeCount == 2);
	if (items != NULL) {
		char *first = dsDataListGetNodeStringPriv(items, 1);
		char *second = dsDataListGetNodeStringPriv(items, 2);

		check("first item is the user name", first != NULL && strcmp(first, "user") == 0);
		check("second item is the password", second != NULL && strcmp(second, "password") == 0);
		check("item index 0 is rejected", dsDataListGetNodeStringPriv(items, 0) == NULL);
		check("item index past the end is rejected", dsDataListGetNodeStringPriv(items, 3) == NULL);
		free(first);
		free(second);
		dsDataListDeallocate(dir, items);
		free(items);
	}

	auth->fBufferLength = 6; // a length of 4 followed by only two bytes
	memcpy(auth->fBufferData, "\x04\x00\x00\x00" "ab", 6);
	check("a truncated auth buffer is rejected", dsAuthBufferGetDataListAllocPriv(auth) == NULL);
	dsDataBufferDeAllocate(dir, auth);

	check("dsCloseDirService succeeds", dsCloseDirService(dir) == eDSNoErr);
	printf("%s\n", failures == 0 ? "PASS" : "FAIL");
	return failures != 0;
}
