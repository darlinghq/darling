project(QuartzFilters)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(QuartzFilters
	BASE_PATH ${quartz_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "include/QuartzFilters"
)

add_framework(QuartzFilters
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "Quartz"

	SOURCES
		src/QFilterComponentDefaultProfile.m
		src/QConvolutionIsNotNone.m
		src/QuartzFilterInternal.m
		src/TargetComponentInfo.m
		src/QFilter.m
		src/QFilterComponentImageCompression.m
		src/QImageQualityTransformer.m
		src/QFilterPanelPrivate.m
		src/QFilterComponentImageMonoSampling.m
		src/QComponentAssignProfileCellView.m
		src/QFilterComponentPDFPolicies.m
		src/QCompressionIsJPEGTransformer.m
		src/QFilterComponentImageSampling.m
		src/QRectObject.m
		src/QIntentTuple.m
		src/QProfileTuple.m
		src/QFilterOutlineView.m
		src/QComponentTupleSetCellView.m
		src/QFilterTableCellView.m
		src/QFilterPDFX3Info.m
		src/AssignComponentInfo.m
		src/QEditableTextField.m
		src/QComponentImageSamplingCellView.m
		src/QuartzFilter.m
		src/QFilterView.m
		src/QFilterComponentConvertToProfile.m
		src/QValueIsNonzeroTransformer.m
		src/QFilterComponentComments.m
		src/QComponentInfoTuple.m
		src/QComponentImageMonoSamplingCellView.m
		src/QIntermediateTuple.m
		src/QSizeObject.m
		src/QFilterUndoController.m
		src/QProfileTupleCellView.m
		src/QuartzFilterManagerInternal.m
		src/IntermediateComponentInfo.m
		src/QuartzFilterView.m
		src/RenderingIntentInfo.m
		src/QFilterActionMenu.m
		src/QComponentDefaultProfileCellView.m
		src/QFilterComponentChild.m
		src/QFilterComponentPDFX3Options.m
		src/QFilterViewController.m
		src/QComponentPDFX3OptionsCellView.m
		src/QFilterButton.m
		src/QFilterPanel.m
		src/QFilterComponentImageConvolution.m
		src/QComponentConvertToProfileCellView.m
		src/QComponentIntermediateCellView.m
		src/QFilterInfo.m
		src/QFilterCategory.m
		src/QFilterComponentAssignProfile.m
		src/QFilterComponent.m
		src/QFilterComponentRenderingIntent.m
		src/DefaultComponentInfo.m
		src/QFilterComponentIntermediate.m
		src/QFilterComponentDomains.m
		src/QComponentRenderingIntentCellView.m
		src/QuartzFilterManager.m
		src/QFilterImageInfo.m
		src/QuartzFilters.m

	DEPENDENCIES
		system
		objc
		Foundation
)
