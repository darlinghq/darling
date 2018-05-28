project(SecurityInterface)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "55109.30.2")

add_framework(SecurityInterface
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/SecurityInterface.m
        src/SFButtonNotBecomeKey.m
        src/SFAuthorizationRights.m
        src/SFAuthorizationView_ivars.m
        src/SFAuthorizationView.m
        src/SFChooseIdentityTableCellView.m
        src/SFCertificatePanel_ivars.m
        src/SFCertificatePanel.m
        src/SFCertificateTrustPanel_ivars.m
        src/SFCertificateTrustPanel.m
        src/SFCertificateView_ivars.m
        src/SFFlippedView.m
        src/SFCertificateView.m
        src/SFChooseIdentityPanel_ivars.m
        src/SFChooseIdentityPanel.m
        src/SFKeychainSavePanel_ivars.m
        src/SFKeychainSavePanel.m
        src/SFKeychainSettingsPanel_ivars.m
        src/SFAutoLockTextValue.m
        src/SFKeychainSettingsPanel.m
        src/SFEntropyBarView.m
        src/SFPasswordAsstController.m
        src/SFPasswordAsstModel.m
        src/SFPasswordAsstView.m
        src/SFPasswordAssistantInspectorController.m
        src/SFPasswordAssistantButton.m
        src/SFAnimatedLockButton.m
        src/PWALengthSliderCell.m
        src/PWALengthSlider.m
        src/SFPasswordErrorSource.m
        src/SFExpandableTextField.m
        src/SFCertificateGroupView.m
        src/SFIconView.m
        src/SFIconCache_ivars.m
        src/SFIconCache.m
        src/SFKeychainStorage.m
        src/SFAuthorizationPluginView.m
        src/AuthorizationSpecialEngineRef.m
        src/SFCertificateImportManager_ivars.m
        src/SFCertificateImportManager.m
        src/SFPasswordStrengthTextField.m
        src/SFPasswordStrengthController.m
        src/SheetSupport.m
        src/SFPasswordStrengthView_ivars.m
        src/SFPasswordStrengthView.m
        src/SFTokenHIPlugin.m
        src/SFTokenHIPluginSupport.m
        src/SFBasicTLV.m
        src/SFCertificateTrust_ivars.m
        src/SFCertificateTrust.m

    DEPENDENCIES
        system
        objc
        Foundation
)
