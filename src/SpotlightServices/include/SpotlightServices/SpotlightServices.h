/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _SpotlightServices_H_
#define _SpotlightServices_H_

#import <Foundation/Foundation.h>

#import <SpotlightServices/PARResultFactory.h>
#import <SpotlightServices/PARSessionDelegate.h>
#import <SpotlightServices/PRSFeatureVector.h>
#import <SpotlightServices/PRSResourceProvider.h>
#import <SpotlightServices/PRSTriggerTask.h>
#import <SpotlightServices/SFFeedbackListener.h>
#import <SpotlightServices/SSDataCollectible.h>
#import <SpotlightServices/PRSRankingUtilities.h>
#import <SpotlightServices/PRSModel.h>
#import <SpotlightServices/SSADEventReporter.h>
#import <SpotlightServices/PRSRankingItemRelativeFeatureContext.h>
#import <SpotlightServices/PRSFeedback.h>
#import <SpotlightServices/PRSSearchFeedback.h>
#import <SpotlightServices/PRSRenderFeedback.h>
#import <SpotlightServices/PRSFeedbackContext.h>
#import <SpotlightServices/PRSAbandonmentFeedback.h>
#import <SpotlightServices/PRSMLFeedback.h>
#import <SpotlightServices/PRSPreviewFeedback.h>
#import <SpotlightServices/PRSEngagementFeedback.h>
#import <SpotlightServices/PRSMapsEngagementFeedback.h>
#import <SpotlightServices/PRSiTunesEngagementFeedback.h>
#import <SpotlightServices/PRSNBRankingFeedback.h>
#import <SpotlightServices/PRSLogisticWithNaiveBayesEngine.h>
#import <SpotlightServices/PRSQueryRankingConfiguration.h>
#import <SpotlightServices/PRSModelResourceLoader.h>
#import <SpotlightServices/PRSRankingItem.h>
#import <SpotlightServices/PRSL3FeatureVector.h>
#import <SpotlightServices/PRSRankingConfiguration.h>
#import <SpotlightServices/PRSRankingSpanCalculator.h>
#import <SpotlightServices/SSCoreMLInterface.h>
#import <SpotlightServices/SSParsecBundleFeatureCollection.h>
#import <SpotlightServices/PRSFTETask.h>
#import <SpotlightServices/PRSBagHandler.h>
#import <SpotlightServices/PRSRankingItemRanker.h>
#import <SpotlightServices/PRSNaiveBayesCoreEngine.h>
#import <SpotlightServices/PRSQueryTask.h>
#import <SpotlightServices/PRSDictationQueryTask.h>
#import <SpotlightServices/PRSCoreParsecQueryTask.h>
#import <SpotlightServices/PRSMovie.h>
#import <SpotlightServices/PRSAlbum.h>
#import <SpotlightServices/PRSBuyButton.h>
#import <SpotlightServices/PRSAppLinkSection.h>
#import <SpotlightServices/PRSColumnSection.h>
#import <SpotlightServices/PRSTrailerEntry.h>
#import <SpotlightServices/PRSTitleCardSection.h>
#import <SpotlightServices/PRSDescriptionCardSection.h>
#import <SpotlightServices/PRSRowCardSection.h>
#import <SpotlightServices/PRSTVRowCardSection.h>
#import <SpotlightServices/PRSAuxiliaryInfo.h>
#import <SpotlightServices/PRSRichTitleCardSection.h>
#import <SpotlightServices/PRSSportsSummaryScoreCardSection.h>
#import <SpotlightServices/PRSSportsDataCardWithImageCardSection.h>
#import <SpotlightServices/PRSTwitterCardSection.h>
#import <SpotlightServices/PRSAppLinkCardSection.h>
#import <SpotlightServices/PRSTextColumnsCardSection.h>
#import <SpotlightServices/PRSTrailersCardSection.h>
#import <SpotlightServices/PRSLinkEntry.h>
#import <SpotlightServices/PRSLinkTableCardSection.h>
#import <SpotlightServices/PRSTrackListCardSection.h>
#import <SpotlightServices/PRSFUSETrackListCardSection.h>
#import <SpotlightServices/PRSFUSEAlbumListCardSection.h>
#import <SpotlightServices/PRSMovieListCardSection.h>
#import <SpotlightServices/PRSWebSiteCardSection.h>
#import <SpotlightServices/PRSStockChartCardSection.h>
#import <SpotlightServices/PRSHTMLCardSection.h>
#import <SpotlightServices/PRSBaseCardSection.h>
#import <SpotlightServices/PRSActionButton.h>
#import <SpotlightServices/PRSCardSection.h>
#import <SpotlightServices/PRSBaseEntry.h>
#import <SpotlightServices/PRSTableRowCardSection.h>
#import <SpotlightServices/PRSTableAlignmentSchema.h>
#import <SpotlightServices/PRSFormattedTextItem.h>
#import <SpotlightServices/PRSCoreDuet.h>
#import <SpotlightServices/PRSFeedbackProxy.h>
#import <SpotlightServices/PRSSearchSession.h>
#import <SpotlightServices/PRSDirectivesManager.h>
#import <SpotlightServices/SPLogisticWithNaiveBayesEngine.h>
#import <SpotlightServices/PRSMLManager.h>
#import <SpotlightServices/PRSRankingServerKnobs.h>
#import <SpotlightServices/PRSRankingItemCollection.h>
#import <SpotlightServices/PRSAnonymousPipelineManager.h>
#import <SpotlightServices/SSPlistDataReader.h>
#import <SpotlightServices/PRSAnonymousPipelineManagerSession.h>
#import <SpotlightServices/PRSRankingManager.h>
#import <SpotlightServices/SSEngagementHandler.h>
#import <SpotlightServices/PRSL2FeatureVectorProcessingContext.h>
#import <SpotlightServices/PRSL2FeatureVector.h>
#import <SpotlightServices/PRSRankingCosineComponents.h>
#import <SpotlightServices/PRSRankingPolicyManager.h>
#import <SpotlightServices/PRSModelFeedbackContext.h>
#import <SpotlightServices/PRSModelContext.h>
#import <SpotlightServices/PRSModelManager.h>

