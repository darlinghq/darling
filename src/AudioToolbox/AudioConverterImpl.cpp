#include "AudioConverterImpl.h"
#include <CoreServices/MacErrors.h>
#include <util/debug.h>
#include <stdexcept>
#include <cstring>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/mem.h>
}

// http://blinkingblip.wordpress.com/

__attribute__((constructor)) static void init_avcodec()
{
	avcodec_register_all();
}

AudioConverter::AudioConverter(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat)
    : m_sourceFormat(*inSourceFormat), m_destinationFormat(*inDestinationFormat), m_decoder(nullptr), m_encoder(nullptr)
{
	memset(&m_avpkt, 0, sizeof(m_avpkt));
}

void AudioConverter::flush()
{
	TRACE();
	avcodec_flush_buffers(m_decoder);
	avcodec_flush_buffers(m_encoder);
	//avcodec_close(m_encoder);
}

OSStatus AudioConverter::create(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat, AudioConverter** out)
{
	TRACE2(inSourceFormat, inDestinationFormat);
	
	// TODO: non-interleaved audio

	AVCodec *codecIn, *codecOut;
	AVCodecContext *cIn;
	AVCodecContext *cOut;
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
	(*out)->m_encoder = cOut = avcodec_alloc_context3(codecOut);
	(*out)->m_codecIn = codecIn;
	(*out)->m_codecOut = codecOut;

	return noErr;
}

void AudioConverter::initEncoder()
{
	int err;
	
	if (m_encoderInitialized)
		throw std::logic_error("Encoder already initialized");
	
	m_encoder->codec_type = AVMEDIA_TYPE_AUDIO;
	m_encoder->bit_rate = m_outBitRate;
	m_encoder->channels = m_destinationFormat.mChannelsPerFrame;
	m_encoder->sample_rate = m_destinationFormat.mSampleRate;
	m_encoder->channel_layout = CAChannelCountToLayout(m_destinationFormat.mChannelsPerFrame);
	m_encoder->sample_fmt = CACodecSampleFormat(&m_destinationFormat);
	
	err = avcodec_open2(m_encoder, m_codecOut, 0);
	if (err < 0)
		throw std::runtime_error("avcodec_open2() failed for encoder");
	
	allocateBuffers();
	m_encoderInitialized = true;
}

void AudioConverter::allocateBuffers()
{
	m_audioFrame = av_frame_alloc();
	
	m_audioFrame->nb_samples = 4096;
	m_audioFrame->format = m_encoder->sample_fmt;
	m_audioFrame->channel_layout = m_encoder->channel_layout;

	
	int audioSampleBuffer_size = av_samples_get_buffer_size(nullptr, m_encoder->channels, m_audioFrame->nb_samples, m_encoder->sample_fmt, 0);
	void* audioSampleBuffer = (uint8_t*) av_malloc(audioSampleBuffer_size);

	if (!audioSampleBuffer)
	{
		ERROR() << "Failed to allocate sample buffer";
		//return paramErr; // TODO
	}

	// Setup the data pointers in the AVFrame
	if (avcodec_fill_audio_frame(m_audioFrame, m_encoder->channels, m_encoder->sample_fmt, (const uint8_t*) audioSampleBuffer, audioSampleBuffer_size, 0 ) < 0)
	{
		ERROR() << "Could not set up audio frame";
		//return paramErr; // TODO
	}
}

AudioConverter::~AudioConverter()
{
	TRACE();
	if (m_decoder)
		avcodec_close(m_decoder);
	if (m_encoder)
		avcodec_close(m_encoder);
	if (m_audioFrame)
		av_free(m_audioFrame);
	if (m_resampler)
		avresample_free(&m_resampler);
}

template <typename T> OSStatus setPropertyT(UInt32 inPropertyDataSize, T* localProperty, const void* propertySource)
{
	const T* t = static_cast<const T*>(propertySource);
	
	if (inPropertyDataSize != sizeof(T))
		return kAudioConverterErr_BadPropertySizeError;
	
	*localProperty = *t;
	return noErr;
}

OSStatus AudioConverter::setProperty(AudioConverterPropertyID inPropertyID, UInt32 inPropertyDataSize, const void *inPropertyData)
{
	switch (inPropertyID)
	{
		case kAudioConverterEncodeBitRate:
		{
			return setPropertyT(inPropertyDataSize, &m_outBitRate, inPropertyData);
		}
		case kAudioConverterInputChannelLayout:
		{
		}
		case kAudioConverterOutputChannelLayout:
		{
		}
		case kAudioConverterCurrentOutputStreamDescription:
		{
			return kAudioConverterErr_PropertyNotSupported;
			//return setPropertyT(inPropertyDataSize, &m_sourceFormat, inPropertyData);
		}
		case kAudioConverterCurrentInputStreamDescription:
		{
			return kAudioConverterErr_PropertyNotSupported;
			//return setPropertyT(inPropertyDataSize, &m_destinationFormat, inPropertyData);
		}
		default:
		{
			STUB();
			return kAudioConverterErr_PropertyNotSupported;
		}
	}
	
	return unimpErr;
}

