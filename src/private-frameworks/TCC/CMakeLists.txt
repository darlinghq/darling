project(TCC)

remove_sdk_framework(TCC
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(TCC
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/TCC"
)

add_framework(TCC
	PRIVATE
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/OS_tcc_authorization_cache.m
		src/OS_tcc_service.m
		src/OS_tcc_identity.m
		src/OS_tcc_server.m
		src/OS_tcc_events_filter.m
		src/OS_tcc_message_options.m
		src/OS_tcc_authorization_change_monitor.m
		src/OS_tcc_credential.m
		src/OS_tcc_authorization_record.m
		src/OS_tcc_events_subscription.m
		src/OS_tcc_attributed_entity.m
		src/OS_tcc_object.m
		src/TCC.m

	DEPENDENCIES
		system
		objc
		Foundation
)
