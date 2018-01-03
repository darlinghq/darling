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


#include <SpotlightServices/SpotlightServices.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* GetActivityName(void) {
    if (verbose) puts("STUB: GetActivityName called");
    return NULL;
}

void* PRSLogCategoryDefault(void) {
    if (verbose) puts("STUB: PRSLogCategoryDefault called");
    return NULL;
}

void* PRSLogInit(void) {
    if (verbose) puts("STUB: PRSLogInit called");
    return NULL;
}

void* PRSRankingConfig_addRankingAttrs(void) {
    if (verbose) puts("STUB: PRSRankingConfig_addRankingAttrs called");
    return NULL;
}

void* PRSRankingItemRanker_copyRankerConfig(void) {
    if (verbose) puts("STUB: PRSRankingItemRanker_copyRankerConfig called");
    return NULL;
}

void* PRSRankingItemRanker_copyRankerForQueryString(void) {
    if (verbose) puts("STUB: PRSRankingItemRanker_copyRankerForQueryString called");
    return NULL;
}

void* PRSRankingItemRanker_copyRankingQueries(void) {
    if (verbose) puts("STUB: PRSRankingItemRanker_copyRankingQueries called");
    return NULL;
}

void* PRSRankingItemRanker_rankQueryResults(void) {
    if (verbose) puts("STUB: PRSRankingItemRanker_rankQueryResults called");
    return NULL;
}

void* PRSRankingQueries(void) {
    if (verbose) puts("STUB: PRSRankingQueries called");
    return NULL;
}

void* ReleaseActivityName(void) {
    if (verbose) puts("STUB: ReleaseActivityName called");
    return NULL;
}

void* SPQueryDisplayNameFuzzyQuery(void) {
    if (verbose) puts("STUB: SPQueryDisplayNameFuzzyQuery called");
    return NULL;
}

void* SPQueryEscapeString(void) {
    if (verbose) puts("STUB: SPQueryEscapeString called");
    return NULL;
}

void* allTimeWindow(void) {
    if (verbose) puts("STUB: allTimeWindow called");
    return NULL;
}

void* associatedL2FeatureFromSnippetFeature(void) {
    if (verbose) puts("STUB: associatedL2FeatureFromSnippetFeature called");
    return NULL;
}

void* associatedResultSetL2FeatureFromSnippetFeature(void) {
    if (verbose) puts("STUB: associatedResultSetL2FeatureFromSnippetFeature called");
    return NULL;
}

void* bundleFeatureHasAssociatedResultSetFeature(void) {
    if (verbose) puts("STUB: bundleFeatureHasAssociatedResultSetFeature called");
    return NULL;
}

void* comp(void) {
    if (verbose) puts("STUB: comp called");
    return NULL;
}

void* dailyWindow(void) {
    if (verbose) puts("STUB: dailyWindow called");
    return NULL;
}

void* days(void) {
    if (verbose) puts("STUB: days called");
    return NULL;
}

void* decodeImage(void) {
    if (verbose) puts("STUB: decodeImage called");
    return NULL;
}

void* decodeImageAlignment(void) {
    if (verbose) puts("STUB: decodeImageAlignment called");
    return NULL;
}

void* decodeImageArray(void) {
    if (verbose) puts("STUB: decodeImageArray called");
    return NULL;
}

void* decodeSFColumnAlignment(void) {
    if (verbose) puts("STUB: decodeSFColumnAlignment called");
    return NULL;
}

void* decodeSFImageAlignment(void) {
    if (verbose) puts("STUB: decodeSFImageAlignment called");
    return NULL;
}

void* decodeSFRichTextToFormattedText(void) {
    if (verbose) puts("STUB: decodeSFRichTextToFormattedText called");
    return NULL;
}

void* decodeSFRowAlignment(void) {
    if (verbose) puts("STUB: decodeSFRowAlignment called");
    return NULL;
}

void* decodeSFTextAlignment(void) {
    if (verbose) puts("STUB: decodeSFTextAlignment called");
    return NULL;
}

void* decodeTextAlignment(void) {
    if (verbose) puts("STUB: decodeTextAlignment called");
    return NULL;
}

