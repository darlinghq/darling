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

#include <string.h>
#include "launcher-mode.h"

static bool enabled(const char* value)
{
	return value != NULL && (value[0] == '1' || value[0] == 't' || value[0] == 'T');
}

enum launcher_privilege launcherClassifyPrivilege(bool euidRoot, bool secureExec, bool capabilities)
{
	if (euidRoot)
		return LAUNCHER_PRIVILEGED;
	if (secureExec || capabilities)
		return LAUNCHER_PRIVILEGED_NOT_ROOT;
	return LAUNCHER_UNPRIVILEGED;
}

enum launcher_mode launcherSelectMode(bool privileged, const char* nonRoot, const char* rootless, bool defaultNonRoot)
{
	if (privileged)
		return LAUNCHER_MODE_ROOT;
	if (enabled(nonRoot))
		return LAUNCHER_MODE_NONROOT;
	if (enabled(rootless))
		return LAUNCHER_MODE_ROOTLESS;
	return defaultNonRoot ? LAUNCHER_MODE_NONROOT : LAUNCHER_MODE_NONE;
}

bool launcherInstallPrefix(enum launcher_mode mode, const char* compiledPrefix, const char* exePath, char* out, size_t size)
{
	static const char suffix[] = "/bin/darling";
	const char* chosen = compiledPrefix;
	size_t length;

	if (mode != LAUNCHER_MODE_ROOT && exePath != NULL && exePath[0] == '/')
	{
		size_t exeLength = strlen(exePath);

		if (exeLength > sizeof(suffix) - 1 && strcmp(exePath + exeLength - (sizeof(suffix) - 1), suffix) == 0)
		{
			length = exeLength - (sizeof(suffix) - 1);
			if (length >= size)
				return false;
			memcpy(out, exePath, length);
			out[length] = '\0';
			return true;
		}
	}

	length = strlen(chosen);
	if (length >= size)
		return false;
	memcpy(out, chosen, length + 1);
	return true;
}

const char* launcherPickLoginName(const char* login, const char* userEnv, bool userEnvIsUid,
	const char* lognameEnv, bool lognameEnvIsUid, const char* fallback)
{
	if (login != NULL && login[0] != '\0')
		return login;
	if (userEnv != NULL && userEnv[0] != '\0' && userEnvIsUid)
		return userEnv;
	if (lognameEnv != NULL && lognameEnv[0] != '\0' && lognameEnvIsUid)
		return lognameEnv;
	if (fallback != NULL && fallback[0] != '\0')
		return fallback;
	return NULL;
}
