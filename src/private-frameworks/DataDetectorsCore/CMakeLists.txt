project(DataDetectorsCore)

remove_sdk_framework(DataDetectorsCore
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "590.3.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DataDetectorsCore
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DataDetectorsCore"
	PRIVATE
)

add_framework(DataDetectorsCore
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/DataDetectorsCore.m
        src/DDBindableRegexp.m
        src/DDBasicRegexp.m
        src/DDVariable.m
        src/DDOperatorRegexp.m
        src/DDDisjunctionRegexp.m
        src/DDConcatenationRegexp.m
        src/DDUnaryOperator.m
        src/DDOneOrMoreExp.m
        src/DDRepeatCount.m
        src/DDRepeatMax.m
        src/DDOptionalExp.m
        src/DDStarExp.m
        src/DDRepeatMinMax.m
        src/DDAtomicRegexp.m
        src/DDTokenRegexp.m
        src/DDLookupRegexp.m
        src/DDErrorRegexp.m
        src/DDVariableNotFoundError.m
        src/DDEmptyPatternError.m
        src/DDInvalidRangeError.m
        src/DDDictionaryError.m
        src/DDBinderInfo.m
        src/DDScannerObject.m
        src/DDScannerList.m
        src/DDScanServerDispatcher.m
        src/DDScanServer.m
        src/DDRegexpManager.m
        src/DDScannerResult.m
        src/DDGrammar.m
        src/DDSymbol.m
        src/DDNonTerminal.m
        src/DDProduction.m
        src/DDLocation.m
        src/DDCompilationNote.m
        src/DDParserState.m
        src/DDStatsBuilder.m
        src/DDCompilerState.m
        src/DDLRItem.m
        src/DDAbstractType.m
        src/DDTypeInhabitant.m
        src/DDTypeChecker.m
        src/DDMessageCacheElement.m
        src/DDMessageCache.m
        src/DataDetectorsSourceAccess.m

    DEPENDENCIES
        system
        objc
        Foundation
)
