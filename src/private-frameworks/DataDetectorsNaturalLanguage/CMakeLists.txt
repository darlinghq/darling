project(DataDetectorsNaturalLanguage)

remove_sdk_framework(DataDetectorsNaturalLanguage
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DataDetectorsNaturalLanguage
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DataDetectorsNaturalLanguage"
	PRIVATE
)

add_framework(DataDetectorsNaturalLanguage
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/DataDetectorsNaturalLanguage.m
        src/IPFeatureScanner.m
        src/IPFeatureMailScanner.m
        src/IPFeatureTextMessageScanner.m
        src/IPDataDetectorsFeatureExtractor.m
        src/IPFeatureExtractor.m
        src/IPFeatureManager.m
        src/IPTenseDetector.m
        src/IPFeatureData.m
        src/IPKeywordFeatureExtractor.m
        src/IPCircularBufferArray.m
        src/IPRegexToolbox.m
        src/IPPerson.m
        src/IPTextMessageConversation.m
        src/IPSentenceFeatureExtractor.m
        src/IPQuoteParser.m
        src/IPFeature.m
        src/IPMessageXMLParser.m
        src/IPMessageUnit.m
        src/IPMessage.m
        src/IPFeatureSentenceFragment.m
        src/IPFeatureSentence.m
        src/IPFeatureKeyword.m
        src/IPEventClassificationType.m

    DEPENDENCIES
        system
        objc
        Foundation
)
