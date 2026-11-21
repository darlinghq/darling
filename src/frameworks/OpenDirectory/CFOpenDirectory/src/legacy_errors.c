/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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


/*
 Conversion of OpenDirectory error codes (kODError*) to the status codes of the
 legacy DirectoryService API (tDirStatus), used by DirectoryService.framework
 when it reports a failure of the calls it forwards to this framework.
*/

#include <CFOpenDirectory/CFOpenDirectoryPriv.h>
#include <CFOpenDirectory/CFOpenDirectoryConstants.h>
#include <CoreFoundation/CoreFoundation.h>

static const struct {
	CFIndex odCode;
	tDirStatus legacyStatus;
} legacyErrors[] = {
	{ kODErrorSessionLocalOnlyDaemonInUse, eDSLocalDSDaemonInUse },
	{ kODErrorSessionNormalDaemonInUse, eDSNormalDSDaemonInUse },
	{ kODErrorSessionDaemonNotRunning, eServerNotRunning },
	{ kODErrorSessionDaemonRefused, eDSOpenFailed },
	{ kODErrorSessionProxyCommunicationError, eDSTCPSendError },
	{ kODErrorSessionProxyVersionMismatch, eDSVersionMismatch },
	{ kODErrorSessionProxyIPUnreachable, eDSIPUnreachable },
	{ kODErrorSessionProxyUnknownHost, eDSUnknownHost },
	{ kODErrorNodeUnknownName, eDSOpenNodeFailed },
	{ kODErrorNodeDisabled, ePlugInNotActive },
	{ kODErrorNodeConnectionFailed, eDSCannotAccessSession },
	{ kODErrorNodeUnknownHost, eDSBogusServer },
	{ kODErrorQueryInvalidMatchType, eDSUnknownMatchType },
	{ kODErrorQueryUnsupportedMatchType, eDSUnSupportedMatchType },
	{ kODErrorRecordReadOnlyNode, eDSReadOnly },
	{ kODErrorRecordPermissionError, eDSPermissionError },
	{ kODErrorRecordParameterError, eParameterError },
	{ kODErrorRecordInvalidType, eDSInvalidRecordType },
	{ kODErrorRecordAlreadyExists, eDSRecordAlreadyExists },
	{ kODErrorRecordTypeDisabled, eDSRecordTypeDisabled },
	{ kODErrorRecordNoLongerExists, eDSRecordNotFound },
	{ kODErrorRecordAttributeUnknownType, eDSNoStdMappingAvailable },
	{ kODErrorRecordAttributeNotFound, eDSAttributeNotFound },
	{ kODErrorRecordAttributeValueSchemaError, eDSSchemaError },
	{ kODErrorRecordAttributeValueNotFound, eDSAttributeValueNotFound },
	{ kODErrorCredentialsInvalid, eDSAuthFailed },
	{ kODErrorCredentialsMethodNotSupported, eDSAuthMethodNotSupported },
	{ kODErrorCredentialsNotAuthorized, eDSNotAuthorized },
	{ kODErrorCredentialsParameterError, eDSAuthParameterError },
	{ kODErrorCredentialsOperationFailed, eDSOperationFailed },
	{ kODErrorCredentialsServerUnreachable, eDSServiceUnavailable },
	{ kODErrorCredentialsServerNotFound, eDSAuthNoAuthServerFound },
	{ kODErrorCredentialsServerError, eDSInvalidSession },
	{ kODErrorCredentialsServerTimeout, eDSAuthMasterUnreachable },
	{ kODErrorCredentialsContactMaster, eDSContactMaster },
	{ kODErrorCredentialsServerCommunicationError, eDSBadPacket },
	{ kODErrorCredentialsAccountNotFound, eDSAuthUnknownUser },
	{ kODErrorCredentialsAccountDisabled, eDSAuthAccountDisabled },
	{ kODErrorCredentialsAccountExpired, eDSAuthAccountExpired },
	{ kODErrorCredentialsAccountInactive, eDSAuthAccountInactive },
	{ kODErrorCredentialsPasswordExpired, eDSAuthPasswordExpired },
	{ kODErrorCredentialsPasswordChangeRequired, eDSAuthNewPasswordRequired },
	{ kODErrorCredentialsPasswordQualityFailed, eDSAuthPasswordQualityCheckFailed },
	{ kODErrorCredentialsPasswordTooShort, eDSAuthPasswordTooShort },
	{ kODErrorCredentialsPasswordTooLong, eDSAuthPasswordTooLong },
	{ kODErrorCredentialsPasswordNeedsLetter, eDSAuthPasswordNeedsLetter },
	{ kODErrorCredentialsPasswordNeedsDigit, eDSAuthPasswordNeedsDigit },
	{ kODErrorCredentialsPasswordChangeTooSoon, eDSAuthPasswordChangeTooSoon },
	{ kODErrorCredentialsInvalidLogonHours, eDSAuthInvalidLogonHours },
	{ kODErrorCredentialsInvalidComputer, eDSAuthInvalidComputer },
	{ kODErrorCredentialsPasswordUnrecoverable, eDSUnrecoverablePassword },
	{ kODErrorPluginOperationNotSupported, eNotYetImplemented },
	{ kODErrorPluginError, eUndefinedError },
	{ kODErrorDaemonError, eServerError },
};

tDirStatus ODConvertToLegacyErrorCode(CFIndex code)
{
	// Zero and negative codes already are legacy statuses (eDSNoErr is 0)
	if (code <= 0)
		return (tDirStatus) code;

	for (size_t i = 0; i < sizeof(legacyErrors) / sizeof(legacyErrors[0]); i++)
	{
		if (legacyErrors[i].odCode == code)
			return legacyErrors[i].legacyStatus;
	}

	return eUndefinedError;
}
