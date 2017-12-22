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


#ifndef _SecurityInterface_H_
#define _SecurityInterface_H_

#import <Foundation/Foundation.h>

#import <SecurityInterface/AuthenticationHintsDelegate.h>
#import <SecurityInterface/AuthenticationHintsProvider.h>
#import <SecurityInterface/LAUIAuthenticationSheetDelegate.h>
#import <SecurityInterface/NSOpenSavePanelDelegate.h>
#import <SecurityInterface/NSSplitViewDelegate.h>
#import <SecurityInterface/SFTokenHIPlugin.h>
#import <SecurityInterface/SFButtonNotBecomeKey.h>
#import <SecurityInterface/SFAuthorizationRights.h>
#import <SecurityInterface/SFAuthorizationView_ivars.h>
#import <SecurityInterface/SFAuthorizationView.h>
#import <SecurityInterface/SFChooseIdentityTableCellView.h>
#import <SecurityInterface/SFCertificatePanel_ivars.h>
#import <SecurityInterface/SFCertificatePanel.h>
#import <SecurityInterface/SFCertificateTrustPanel_ivars.h>
#import <SecurityInterface/SFCertificateTrustPanel.h>
#import <SecurityInterface/SFCertificateView_ivars.h>
#import <SecurityInterface/SFFlippedView.h>
#import <SecurityInterface/SFCertificateView.h>
#import <SecurityInterface/SFChooseIdentityPanel_ivars.h>
#import <SecurityInterface/SFChooseIdentityPanel.h>
#import <SecurityInterface/SFKeychainSavePanel_ivars.h>
#import <SecurityInterface/SFKeychainSavePanel.h>
#import <SecurityInterface/SFKeychainSettingsPanel_ivars.h>
#import <SecurityInterface/SFAutoLockTextValue.h>
#import <SecurityInterface/SFKeychainSettingsPanel.h>
#import <SecurityInterface/SFEntropyBarView.h>
#import <SecurityInterface/SFPasswordAsstController.h>
#import <SecurityInterface/SFPasswordAsstModel.h>
#import <SecurityInterface/SFPasswordAsstView.h>
#import <SecurityInterface/SFPasswordAssistantInspectorController.h>
#import <SecurityInterface/SFPasswordAssistantButton.h>
#import <SecurityInterface/SFAnimatedLockButton.h>
#import <SecurityInterface/PWALengthSliderCell.h>
#import <SecurityInterface/PWALengthSlider.h>
#import <SecurityInterface/SFPasswordErrorSource.h>
#import <SecurityInterface/SFExpandableTextField.h>
#import <SecurityInterface/SFCertificateGroupView.h>
#import <SecurityInterface/SFIconView.h>
#import <SecurityInterface/SFIconCache_ivars.h>
#import <SecurityInterface/SFIconCache.h>
#import <SecurityInterface/SFKeychainStorage.h>
#import <SecurityInterface/SFAuthorizationPluginView.h>
#import <SecurityInterface/AuthorizationSpecialEngineRef.h>
#import <SecurityInterface/SFCertificateImportManager_ivars.h>
#import <SecurityInterface/SFCertificateImportManager.h>
#import <SecurityInterface/SFPasswordStrengthTextField.h>
#import <SecurityInterface/SFPasswordStrengthController.h>
#import <SecurityInterface/SheetSupport.h>
#import <SecurityInterface/SFPasswordStrengthView_ivars.h>
#import <SecurityInterface/SFPasswordStrengthView.h>
#import <SecurityInterface/SFTokenHIPlugin.h>
#import <SecurityInterface/SFTokenHIPluginSupport.h>
#import <SecurityInterface/SFBasicTLV.h>
#import <SecurityInterface/SFCertificateTrust_ivars.h>
#import <SecurityInterface/SFCertificateTrust.h>

void* _CommonCocoaPanels_ChooseIdentity(void);
void* _CommonCocoaPanels_ChooseIdentitySheet(void);
void* _CommonCocoaPanels_CreateKeychainPanel(void);
void* _CommonCocoaPanels_DisplayCertificate(void);
void* _CommonCocoaPanels_DisplayCertificateGroup(void);
void* _CommonCocoaPanels_KeychainSettingsPanel(void);
void* _CommonCocoaPanels_SecEditTrust(void);
void* _CommonCocoaPanels_SecEditTrustSheet(void);

#endif
