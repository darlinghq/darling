#include "AudioConverterInternal.h"
#include <CoreServices/MacErrors.h>
#include <util/debug.h>

// http://blinkingblip.wordpress.com/

__attribute__((constructor)) static void init_avcodec()
{
	avcodec_register_all();
}

OSStatus AudioConverterNew(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat, AudioConverterRef* outAudioConverter)
{
	return AudioConverter::create(inSourceFormat, inDestinationFormat, outAudioConverter);
}

OSStatus AudioConverterNewSpecific(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat, UInt32 inNumberClassDescriptions, AudioClassDescription* nClassDescriptions, AudioConverterRef* outAudioConverter)
{
	// FIXME
	return AudioConverterNew(inSourceFormat, inDestinationFormat, outAudioConverter);
}

OSStatus AudioConverterReset(AudioConverterRef inAudioConverter)
{
	if (inAudioConverter)
	{
		inAudioConverter->flush();
		return noErr;
	}
	else
		return paramErr;
}

OSStatus AudioConverterDispose(AudioConverterRef inAudioConverter)
{
	delete inAudioConverter;
	return noErr;
}

OSStatus AudioConverterGetProperty(AudioConverterRef inAudioConverter, AudioConverterPropertyID inPropertyID, UInt32 *ioPropertyDataSize, void *outPropertyData)
{
	STUB();
	return unimpErr;
}

OSStatus AudioConverterGetPropertyInfo(AudioConverterRef inAudioConverter, AudioConverterPropertyID inPropertyID, UInt32 *outSize, Boolean *outWritable)
{
	STUB();
	return unimpErr;
}

OSStatus AudioConverterSetProperty(AudioConverterRef inAudioConverter, AudioConverterPropertyID inPropertyID, UInt32 inPropertyDataSize, const void *inPropertyData)
{
	STUB();
	return unimpErr;
}

OSStatus AudioConverterConvertBuffer(AudioConverterRef inAudioConverter, UInt32 inInputDataSize, const void* inInputData, UInt32 *ioOutputDataSize, void *outOutputData)
{
	AudioBufferList in, out;
	OSStatus stat;

	in.mNumberBuffers = out.mNumberBuffers = 1;

	in.mBuffers[0].mNumberChannels = 2;
	out.mBuffers[0].mNumberChannels = 2;

	in.mBuffers[0].mDataByteSize = inInputDataSize;
	in.mBuffers[0].mData = const_cast<void*>(inInputData);

	out.mBuffers[0].mDataByteSize = *ioOutputDataSize;
	out.mBuffers[0].mData = outOutputData;

	stat = AudioConverterConvertComplexBuffer(inAudioConverter, inInputDataSize/inAudioConverter->frameSize(), &in, &out);
	*ioOutputDataSize = out.mBuffers[0].mDataByteSize;

	return stat;
}

OSStatus AudioConverterFillComplexBuffer(AudioConverterRef inAudioConverter, AudioConverterComplexInputDataProc inInputDataProc, void *inInputDataProcUserData, UInt32 *ioOutputDataPacketSize, AudioBufferList *outOutputData, AudioStreamPacketDescription*outPacketDescription)
{
	STUB();
	return unimpErr;
}

OSStatus AudioConverterConvertComplexBuffer(AudioConverterRef inAudioConverter, UInt32 inNumberPCMFrames, const AudioBufferList *inInputData, AudioBufferList *outOutputData)
{
	OSStatus status;
	UInt32 dataPacketSize;
	int64_t totalOutBytes = 0;

	AudioConverterComplexInputDataProc proc = [](AudioConverterRef audioConverter, UInt32* numberDataPackets, AudioBufferList* data, AudioStreamPacketDescription** dataPacketDescription, void* userData) -> OSStatus
	{
		const AudioBufferList *inInputData = (AudioBufferList*) inInputData;

		if (dataPacketDescription)
			*dataPacketDescription = nullptr;
		
		// TODO

		return unimpErr;
	};

	for (UInt32 i = 0; i < outOutputData->mNumberBuffers; i++)
		totalOutBytes += outOutputData->mBuffers[i].mDataByteSize;

	dataPacketSize = totalOutBytes / inAudioConverter->frameSize();

	status = AudioConverterFillComplexBuffer(inAudioConverter, proc, (void*) inInputData, &dataPacketSize, outOutputData, nullptr);
	return status;
}

AudioConverter::AudioConverter(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat)
    : m_sourceFormat(*inSourceFormat), m_destinationFormat(*inDestinationFormat), m_decoder(nullptr), m_encoder(nullptr), m_hadData(false)
{
}

void AudioConverter::flush()
{
	if (m_hadData)
	{
		avcodec_flush_buffers(m_decoder);
		avcodec_flush_buffers(m_encoder);
		avcodec_close(m_encoder);
		m_hadData = false;
	}
}