void* GetActivityName(void);
void* PRSLogCategoryDefault(void);
void* PRSLogInit(void);
void* PRSRankingConfig_addRankingAttrs(void);
void* PRSRankingItemRanker_copyRankerConfig(void);
void* PRSRankingItemRanker_copyRankerForQueryString(void);
void* PRSRankingItemRanker_copyRankingQueries(void);
void* PRSRankingItemRanker_rankQueryResults(void);
void* PRSRankingQueries(void);
void* ReleaseActivityName(void);
void* SPQueryDisplayNameFuzzyQuery(void);
void* SPQueryEscapeString(void);
void* allTimeWindow(void);
void* associatedL2FeatureFromSnippetFeature(void);
void* associatedResultSetL2FeatureFromSnippetFeature(void);
void* bundleFeatureHasAssociatedResultSetFeature(void);
void* comp(void);
void* dailyWindow(void);
void* days(void);
void* decodeImage(void);
void* decodeImageAlignment(void);
void* decodeImageArray(void);
void* decodeSFColumnAlignment(void);
void* decodeSFImageAlignment(void);
void* decodeSFRichTextToFormattedText(void);
void* decodeSFRowAlignment(void);
void* decodeSFTextAlignment(void);
void* decodeTextAlignment(void);
void* defaultValueForCrossAttributeFeature(void);
void* defaultValueForResultSetSnippetFeature(void);
void* getBinForValue(void);
void* hours(void);
void* json_parse(void);
void* json_utf8_string(void);
void* lowercase_keys(void);
void* maskFromRankingDescriptors(void);
void* minutes(void);
void* mt_log_Spotlight_rankingAnalytics(void);
void* numBitsSet(void);
void* prs_feature_population_ctx_destroy(void);
void* prs_feature_population_ctx_init(void);
void* prs_feature_population_ctx_reset_for_item(void);
void* prs_feature_population_ctx_reset_for_property(void);
void* prs_feature_population_ctx_reset_with_important_properties(void);
void* ranking_index_score_init(void);
void* recentWindow(void);
void* recommendedValuesList(void);
void* register_activity(void);
void* resultset_computation_ctx_destroy(void);
void* resultset_computation_ctx_init(void);
void* roundNum(void);
void* shouldUpdateCrossAttributeFeature(void);
void* updateMeEmailAddresses(void);
void* updatePhoneFavoritesQuery(void);
void* updateVIPList(void);
void* updateVIPRankingQuery(void);
void* updatedCrossAttributeFeature(void);
void* weeklyWindow(void);
void* weeks(void);

#endif
