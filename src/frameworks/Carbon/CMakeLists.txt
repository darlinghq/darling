project(Carbon)

add_subdirectory(CommonPanels)
add_subdirectory(Help)
add_subdirectory(HIToolbox)
add_subdirectory(ImageCapture)
add_subdirectory(Ink)
add_subdirectory(OpenScripting)
add_subdirectory(Print)
add_subdirectory(SecurityHI)
add_subdirectory(SpeechRecognition)

set(DYLIB_COMPAT_VERSION "2.0.0")
set(DYLIB_CURRENT_VERSION "158.0.0")

add_framework(Carbon
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/Carbon.c

    DEPENDENCIES
        CommonPanels
        Help
        HIToolbox
        ImageCapture
        Ink
        OpenScripting
        Print
        SecurityHI
        SpeechRecognition
    	CoreFoundation
        Foundation
        system
)

set(CommonPanels_BUILD ${CMAKE_CURRENT_BINARY_DIR}/CommonPanels/CommonPanels)
set(Help_BUILD ${CMAKE_CURRENT_BINARY_DIR}/Help/Help)
set(HIToolbox_BUILD ${CMAKE_CURRENT_BINARY_DIR}/HIToolbox/HIToolbox)
set(ImageCapture_BUILD ${CMAKE_CURRENT_BINARY_DIR}/ImageCapture/ImageCapture)
set(Ink_BUILD ${CMAKE_CURRENT_BINARY_DIR}/Ink/Ink)
set(OpenScripting_BUILD ${CMAKE_CURRENT_BINARY_DIR}/OpenScripting/OpenScripting)
set(Print_BUILD ${CMAKE_CURRENT_BINARY_DIR}/Print/Print)
set(SecurityHI_BUILD ${CMAKE_CURRENT_BINARY_DIR}/SecurityHI/SecurityHI)
set(SpeechRecognition_BUILD ${CMAKE_CURRENT_BINARY_DIR}/SpeechRecognition/SpeechRecognition)

reexport(Carbon CommonPanels ${CommonPanels_BUILD})
reexport(Carbon Help ${Help_BUILD})
reexport(Carbon HIToolbox ${HIToolbox_BUILD})
reexport(Carbon ImageCapture ${ImageCapture_BUILD})
reexport(Carbon Ink ${Ink_BUILD})
reexport(Carbon OpenScripting ${OpenScripting_BUILD})
reexport(Carbon Print ${Print_BUILD})
reexport(Carbon SecurityHI ${SecurityHI_BUILD})
reexport(Carbon SpeechRecognition ${SpeechRecognition_BUILD})

set(ApplicationServices_BUILD ${CMAKE_CURRENT_BINARY_DIR}/../ApplicationServices/ApplicationServices)

reexport(Carbon ApplicationServices ${ApplicationServices_BUILD})
