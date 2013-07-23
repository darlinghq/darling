#ifndef AUDIOCONVERTERINTERNAL_H
#define AUDIOCONVERTERINTERNAL_H
#include "AudioConverter.h"

extern "C" {
//#include <libavresample/avresample.h>
#include <libavcodec/avcodec.h>
}

class AudioConverter
{
	AudioConverter(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat);
public:
	static OSStatus create(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat, AudioConverter** out);
	~AudioConverter();

	void flush();
	UInt32 frameSize() const { return m_sourceFormat.mBytesPerFrame; }
private:
	static enum AVCodecID CACodecToAV(const AudioStreamBasicDescription* desc);
private:
	AudioStreamBasicDescription m_sourceFormat, m_destinationFormat;
	AVCodecContext* m_decoder;
	AVCodecContext* m_encoder;
	//AVAudioResampleContext* m_resampler;
	bool m_hadData;
};

#endif


