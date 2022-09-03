project(DataDetectors)

remove_sdk_framework(DataDetectors
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "376.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DataDetectors
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DataDetectors"
	PRIVATE
)

add_framework(DataDetectors
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/DataDetectors.m
        src/DDMenuItem.m
        src/DDRemoteBubbleActionController.m
        src/DDAction.m
        src/DDURLAction.m
        src/DDBubbleAction.m
        src/DDMiscAction.m
        src/DDSeparatorAction.m
        src/DDDialController.m
        src/DDMessageController.m
        src/DDGenericTTYDialController.m
        src/DDTTYDirectDialController.m
        src/DDTTYRelayDialController.m
        src/DDFacetimeController.m
        src/DDFacetimeAudioController.m
        src/DDActionsManager.m
        src/DDHighlightObject.m
        src/DDHighlightRequest.m
        src/DDDataDetectorsViewHost.m
        src/DDSignaturePreviewController.m
        src/DDSignatureNewContactEditorController.m
        src/DDSignatureExistingContactEditorController.m
        src/DDSignatureMapController.m
        src/DDButtonBar.m
        src/DDScanRequest.m
        src/DDOperation.m
        src/DDHTMLMessageScanningOperation.m
        src/DDParsecController.m
        src/DDThreadManager.m
        src/DDPhoneOrMailURI.m
        src/DDBasicHighlightsView.m
        src/DDPatternCompiler.m
        src/DDFlightController.m
        src/DDPreviewMenuItem.m
        src/DDMapTable.m
        src/DDActionViewController.m
        src/DDHighlightRangeUpdater.m
        src/DDRange.m
        src/DDHighlightButton.m
        src/DDHighlightButtonCell.m
        src/DDLargeTypeController.m
        src/DDActionContext.m
        src/DDEventController.m
        src/DDPopover.m
        src/DDDetectorManager2.m
        src/DDPhoneNumber.m
        src/DDHighlightRangeUpdater2.m
        src/DDMapLinkController.m
        src/DDShowDateController.m
        src/DDGenericBubbleController.m
        src/DDRange2.m
        src/DDWK2Overlay.m

    DEPENDENCIES
        system
        objc
        Foundation
)