void* defaultValueForCrossAttributeFeature(void) {
    if (verbose) puts("STUB: defaultValueForCrossAttributeFeature called");
    return NULL;
}

void* defaultValueForResultSetSnippetFeature(void) {
    if (verbose) puts("STUB: defaultValueForResultSetSnippetFeature called");
    return NULL;
}

void* getBinForValue(void) {
    if (verbose) puts("STUB: getBinForValue called");
    return NULL;
}

void* hours(void) {
    if (verbose) puts("STUB: hours called");
    return NULL;
}

void* json_parse(void) {
    if (verbose) puts("STUB: json_parse called");
    return NULL;
}

void* json_utf8_string(void) {
    if (verbose) puts("STUB: json_utf8_string called");
    return NULL;
}

void* lowercase_keys(void) {
    if (verbose) puts("STUB: lowercase_keys called");
    return NULL;
}

void* maskFromRankingDescriptors(void) {
    if (verbose) puts("STUB: maskFromRankingDescriptors called");
    return NULL;
}

void* minutes(void) {
    if (verbose) puts("STUB: minutes called");
    return NULL;
}

void* mt_log_Spotlight_rankingAnalytics(void) {
    if (verbose) puts("STUB: mt_log_Spotlight_rankingAnalytics called");
    return NULL;
}

void* numBitsSet(void) {
    if (verbose) puts("STUB: numBitsSet called");
    return NULL;
}

void* prs_feature_population_ctx_destroy(void) {
    if (verbose) puts("STUB: prs_feature_population_ctx_destroy called");
    return NULL;
}

void* prs_feature_population_ctx_init(void) {
    if (verbose) puts("STUB: prs_feature_population_ctx_init called");
    return NULL;
}

void* prs_feature_population_ctx_reset_for_item(void) {
    if (verbose) puts("STUB: prs_feature_population_ctx_reset_for_item called");
    return NULL;
}

void* prs_feature_population_ctx_reset_for_property(void) {
    if (verbose) puts("STUB: prs_feature_population_ctx_reset_for_property called");
    return NULL;
}

void* prs_feature_population_ctx_reset_with_important_properties(void) {
    if (verbose) puts("STUB: prs_feature_population_ctx_reset_with_important_properties called");
    return NULL;
}

void* ranking_index_score_init(void) {
    if (verbose) puts("STUB: ranking_index_score_init called");
    return NULL;
}

void* recentWindow(void) {
    if (verbose) puts("STUB: recentWindow called");
    return NULL;
}

void* recommendedValuesList(void) {
    if (verbose) puts("STUB: recommendedValuesList called");
    return NULL;
}

void* register_activity(void) {
    if (verbose) puts("STUB: register_activity called");
    return NULL;
}

void* resultset_computation_ctx_destroy(void) {
    if (verbose) puts("STUB: resultset_computation_ctx_destroy called");
    return NULL;
}

void* resultset_computation_ctx_init(void) {
    if (verbose) puts("STUB: resultset_computation_ctx_init called");
    return NULL;
}

void* roundNum(void) {
    if (verbose) puts("STUB: roundNum called");
    return NULL;
}

void* shouldUpdateCrossAttributeFeature(void) {
    if (verbose) puts("STUB: shouldUpdateCrossAttributeFeature called");
    return NULL;
}

void* updateMeEmailAddresses(void) {
    if (verbose) puts("STUB: updateMeEmailAddresses called");
    return NULL;
}

void* updatePhoneFavoritesQuery(void) {
    if (verbose) puts("STUB: updatePhoneFavoritesQuery called");
    return NULL;
}

void* updateVIPList(void) {
    if (verbose) puts("STUB: updateVIPList called");
    return NULL;
}

void* updateVIPRankingQuery(void) {
    if (verbose) puts("STUB: updateVIPRankingQuery called");
    return NULL;
}

void* updatedCrossAttributeFeature(void) {
    if (verbose) puts("STUB: updatedCrossAttributeFeature called");
    return NULL;
}

void* weeklyWindow(void) {
    if (verbose) puts("STUB: weeklyWindow called");
    return NULL;
}

void* weeks(void) {
    if (verbose) puts("STUB: weeks called");
    return NULL;
}
