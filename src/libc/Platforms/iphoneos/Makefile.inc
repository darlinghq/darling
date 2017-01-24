#
# Selectable features for iPhone
#

# Legacy *64 APIs
#FEATURE_LEGACY_64_APIS = 1

# Legacy crt1.o environ support
#FEATURE_LEGACY_CRT1_ENVIRON = 1

# Legacy rune APIs
#FEATURE_LEGACY_RUNE_APIS = 1

# Legacy utmp APIs
#FEATURE_LEGACY_UTMP_APIS = 1

# OSThermalNotification APIs
FEATURE_THERM_NOTIFICATION_APIS = 1

# Move localtime to /var/db/timezone
FEATURE_MOVE_LOCALTIME = 1

# Use TZDIR symlink in /var/db
FEATURE_TZDIR_SYMLINK = 1

.if RC_ProjectName _Sim$
FEATURE_MOVE_LOCALTIME = 0
FEATURE_TZDIR_SYMLINK = 0
.endif

# No pre-1050 variants (should match sys/cdefs.h)
FEATURE_ONLY_1050_VARIANTS = 1

# No legacy variants (should match sys/cdefs.h)
FEATURE_ONLY_UNIX_CONFORMANCE = 1

# Only 64-bit ino_t (should match sys/cdefs.h)
FEATURE_ONLY_64_BIT_INO_T = 1

# Patch 3333969
#FEATURE_PATCH_3333969 = 1

# Patch 3417676
#FEATURE_PATCH_3417676 = 1

# plockstat dtrace support
#FEATURE_PLOCKSTAT = 1

# Timezone change notification
FEATURE_TIMEZONE_CHANGE_NOTIFICATION = 1

# Smaller stdio buffers
FEATURE_SMALL_STDIOBUF = 1

# Disable registration of specific signals (<rdar://problem/21952708>)
# FEATURE_SIGNAL_RESTRICTION = 0
