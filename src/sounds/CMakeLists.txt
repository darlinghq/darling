# The system sounds of the prefix, /System/Library/Sounds/<Name>.aiff, which
# +[NSSound soundNamed:] finds by name.  The files are listed in SOURCES.txt.
file(GLOB system_sounds ${CMAKE_CURRENT_SOURCE_DIR}/*.aiff)
install(FILES ${system_sounds}
	DESTINATION libexec/darling/System/Library/Sounds)
install(FILES
	SOURCES.txt
	LICENSE-Kenney-Interface-Sounds.txt
	DESTINATION libexec/darling/usr/share/licenses/darling-system-sounds)
