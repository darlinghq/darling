#ifndef DMGSTREAM_H
#define DMGSTREAM_H
#include <stdint.h>
#include "IStream.h"
#include "../util/MSB.h"

// TODO: copyright vo1tur

// base64: http://www.ioncannon.net/programming/34/howto-base64-encode-with-cc-and-openssl/

struct kolyblk {
	MSB<uint32_t> Signature;
	MSB<uint32_t> Version;
	MSB<uint32_t> HeaderSize;
	MSB<uint32_t> Flags;
	MSB<uint64_t> RunningDataForkOffset;
	MSB<uint64_t> DataForkOffset;
	MSB<uint64_t> DataForkLength;
	MSB<uint64_t> RsrcForkOffset;
	MSB<uint64_t> RsrcForkLength;
	MSB<uint32_t> SegmentNumber;
	MSB<uint32_t> SegmentCount;
	MSB<uint32_t> SegmentID1;
	MSB<uint32_t> SegmentID2;
	MSB<uint32_t> SegmentID3;
	MSB<uint32_t> SegmentID4;
	MSB<uint32_t> DataForkChecksumType;
	MSB<uint32_t> Reserved1;
	MSB<uint32_t> DataForkChecksum;
	MSB<uint32_t> Reserved2;
	char Reserved3[120];
	MSB<uint64_t> XMLOffset;
	MSB<uint64_t> XMLLength;
	char Reserved4[120];
	MSB<uint32_t> MasterChecksumType;
	MSB<uint32_t> Reserved5;
	MSB<uint32_t> MasterChecksum;
	MSB<uint32_t> Reserved6;
	char Reserved7[120];
	MSB<uint32_t> ImageVariant;
	MSB<uint64_t> SectorCount;
	char Reserved8[12];
} __attribute__ ((__packed__));

struct _mishblk {
	MSB<uint32_t> BlocksSignature;
	MSB<uint32_t> InfoVersion;
	MSB<uint64_t> FirstSectorNumber;
	MSB<uint64_t> SectorCount;
	MSB<uint64_t> DataStart;
	MSB<uint32_t> DecompressedBufferRequested;
	MSB<uint32_t> BlocksDescriptor;
	char Reserved1[24];
	MSB<uint32_t> ChecksumType;
	MSB<uint32_t> Reserved2;
	MSB<uint32_t> Checksum;
	MSB<uint32_t> Reserved3;
	char Reserved4[120];
	MSB<uint32_t> BlocksRunCount;
	char *Data;
} __attribute__ ((__packed__));

class DMGStream : public IStream
{
private:
	DMGStream(IStream* stream);
public:
	static IStream* open(IStream* stream);
	virtual ~DMGStream();
	virtual ssize_t read(void* buffer, ssize_t bytes) override;
	virtual int64_t seek(int64_t offset, int whence) override;
	virtual int64_t length() const override;
private:
	IStream* m_stream;
	kolyblk m_koly;
};

#endif