OSStatus AudioConverter::create(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat, AudioConverter** out)
{
	TRACE2(inSourceFormat, inDestinationFormat);
	
	// TODO: create a special case for when *inSourceFormat == *inDestinationFormat?

	AVCodec *codecIn, *codecOut;
	AVCodecContext *cIn;
	enum AVCodecID idIn, idOut;

	*out = nullptr;
	idIn = CACodecToAV(inSourceFormat);
	idOut = CACodecToAV(inDestinationFormat);

	if (idIn == AV_CODEC_ID_NONE || idOut == AV_CODEC_ID_NONE)
	{
		LOG << "AudioConverter::create(): Unsupported codec, format in = " << std::hex << inSourceFormat->mFormatID << ", out = " << inDestinationFormat->mFormatID << std::dec << std::endl;
		return paramErr;
	}

	codecIn = avcodec_find_decoder(idIn);
	codecOut = avcodec_find_encoder(idOut);

	if (!codecIn || !codecOut)
	{
		LOG << "AudioConverter::create(): avcodec_find_*() failed, format in = " << std::hex << inSourceFormat->mFormatID << ", out = " << inDestinationFormat->mFormatID << std::dec << std::endl;
		return paramErr;
	}

	*out = new AudioConverter(inSourceFormat, inDestinationFormat);

	(*out)->m_decoder = cIn = avcodec_alloc_context3(codecIn);
	
	if (inSourceFormat->mFormatID == kAudioFormatLinearPCM)
	{
		cIn->channels = inSourceFormat->mChannelsPerFrame;
		cIn->sample_rate = inSourceFormat->mSampleRate;
		
		LOG << "Converting from PCM with " << cIn->channels << " channels at " << cIn->sample_rate << " Hz\n";
	}
	
	if (avcodec_open2((*out)->m_decoder, codecIn, nullptr) < 0)
	{
		delete *out;
		LOG << "AudioConverter::create(): avcodec_open() failed, format in = " << std::hex << inSourceFormat->mFormatID << ", out = " << inDestinationFormat->mFormatID << std::dec << std::endl;

		return paramErr;
	}

	// The encoder will be initialized after we process the first packet
	(*out)->m_encoder = avcodec_alloc_context3(codecOut);

	return noErr;
}

AudioConverter::~AudioConverter()
{
	if (m_decoder)
		av_free(m_decoder);
	if (m_encoder)
		av_free(m_encoder);
	//if (m_resampler)
	//	avresample_free(&m_resampler);
}

enum AVCodecID AudioConverter::CACodecToAV(const AudioStreamBasicDescription* desc)
{
	switch (desc->mFormatID)
	{
		case kAudioFormatLinearPCM:
		{
			if (desc->mFormatFlags & kAudioFormatFlagIsFloat)
			{
				if (desc->mFormatFlags & kAudioFormatFlagIsBigEndian)
				{
					if (desc->mBitsPerChannel == 32)
						return AV_CODEC_ID_PCM_F32BE;
					else if (desc->mBitsPerChannel == 64)
						return AV_CODEC_ID_PCM_F64BE;
				}
				else
				{
					if (desc->mBitsPerChannel == 32)
						return AV_CODEC_ID_PCM_F32LE;
					else if (desc->mBitsPerChannel == 64)
						return AV_CODEC_ID_PCM_F64LE;
				}
			}
			else if (desc->mFormatFlags & kAudioFormatFlagIsSignedInteger)
			{
				enum AVCodecID cid;

				switch (desc->mBitsPerChannel)
				{
					case 8: cid = AV_CODEC_ID_PCM_S8; break;
					case 16: cid = AV_CODEC_ID_PCM_S16LE; break;
					case 24: cid = AV_CODEC_ID_PCM_S24LE; break;
					case 32: cid = AV_CODEC_ID_PCM_S32LE; break;
					default: return AV_CODEC_ID_NONE;
				}

				if (desc->mBitsPerChannel != 8 && desc->mFormatFlags & kAudioFormatFlagIsBigEndian)
					cid++;
				return cid;
			}
			else
			{
				enum AVCodecID cid;

				switch (desc->mBitsPerChannel)
				{
					case 8: cid = AV_CODEC_ID_PCM_U8; break;
					case 16: cid = AV_CODEC_ID_PCM_U16LE; break;
					case 24: cid = AV_CODEC_ID_PCM_U24LE; break;
					case 32: cid = AV_CODEC_ID_PCM_U32LE; break;
					default: return AV_CODEC_ID_NONE;
				}

				if (desc->mBitsPerChannel != 8 && desc->mFormatFlags & kAudioFormatFlagIsBigEndian)
					cid++;
				return cid;
			}
			return AV_CODEC_ID_NONE;
		}
		case kAudioFormatULaw:
			return AV_CODEC_ID_PCM_MULAW;
		case kAudioFormatALaw:
			return AV_CODEC_ID_PCM_ALAW;
		case kAudioFormatMPEGLayer1:
			return AV_CODEC_ID_MP1;
		case kAudioFormatMPEGLayer2:
			return AV_CODEC_ID_MP2;
		case kAudioFormatMPEGLayer3:
			return AV_CODEC_ID_MP3;
		case kAudioFormatAC3:
		case kAudioFormat60958AC3: // TODO: is this correct?
			return AV_CODEC_ID_AC3;
		case kAudioFormatAppleIMA4:
			return AV_CODEC_ID_ADPCM_IMA_DK4; // TODO: is this correct?
		case kAudioFormatMPEG4AAC:
		case kAudioFormatMPEG4AAC_HE:
		case kAudioFormatMPEG4AAC_LD:
		case kAudioFormatMPEG4AAC_ELD:
		case kAudioFormatMPEG4AAC_ELD_SBR:
		case kAudioFormatMPEG4AAC_ELD_V2:
		case kAudioFormatMPEG4AAC_HE_V2:
		case kAudioFormatMPEG4AAC_Spatial:
			return AV_CODEC_ID_AAC;
		case kAudioFormatMPEG4CELP:
			return AV_CODEC_ID_QCELP; // TODO: is this correct?
		case kAudioFormatAMR:
			return AV_CODEC_ID_AMR_NB;
		case kAudioFormatiLBC:
			return AV_CODEC_ID_ILBC;
		case kAudioFormatAppleLossless:
			return AV_CODEC_ID_APE;
		case kAudioFormatMicrosoftGSM:
			return AV_CODEC_ID_GSM_MS;
		case kAudioFormatMACE3:
			return AV_CODEC_ID_MACE3;
		case kAudioFormatMACE6:
			return AV_CODEC_ID_MACE6;
		default:
			return AV_CODEC_ID_NONE;
	}
}

