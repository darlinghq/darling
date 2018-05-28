#ifndef CVBASE_H
#define CVBASE_H

#include <TargetConditionals.h>
#include <Availability.h>
#include <AvailabilityMacros.h>
#include <CoreFoundation/CFBase.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define CV_BRIDGED_TYPE(type)       CF_BRIDGED_TYPE(type)
#define CV_NULLABLE __nullable
#define CV_NONNULL __nonnull
#define CV_EXPORT __attribute__((visibility("default"))) CF_EXPORT
#define CV_INLINE CF_INLINE
#define CV_RETURNS_RETAINED_PARAMETER		CF_RETURNS_RETAINED
#define CV_RELEASES_ARGUMENT        CF_RELEASES_ARGUMENT

typedef uint64_t CVOptionFlags;

typedef struct CVSMPTETime
{
	SInt16 subframes;
	SInt16 subframeDivisor;
	UInt32 counter;
	UInt32 type;
	UInt32 flags;
	SInt16 hours;
	SInt16 minutes;
	SInt16 seconds;
	SInt16 frames;
} CVSMPTETime;

typedef CF_ENUM(uint32_t, CVSMPTETimeType)
{
    kCVSMPTETimeType24        = 0,
    kCVSMPTETimeType25        = 1,
    kCVSMPTETimeType30Drop    = 2,
    kCVSMPTETimeType30        = 3,
    kCVSMPTETimeType2997      = 4,
    kCVSMPTETimeType2997Drop  = 5,
    kCVSMPTETimeType60        = 6,
    kCVSMPTETimeType5994      = 7
};

typedef CF_OPTIONS(uint32_t, CVSMPTETimeFlags)
{
    kCVSMPTETimeValid     = (1L << 0),
    kCVSMPTETimeRunning   = (1L << 1)
};


typedef CF_OPTIONS(int32_t, CVTimeFlags) {
    kCVTimeIsIndefinite = 1 << 0
};

typedef struct
{
    int64_t	    timeValue;
    int32_t	    timeScale;
    int32_t	    flags;
} CVTime;

typedef struct CVTimeStamp
{
	uint32_t version;
	int32_t videoTimeScale;
	int64_t videoTime;
	uint64_t hostTime;
	double rateScalar;
	int64_t videoRefreshPeriod;
	CVSMPTETime smpteTime;
	uint64_t flags;
	uint64_t reserved;
} CVTimeStamp;

typedef CF_OPTIONS(uint64_t, CVTimeStampFlags)
{
    kCVTimeStampVideoTimeValid              = (1L << 0),
    kCVTimeStampHostTimeValid               = (1L << 1),
    kCVTimeStampSMPTETimeValid              = (1L << 2),
    kCVTimeStampVideoRefreshPeriodValid     = (1L << 3),
    kCVTimeStampRateScalarValid             = (1L << 4),
    kCVTimeStampTopField                    = (1L << 16),
    kCVTimeStampBottomField                 = (1L << 17),
    kCVTimeStampVideoHostTimeValid          = (kCVTimeStampVideoTimeValid | kCVTimeStampHostTimeValid),
    kCVTimeStampIsInterlaced                = (kCVTimeStampTopField | kCVTimeStampBottomField)
};

CV_EXPORT const CVTime kCVZeroTime;
CV_EXPORT const CVTime kCVIndefiniteTime;

#if defined(__cplusplus)
}
#endif

#endif