OSStatus AudioConverter::getPropertyInfo(AudioConverterPropertyID inPropertyID, UInt32 *outSize, Boolean *outWritable)
{
	STUB();
	return unimpErr;
}

template <typename T> OSStatus getPropertyT(UInt32 *ioPropertyDataSize, const T* localProperty, void* propertyTarget)
{
	T* t = static_cast<T*>(propertyTarget);
	
	if (*ioPropertyDataSize < sizeof(T))
		return kAudioConverterErr_BadPropertySizeError;
	*ioPropertyDataSize = sizeof(T);
	
	*t = *localProperty;
	return noErr;
}

OSStatus AudioConverter::getProperty(AudioConverterPropertyID inPropertyID, UInt32 *ioPropertyDataSize, void *outPropertyData)
{
	switch (inPropertyID)
	{
		case kAudioConverterEncodeBitRate:
		{
			return getPropertyT(ioPropertyDataSize, &m_outBitRate, outPropertyData);
		}
		case kAudioConverterCurrentInputStreamDescription:
		{
			return getPropertyT(ioPropertyDataSize, &m_sourceFormat, outPropertyData);
		}
		case kAudioConverterCurrentOutputStreamDescription:
		{
			return getPropertyT(ioPropertyDataSize, &m_destinationFormat, outPropertyData);
		}
		default:
		{
			STUB();
			return kAudioConverterErr_PropertyNotSupported;
		}
	}
}

OSStatus AudioConverter::feedInput(AudioConverterComplexInputDataProc dataProc, void* opaque, UInt32& numDataPackets)
{
	AudioBufferList bufferList;
	AudioStreamPacketDescription* aspd;
	OSStatus err;

	numDataPackets = 4096; // TODO: increase this?
	bufferList.mNumberBuffers = 1;
	bufferList.mBuffers[0].mDataByteSize = 0;
	bufferList.mBuffers[0].mData = nullptr;
	
	err = dataProc(this, &numDataPackets, &bufferList, &aspd, opaque);
	
	if (err != noErr)
		return err;
	
	m_avpkt.size = bufferList.mBuffers[0].mDataByteSize;
	m_avpkt.data = (uint8_t*) bufferList.mBuffers[0].mData;
	
	LOG << "dataProc() returned " << m_avpkt.size << " bytes of data\n";
	
	return noErr;
}

void AudioConverter::setupResampler(const AVFrame* frame)
{
	int err;
	
	if (m_resampler != nullptr)
		throw std::logic_error("Resampler already created");
	
	m_resampler = avresample_alloc_context();
	
	av_opt_set_int(m_resampler, "in_channel_layout", CAChannelCountToLayout(m_sourceFormat.mChannelsPerFrame), 0);
	av_opt_set_int(m_resampler, "out_channel_layout", CAChannelCountToLayout(m_destinationFormat.mChannelsPerFrame), 0);
	av_opt_set_int(m_resampler, "in_channels", m_sourceFormat.mChannelsPerFrame, 0);
	av_opt_set_int(m_resampler, "out_channels", m_destinationFormat.mChannelsPerFrame, 0);
	av_opt_set_int(m_resampler, "in_sample_rate", frame->sample_rate, 0);
	av_opt_set_int(m_resampler, "out_sample_rate", m_destinationFormat.mSampleRate, 0);
	av_opt_set_int(m_resampler, "in_sample_fmt", frame->format, 0);
	av_opt_set_int(m_resampler, "out_sample_fmt", CACodecSampleFormat(&m_destinationFormat), 0);
	
	err = avresample_open(m_resampler);
	if (err < 0)
		throw std::runtime_error("avresample_open() failed");
}

