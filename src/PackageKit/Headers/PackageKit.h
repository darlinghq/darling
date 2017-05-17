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

#import <Foundation/Foundation.h>
#import <PackageKit/PKInstallStateHelper.h>
#import <PackageKit/PKDataSizeFormatter.h>
#import <PackageKit/PKDataSizeValueTransformer.h>
#import <PackageKit/PKTimeRemainingFormatter.h>
#import <PackageKit/PKTimeRemainingValueTransformer.h>
#import <PackageKit/PKInstallClient.h>
#import <PackageKit/_PKInstallClientConnection.h>
#import <PackageKit/PKInstallDaemon.h>
#import <PackageKit/PKArchive.h>
#import <PackageKit/PKMutableArchive.h>
#import <PackageKit/PKArchiveSignature.h>
#import <PackageKit/PKExtendedAttributeEnumerator.h>
#import <PackageKit/PKArchiveSigner.h>
#import <PackageKit/PKFolderArchive.h>
#import <PackageKit/PKFolderArchiveSignature.h>
#import <PackageKit/XARDirectoryEnumerator.h>
#import <PackageKit/PKXARArchive.h>
#import <PackageKit/PKMutableXARArchive.h>
#import <PackageKit/PKXARArchiveSignature.h>
#import <PackageKit/PKInstall.h>
#import <PackageKit/PKInstallSandbox.h>
#import <PackageKit/PKInstallError.h>
#import <PackageKit/PKInstallRequest.h>
#import <PackageKit/PKBOM.h>
#import <PackageKit/PKMutableBOM.h>
#import <PackageKit/PKBOMDirectoryEnumerator.h>
#import <PackageKit/PKComponent.h>
#import <PackageKit/PKBundleComponent.h>
#import <PackageKit/PKJaguarPackage.h>
#import <PackageKit/PKLeopardPackage.h>
#import <PackageKit/PKMutableLeopardPackage.h>
#import <PackageKit/PKPackage.h>
#import <PackageKit/PKMutablePackage.h>
#import <PackageKit/_PKPackageCreationFileDigest.h>
#import <PackageKit/PKPackageInfo.h>
#import <PackageKit/PKTrust.h>
#import <PackageKit/PKReceipt.h>
#import <PackageKit/PKMutableReceipt.h>
#import <PackageKit/PKDistribution.h>
#import <PackageKit/PKDownloader.h>
#import <PackageKit/PKDownloaderSpeedTracker.h>
#import <PackageKit/PKDownloaderQueueElement.h>
#import <PackageKit/PKDownloaderQueue.h>
#import <PackageKit/PKPackageReference.h>
#import <PackageKit/PKPackageSourceManager.h>
#import <PackageKit/PKURLConnection.h>
#import <PackageKit/_PKURLConnectionDelegate.h>
#import <PackageKit/PKArchiveProduct.h>
#import <PackageKit/PKNetEnabledProduct.h>
#import <PackageKit/PKProduct.h>
#import <PackageKit/PKMutableProduct.h>
#import <PackageKit/PKProductInfo.h>
#import <PackageKit/PKStreamingProduct.h>
#import <PackageKit/PKInstallHistory.h>
#import <PackageKit/PKScript.h>
#import <PackageKit/PKInstallTask.h>
#import <PackageKit/PKFirmwareInstallationController.h>
#import <PackageKit/PKInstallAnalyzer.h>
#import <PackageKit/PKBundleComponentVersion.h>
#import <PackageKit/PKPackageRequirements.h>
#import <PackageKit/PKDistributionScript.h>
#import <PackageKit/PKDistributionChoice.h>
#import <PackageKit/PKDistributionPackageReference.h>
#import <PackageKit/PKPackageSpecifier.h>
#import <PackageKit/PKDistributionContainer.h>
#import <PackageKit/PKMutableDistributionContainer.h>
#import <PackageKit/PKFlatDistributionContainer.h>
#import <PackageKit/PKArchiveDistributionContainer.h>
#import <PackageKit/PKSecureNetEnabledProduct.h>
#import <PackageKit/PKInstallOperationController.h>
#import <PackageKit/PKInstallOperation.h>
#import <PackageKit/PKExtractInstallOperation.h>
#import <PackageKit/PKRunPackageScriptInstallOperation.h>
#import <PackageKit/PKPatchFilesInstallOperation.h>
#import <PackageKit/PKRelocateComponentsInstallOperation.h>
#import <PackageKit/PKObsoletionInstallOperation.h>
#import <PackageKit/PKAddExtendedAttributesInstallOperation.h>
#import <PackageKit/PKDYLDCacheInstallOperation.h>
#import <PackageKit/PKExtendedAttribute.h>
#import <PackageKit/PKSetupDeferredInstallOperation.h>
#import <PackageKit/PKShoveInstallOperation.h>
#import <PackageKit/PKKextCacheInstallOperation.h>
#import <PackageKit/PKLSRegisterInstallOperation.h>
#import <PackageKit/PKWriteReceiptsInstallOperation.h>
#import <PackageKit/PKDownload.h>
#import <PackageKit/PKFileDownload.h>
#import <PackageKit/PKURLDownload.h>
#import <PackageKit/PKPayloadCopier.h>
#import <PackageKit/PKProductUnarchiver.h>
#import <PackageKit/PKProductArchiver.h>
#import <PackageKit/PKCheckError.h>
#import <PackageKit/_PKCheckErrorRecoveryAttempter.h>
#import <PackageKit/PKFirmwareBundleComponentModel.h>
#import <PackageKit/PKFirmwareBundleComponent.h>
#import <PackageKit/PKAddRestrictedRootFlagInstallOperation.h>
#import <PackageKit/PKPackageChecker.h>
#import <PackageKit/PKProductChecker.h>
#import <PackageKit/PKTrustAdditions.h>
#import <PackageKit/PKInstallManifest.h>
#import <PackageKit/PKUpdateEFWCacheInstallOperation.h>
#import <PackageKit/PKCleanEFWCacheInstallOperation.h>
#import <PackageKit/PKStreamingXARArchive.h>
#import <PackageKit/PKPatchAndUpdateInstallOperation.h>
#import <PackageKit/PKWriteMASReceiptInstallOperation.h>
#import <PackageKit/PKBundleStorage.h>
#import <PackageKit/PKCFBundleStorage.h>
#import <PackageKit/PKSafariExtensionBundleStorage.h>
#import <PackageKit/PKMachOWithInfoPlistBundleStorage.h>
#import <PackageKit/PKSigningIdentity.h>
#import <PackageKit/PKSignedContainer.h>
#import <PackageKit/PKMutableSignedContainer.h>
#import <PackageKit/_PKSignedContainerCopyCancelHandler.h>
#import <PackageKit/PKInstallableObject.h>
#import <PackageKit/PKInstallSandboxManager.h>
#import <PackageKit/PKPrepareForCommitInstallOperation.h>
#import <PackageKit/PKPrepareDiskInstallOperation.h>
#import <PackageKit/PKXPCCacheInstallOperation.h>
#import <PackageKit/PKVerifyMASPayloadInstallOperation.h>
#import <PackageKit/PKInstallCommitManager.h>
#import <PackageKit/PKFolderArchiveProduct.h>
#import <PackageKit/PKDeferredInstallManager.h>
#import <PackageKit/PKInstallDaemonClient.h>
