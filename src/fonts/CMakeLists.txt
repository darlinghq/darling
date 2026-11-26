# Copyright (C) 2026 Lubos Dolezel
#
# The "Helvetica Neue" family of the prefix, /System/Library/Fonts/HelveticaNeue.ttc.
# Darling cannot ship Apple's fonts. Applications look the family up through
# CoreText and then open the returned file themselves, expecting the faces in
# it to carry the family name, so the prefix provides a font of that name,
# made from URW++ Nimbus Sans (AGPLv3 with a font exception; see nimbus-sans/
# and HelveticaNeue-NOTICE.txt). The renaming runs at build time with the
# fontTools Python package; without it the font is not built and the prefix
# has no Helvetica Neue.

find_package(Python3 QUIET COMPONENTS Interpreter)
set(HELVETICA_NEUE_CAN_BUILD FALSE)
if (Python3_Interpreter_FOUND)
	execute_process(COMMAND ${Python3_EXECUTABLE} -c "import fontTools.ttLib"
		RESULT_VARIABLE fonttools_result OUTPUT_QUIET ERROR_QUIET)
	if (fonttools_result EQUAL 0)
		set(HELVETICA_NEUE_CAN_BUILD TRUE)
	endif()
endif()

if (HELVETICA_NEUE_CAN_BUILD)
	set(sources_dir ${CMAKE_CURRENT_SOURCE_DIR}/nimbus-sans)
	set(sources
		${sources_dir}/NimbusSans-Regular.ttf
		${sources_dir}/NimbusSans-Bold.ttf
		${sources_dir}/NimbusSans-Italic.ttf
		${sources_dir}/NimbusSans-BoldItalic.ttf)
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/HelveticaNeue.ttc
		COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/make-helvetica-neue.py
			${CMAKE_CURRENT_BINARY_DIR}/HelveticaNeue.ttc ${sources}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/make-helvetica-neue.py ${sources}
		COMMENT "Building HelveticaNeue.ttc from Nimbus Sans"
		VERBATIM)
	add_custom_target(HelveticaNeue ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/HelveticaNeue.ttc)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/HelveticaNeue.ttc
		DESTINATION libexec/darling/System/Library/Fonts)
	install(FILES
		HelveticaNeue-NOTICE.txt
		nimbus-sans/COPYING
		nimbus-sans/LICENSE
		DESTINATION libexec/darling/usr/share/licenses/darling-helvetica-neue)
else()
	message(STATUS "Python 3 with fontTools not found: HelveticaNeue.ttc will not be built")
endif()