OSStatus AudioConverter::fillComplex(AudioConverterComplexInputDataProc dataProc, void* opaque, UInt32* ioOutputDataPacketSize, AudioBufferList *outOutputData, AudioStreamPacketDescription* outPacketDescription)
{
	AVFrame* srcaudio;

	srcaudio = av_frame_alloc();
	av_frame_unref(srcaudio);
	
	try
	{
		for (uint32_t i = 0; i < outOutputData->mNumberBuffers; i++)
		{
			UInt32 origSize = outOutputData->mBuffers[i].mDataByteSize;
			UInt32& newSize = outOutputData->mBuffers[i].mDataByteSize;
			
			newSize = 0;
			
			while (newSize < origSize)
			{
				int gotFrame = 0, err;
				uint8_t* resampledData;
				int outSamples;
				
				// Read input
				if (!m_avpkt.size)
				{
					UInt32 numDataPackets = 0;
					OSStatus err = feedInput(dataProc, opaque, numDataPackets);
					
					if (err != noErr)
						throw err;
					
					if (!m_avpkt.size) // numDataPackets cannot be trusted
						goto end;
				}
				
				err = avcodec_decode_audio4(m_decoder, srcaudio, &gotFrame, &m_avpkt);
				if (err < 0)
					throw std::runtime_error("avcodec_decode_audio4() failed");
				
				m_avpkt.size -= err;
				m_avpkt.data += err;
				
				if (!gotFrame)
					continue;
				if (!m_resampler)
					setupResampler(srcaudio);
					
				// Resample PCM
				err = avresample_convert(m_resampler, nullptr, 0, 0, srcaudio->data, 0, srcaudio->nb_samples);
				if (err < 0)
					throw std::runtime_error("avresample_convert() failed");
				
				while (avresample_available(m_resampler) > 0 && newSize < origSize)
				{
					AVPacket pkt;
					uint8_t *output;
					int out_linesize;
					int avail = avresample_available(m_resampler);
					
					if (!m_encoderInitialized)
						initEncoder();
					
					av_samples_alloc(&output, &out_linesize, m_destinationFormat.mChannelsPerFrame, avail, m_encoder->sample_fmt, 0);
					
					if (avresample_read(m_resampler, &output, avail) != avail)
						throw std::runtime_error("avresample_read() failed");
					
					av_init_packet(&pkt);
					pkt.data = 0;
					pkt.size = 0;
					m_audioFrame->nb_samples = avail;
					
					LOG << "Got " << avail << " samples\n";
					err = avcodec_fill_audio_frame(m_audioFrame, m_encoder->channels, m_encoder->sample_fmt, output, out_linesize, 0);
					if (err < 0)
						throw std::runtime_error("avcodec_fill_audio_frame() failed");
					
					// Encode PCM data
					err = avcodec_encode_audio2(m_encoder, &pkt, m_audioFrame, &gotFrame);
					if (err < 0)
						throw std::runtime_error("avcodec_encode_audio2() failed");
					
					if (!gotFrame)
						continue;
					
					// Output data
					int tocopy = std::min<int>(pkt.size, origSize - newSize);
					memcpy(((char*) outOutputData->mBuffers[i].mData) + newSize, pkt.data, tocopy);
					newSize += tocopy;
					
					LOG << "Filled buffer " << i << " with " << tocopy << " bytes\n";
					
					av_free_packet(&pkt);
					av_freep(&output);
				}
			}
		}
end:
		
		av_frame_free(&srcaudio);
	}
	catch (const std::exception& e)
	{
		ERROR() << "Exception: " << e.what();
		av_frame_free(&srcaudio);
	}
	catch (OSStatus err)
	{
		ERROR() << "OSStatus error: " << err;
		av_frame_free(&srcaudio);
		return err;
	}
	
	return noErr;
}

uint32_t AudioConverter::CAChannelCountToLayout(UInt32 numChannels)
{
	// TODO: this is just wild guessing
	switch (numChannels)
	{
		case 1:
			return AV_CH_LAYOUT_MONO;
		case 2:
			return AV_CH_LAYOUT_STEREO;
		case 3:
			return AV_CH_LAYOUT_SURROUND;
		case 4:
			return AV_CH_LAYOUT_4POINT0;
		case 5:
			return AV_CH_LAYOUT_4POINT1;
		case 6:
			return AV_CH_LAYOUT_5POINT1;
		case 7:
			return AV_CH_LAYOUT_6POINT1;
		case 8:
			return AV_CH_LAYOUT_7POINT1;
		default:
			return AV_CH_LAYOUT_STEREO;
	}
}

enum AVSampleFormat AudioConverter::CACodecSampleFormat(const AudioStreamBasicDescription* desc)
{
	if (desc->mFormatFlags & kAudioFormatFlagIsFloat)
	{
		if (desc->mBitsPerChannel == 32)
			return AV_SAMPLE_FMT_FLT;
		else if (desc->mBitsPerChannel == 64)
			return AV_SAMPLE_FMT_DBL;
		else
			return AV_SAMPLE_FMT_NONE;
	}
	else
	{
		
		switch (desc->mBitsPerChannel)
		{
			case 8: return AV_SAMPLE_FMT_U8;
			case 16: return AV_SAMPLE_FMT_S16;
			case 24: return AV_SAMPLE_FMT_S32; // FIXME: 24-bits?
			case 32: return AV_SAMPLE_FMT_S32;
			default: return AV_SAMPLE_FMT_NONE;
		}
	}
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
					cid = (enum AVCodecID)(int(cid)+1);
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
					cid = (enum AVCodecID)(int(cid)+1);
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

