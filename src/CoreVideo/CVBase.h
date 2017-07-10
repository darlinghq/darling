#ifndef CVBASE_H
#define CVBASE_H
#include <stdint.h>
#include <MacTypes.h>

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

#endif

