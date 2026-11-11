// Copyright (C) 2026 Lubos Dolezel
//
// Automator workflow type identifiers, document types, data type identifiers
// and variable identifiers, and _AMSkipOpenUntitledFile.
// Link with -framework Automator. The program passes unchanged on macOS 27.0.1
// (arm64 and x86_64).
#import <Foundation/Foundation.h>
#include <stdio.h>

extern NSString* const AMWorkflowTypeIdentifierWorkflow;
extern NSString* const AMWorkflowTypeIdentifierApplication;
extern NSString* const AMWorkflowTypeIdentifierServicesMenu;
extern NSString* const AMDocumentTypeWorkflow;
extern NSString* const AMDocumentTypeApplication;
extern NSString* const AMDocumentExtensionWorkflow;
extern NSString* const AMDocumentExtensionApp;
extern NSString* const AMWorkflowContentsDirectory;
extern NSString* const AMTypeIdentifierFilesAndFolders;
extern NSString* const AMTypeIdentifierText;
extern NSString* const AMTypeUTICocoaPath;
extern NSString* const AMVariableIdentifierPath;
extern NSString* const AMVariableIdentifierStorage;
extern NSString* const AMVariableIdentifierText;
extern BOOL _AMSkipOpenUntitledFile(void);

static int g_failures = 0;

#define CHECK_STRING(constant, expected) do { \
	if (![(constant) isEqualToString:(expected)]) { \
		printf("FAIL %d: %s is %s\n", __LINE__, #constant, [(constant) UTF8String]); \
		g_failures++; \
	} \
} while (0)

int main(void)
{
	@autoreleasepool {
		CHECK_STRING(AMWorkflowTypeIdentifierWorkflow, @"com.apple.Automator.workflow");
		CHECK_STRING(AMWorkflowTypeIdentifierApplication, @"com.apple.Automator.application");
		CHECK_STRING(AMWorkflowTypeIdentifierServicesMenu, @"com.apple.Automator.servicesMenu");
		CHECK_STRING(AMDocumentTypeWorkflow, @"Workflow");
		CHECK_STRING(AMDocumentTypeApplication, @"Application");
		CHECK_STRING(AMDocumentExtensionWorkflow, @"workflow");
		CHECK_STRING(AMDocumentExtensionApp, @"app");
		CHECK_STRING(AMWorkflowContentsDirectory, @"Contents");
		CHECK_STRING(AMTypeIdentifierFilesAndFolders, @"com.apple.Automator.fileSystemObject");
		CHECK_STRING(AMTypeIdentifierText, @"com.apple.Automator.text");
		CHECK_STRING(AMTypeUTICocoaPath, @"com.apple.cocoa.path");
		CHECK_STRING(AMVariableIdentifierPath, @"com.apple.Automator.Variable.Path");
		CHECK_STRING(AMVariableIdentifierStorage, @"com.apple.Automator.Variable.Storage");
		CHECK_STRING(AMVariableIdentifierText, @"com.apple.Automator.Variable.Text");
		if (_AMSkipOpenUntitledFile()) {
			printf("FAIL: _AMSkipOpenUntitledFile returned YES\n");
			g_failures++;
		}
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
