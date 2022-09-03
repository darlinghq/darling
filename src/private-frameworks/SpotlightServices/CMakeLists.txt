project(SpotlightServices)

remove_sdk_framework(SpotlightServices
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1191.2.6")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SpotlightServices
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SpotlightServices"
	PRIVATE
)

add_framework(SpotlightServices
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/SpotlightServices.m
        src/PRSRankingUtilities.m
        src/PRSModel.m
        src/SSADEventReporter.m
        src/PRSRankingItemRelativeFeatureContext.m
        src/PRSFeedback.m
        src/PRSSearchFeedback.m
        src/PRSRenderFeedback.m
        src/PRSFeedbackContext.m
        src/PRSAbandonmentFeedback.m
        src/PRSMLFeedback.m
        src/PRSPreviewFeedback.m
        src/PRSEngagementFeedback.m
        src/PRSMapsEngagementFeedback.m
        src/PRSiTunesEngagementFeedback.m
        src/PRSNBRankingFeedback.m
        src/PRSLogisticWithNaiveBayesEngine.m
        src/PRSQueryRankingConfiguration.m
        src/PRSModelResourceLoader.m
        src/PRSRankingItem.m
        src/PRSL3FeatureVector.m
        src/PRSRankingConfiguration.m
        src/PRSRankingSpanCalculator.m
        src/SSCoreMLInterface.m
        src/SSParsecBundleFeatureCollection.m
        src/PRSFTETask.m
        src/PRSBagHandler.m
        src/PRSRankingItemRanker.m
        src/PRSNaiveBayesCoreEngine.m
        src/PRSQueryTask.m
        src/PRSDictationQueryTask.m
        src/PRSCoreParsecQueryTask.m
        src/PRSMovie.m
        src/PRSAlbum.m
        src/PRSBuyButton.m
        src/PRSAppLinkSection.m
        src/PRSColumnSection.m
        src/PRSTrailerEntry.m
        src/PRSTitleCardSection.m
        src/PRSDescriptionCardSection.m
        src/PRSRowCardSection.m
        src/PRSTVRowCardSection.m
        src/PRSAuxiliaryInfo.m
        src/PRSRichTitleCardSection.m
        src/PRSSportsSummaryScoreCardSection.m
        src/PRSSportsDataCardWithImageCardSection.m
        src/PRSTwitterCardSection.m
        src/PRSAppLinkCardSection.m
        src/PRSTextColumnsCardSection.m
        src/PRSTrailersCardSection.m
        src/PRSLinkEntry.m
        src/PRSLinkTableCardSection.m
        src/PRSTrackListCardSection.m
        src/PRSFUSETrackListCardSection.m
        src/PRSFUSEAlbumListCardSection.m
        src/PRSMovieListCardSection.m
        src/PRSWebSiteCardSection.m
        src/PRSStockChartCardSection.m
        src/PRSHTMLCardSection.m
        src/PRSBaseCardSection.m
        src/PRSActionButton.m
        src/PRSCardSection.m
        src/PRSBaseEntry.m
        src/PRSTableRowCardSection.m
        src/PRSTableAlignmentSchema.m
        src/PRSFormattedTextItem.m
        src/PRSCoreDuet.m
        src/PRSFeedbackProxy.m
        src/PRSSearchSession.m
        src/PRSDirectivesManager.m
        src/SPLogisticWithNaiveBayesEngine.m
        src/PRSMLManager.m
        src/PRSRankingServerKnobs.m
        src/PRSRankingItemCollection.m
        src/PRSAnonymousPipelineManager.m
        src/SSPlistDataReader.m
        src/PRSAnonymousPipelineManagerSession.m
        src/PRSRankingManager.m
        src/SSEngagementHandler.m
        src/PRSL2FeatureVectorProcessingContext.m
        src/PRSL2FeatureVector.m
        src/PRSRankingCosineComponents.m
        src/PRSRankingPolicyManager.m
        src/PRSModelFeedbackContext.m
        src/PRSModelContext.m
        src/PRSModelManager.m

    DEPENDENCIES
        system
        objc
        Foundation
)
