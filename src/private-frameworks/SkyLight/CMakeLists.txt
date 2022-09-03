project(SkyLight)

remove_sdk_framework(SkyLight
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "64.0.0")
set(DYLIB_CURRENT_VERSION "600.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SkyLight
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SkyLight"
	PRIVATE
)

add_framework(SkyLight
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/SkyLight.m
		src/CGXConnectionBox.m
		src/SLDataTimelineConfig.m
		src/SLDataTimelineConnection.m
		src/SLDataTimelineUpdate.m
		src/SLDataTimelineSnapshotCollection.m
		src/SLDataTimelineServerSnapshotEntry.m
		src/SLDataTimelineSessionEntry.m
		src/SLDataTimelineSessionPlaceholderEntry.m
		src/SLDataTimelineSessionSnapshotEntry.m
		src/SLDataTimelineProcessEntry.m
		src/SLSecureCursorAssertion.m
		src/SLSecureCursorAssertionManager.m
		src/SLSessionOwner.m

	DEPENDENCIES
		system
		objc
		Foundation
)
