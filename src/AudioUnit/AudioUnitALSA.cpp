#include "config.h"
#include "AudioUnitALSA.h"
#include "AudioUnitProperties.h"
#include <CoreServices/MacErrors.h>
#include <util/debug.h>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <cstdio>
#include <dispatch/dispatch.h>

#define SAMPLE_PERIOD	4096

static dispatch_queue_t g_audioQueue;

AudioUnitALSA::AudioUnitALSA(int cardIndex, char* cardName)
: m_cardIndex(cardIndex), m_cardName(cardName), m_pcmOutput(nullptr), m_pcmInput(nullptr)
{
	static dispatch_once_t pred;
	dispatch_once(&pred, ^{
		g_audioQueue = dispatch_queue_create("org.darlinghw.audiounit", nullptr);
	});
}

static void throwAlsaError(const std::string& msg, int errCode)
{
	std::stringstream ss;
	ss << msg << ": " << snd_strerror(errCode);
	throw std::runtime_error(ss.str());
}

static snd_pcm_format_t alsaFormatForASBD(const AudioStreamBasicDescription& asbd)
{
	bool isFloat = asbd.mFormatFlags & kAudioFormatFlagIsFloat;
	bool isSigned = asbd.mFormatFlags & kAudioFormatFlagIsSignedInteger;
	bool isBE = asbd.mFormatFlags & kAudioFormatFlagIsBigEndian;
	
	if (asbd.mFormatID == kAudioFormatLinearPCM)
	{
		if (isFloat)
			return isBE ? SND_PCM_FORMAT_FLOAT_BE : SND_PCM_FORMAT_FLOAT_LE;
		
		switch (asbd.mBitsPerChannel)
		{
			case 8:
				if (isSigned)
					return SND_PCM_FORMAT_S8;
				else
					return SND_PCM_FORMAT_U8;
			case 16:
				if (isSigned)
					return isBE ? SND_PCM_FORMAT_S16_BE : SND_PCM_FORMAT_S16_LE;
				else
					return isBE ? SND_PCM_FORMAT_U16_BE : SND_PCM_FORMAT_U16_LE;
			case 24:
				if (isSigned)
					return isBE ? SND_PCM_FORMAT_S24_BE : SND_PCM_FORMAT_S24_LE;
				else
					return isBE ? SND_PCM_FORMAT_U24_BE : SND_PCM_FORMAT_U24_LE;
			case 32:
				if (isSigned)
					return isBE ? SND_PCM_FORMAT_S32_BE : SND_PCM_FORMAT_S32_LE;
				else
					return isBE ? SND_PCM_FORMAT_U32_BE : SND_PCM_FORMAT_U32_LE;
			default:
				throw std::runtime_error("Invalid mBitsPerChannel value");
		}
	}
	else
		throw std::runtime_error("Unsupported mFormatID value");
}

AudioUnitComponent* AudioUnitALSA::create(int cardIndex)
{
	char* name;
	
	if (cardIndex > 0)
	{
		if (snd_card_get_name(cardIndex, &name))
			return nullptr;
	}
	else
	{
		name = strdup("default");
	}
	
	return new AudioUnitALSA(cardIndex, name);
}

AudioUnitALSA::~AudioUnitALSA()
{
	deinit();
	free(m_cardName);
}

OSStatus AudioUnitALSA::reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	STUB();
	return unimpErr;
}

void AudioUnitALSA::initOutput()
{
	snd_pcm_hw_params_t* hw_params = nullptr;
	snd_pcm_sw_params_t* sw_params = nullptr;
	
	try
	{
		// Configure ALSA based on m_configOutputPlayback
		int err;
		const AudioStreamBasicDescription& alsaConfig = m_config[kOutputBus].second;
		unsigned int rate = alsaConfig.mSampleRate;
		
		//if (memcmp(&m_config[kOutputBus].second, &m_config[kOutputBus].first, sizeof(AudioStreamBasicDescription)) != 0)
		//	throw std::runtime_error("Different input and HW config not implemented yet");
		
		// Let ALSA do the conversion on its own
		m_config[kOutputBus].second = m_config[kOutputBus].first;
		
		err = snd_pcm_open(&m_pcmOutput, m_cardName, SND_PCM_STREAM_PLAYBACK, 0);
		if (err)
			throwAlsaError("Failed to initialize playback PCM", err);
		
		err = snd_pcm_hw_params_malloc(&hw_params);
		if (err)
			throwAlsaError("Failed to alloc hw params", err);
		
		err = snd_pcm_hw_params_any(m_pcmOutput, hw_params);
		if (err)
			throwAlsaError("Failed to init hw params", err);
		
		if (isOutputPlanar())
			err = snd_pcm_hw_params_set_access(m_pcmOutput, hw_params, SND_PCM_ACCESS_RW_NONINTERLEAVED);
		else
			err = snd_pcm_hw_params_set_access(m_pcmOutput, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
		
		if (err)
			throwAlsaError("Failed to set interleaved access", err);
		
		err = snd_pcm_hw_params_set_format(m_pcmOutput, hw_params, alsaFormatForASBD(alsaConfig));
		if (err)
			throwAlsaError("Failed to set format", err);
		
		err = snd_pcm_hw_params_set_rate_near(m_pcmOutput, hw_params, &rate, 0);
		if (err)
			throwAlsaError("Failed to set sample rate", err);
		
		LOG << "Channel count: " << alsaConfig.mChannelsPerFrame << std::endl;
		err = snd_pcm_hw_params_set_channels(m_pcmOutput, hw_params, alsaConfig.mChannelsPerFrame);
		if (err)
			throwAlsaError("Failed to set channel count", err);
		
		err = snd_pcm_hw_params(m_pcmOutput, hw_params);
		if (err)
			throwAlsaError("Failed to set HW parameters", err);
		
		snd_pcm_hw_params_free(hw_params);
		hw_params = nullptr;
		
		err = snd_pcm_sw_params_malloc(&sw_params);
		if (err)
			throwAlsaError("Failed to alloc sw params", err);
		
		err = snd_pcm_sw_params_current(m_pcmOutput, sw_params);
		if (err)
			throwAlsaError("Failed to init sw params", err);
		
		err = snd_pcm_sw_params_set_avail_min(m_pcmOutput, sw_params, SAMPLE_PERIOD);
		if (err)
			throwAlsaError("snd_pcm_sw_params_set_avail_min() failed", err);
		
		err = snd_pcm_sw_params_set_start_threshold(m_pcmOutput, sw_params, 0U);
		if (err)
			throwAlsaError("snd_pcm_sw_params_set_start_threshold() failed", err);
		
		err = snd_pcm_sw_params_set_tstamp_mode(m_pcmOutput, sw_params, SND_PCM_TSTAMP_ENABLE);
		if (err)
			throwAlsaError("snd_pcm_sw_params_set_tstamp_mode() failed", err);
		
		err = snd_pcm_sw_params(m_pcmOutput, sw_params);
		if (err)
			throwAlsaError("Failed to set SW parameters", err);
		
		snd_pcm_sw_params_free(sw_params);
		sw_params = nullptr;
	}
	catch (...)
	{
		if (hw_params != nullptr)
			snd_pcm_hw_params_free(hw_params);
		if (sw_params != nullptr)
			snd_pcm_sw_params_free(sw_params);
		
		throw;
	}
}

void AudioUnitALSA::initInput()
{
	STUB();
	
	try
	{
		//if (memcmp(&m_config[kInputBus].second, &m_config[kInputBus].first, sizeof(AudioStreamBasicDescription)) != 0)
		//	throw std::runtime_error("Different input and HW config not implemented yet");
		
		// Let ALSA do the conversion on its own
		m_config[kInputBus].first = m_config[kInputBus].second;
	}
	catch (...)
	{
		throw;
	}
}

OSStatus AudioUnitALSA::init()
{
	try
	{
		if (m_pcmOutput || m_pcmInput)
			return kAudioUnitErr_Initialized;

		/*if (m_enableInput && snd_pcm_open(&m_pcmInput, m_cardName, SND_PCM_STREAM_CAPTURE, 0))
		{
			deinit();

			LOG << "Failed to initialize capture PCM " << m_cardName << std::endl;
			return kAudioUnitErr_FailedInitialization;
		}*/
		
		if (m_enableOutput)
			initOutput();
		if (m_enableInput)
			initInput();
	}
	catch (const std::exception& e)
	{
		ERROR() << e.what();
		
		deinit();
		return kAudioUnitErr_FailedInitialization;
	}
	
	return noErr;
}

OSStatus AudioUnitALSA::deinit()
{
	if (m_pcmOutput)
	{
		snd_pcm_close(m_pcmOutput);
		m_pcmOutput = nullptr;
	}
	if (m_pcmInput)
	{
		snd_pcm_close(m_pcmInput);
		m_pcmInput = nullptr;
	}
	return noErr;
}

void AudioUnitALSA::processAudioEvent(struct pollfd origPoll, int event)
{
	struct pollfd pfd = origPoll;
	unsigned short revents;
	int err;
	
	TRACE1(event);

	pfd.revents = event;

	err = snd_pcm_poll_descriptors_revents(m_pcmOutput, &pfd, 1, &revents);
	if (err != 0)
		ERROR() << "snd_pcm_poll_descriptors_revents() failed: " << snd_strerror(err);
	
	if (revents & POLLIN)
		pushDataFromInput();
	
	if (revents & POLLOUT)
		requestDataForPlayback();
}

void AudioUnitALSA::requestDataForPlayback()
{
	AudioUnitRenderActionFlags flags;
	AudioTimeStamp ts;
	AudioBufferList* bufs;
	OSStatus err;
	std::unique_ptr<uint8_t[]> data;
	snd_pcm_uframes_t availFrames;
	snd_htimestamp_t alsaTimestamp;
	const AudioStreamBasicDescription& config = m_config[kOutputBus].first;
	UInt32 cc = config.mChannelsPerFrame;
	
	TRACE();
	
	memset(&ts, 0, sizeof(ts));
	
	if (isOutputPlanar())
	{
		bufs = (AudioBufferList*) operator new(sizeof(AudioBufferList) + (cc-1)*sizeof(AudioBuffer));
		bufs->mNumberBuffers = cc;
	}
	else
	{
		bufs = (AudioBufferList*) operator new(sizeof(AudioBufferList));
		bufs->mNumberBuffers = 1;
	}
	
	if (m_shouldAllocateBuffer)
	{
		if (isOutputPlanar())
		{
			UInt32 bytesPerChannel = config.mBytesPerFrame * SAMPLE_PERIOD;
			
			data.reset(new uint8_t[cc*bytesPerChannel]);
			
			for (UInt32 i = 0; i < cc; i++)
			{
				bufs->mBuffers[i].mNumberChannels = 1;
				bufs->mBuffers[i].mDataByteSize = bytesPerChannel;
				bufs->mBuffers[i].mData = data.get() + i*bytesPerChannel;
			}
		}
		else
		{
			bufs->mBuffers[0].mNumberChannels = cc;
			bufs->mBuffers[0].mDataByteSize = config.mBytesPerFrame * SAMPLE_PERIOD;
		
			data.reset(new uint8_t[bufs->mBuffers[0].mDataByteSize]);
			bufs->mBuffers[0].mData = data.get();
		}
	}
	else
	{
		if (isOutputPlanar())
		{
			for (UInt32 i = 0; i < cc; i++)
			{
				bufs->mBuffers[i].mDataByteSize = 0;
				bufs->mBuffers[i].mData = nullptr;
			}
		}
		else
		{
			bufs->mBuffers[0].mDataByteSize = 0;
			bufs->mBuffers[0].mData = nullptr;
		}
	}
	
	snd_pcm_htimestamp(m_pcmOutput, &availFrames, &alsaTimestamp);
	
	// TODO: fill in AudioTimeStamp based on alsaTimestamp
	
	err = AudioUnitRender(m_inputUnit.sourceAudioUnit, &flags, &ts, kOutputBus, SAMPLE_PERIOD, bufs);
	
	if (err != noErr)
	{
		ERROR() << "Render callback failed with error " << err;
		
		// Fill with silence, the error may be temporary
		UInt32 bytes = config.mBytesPerFrame * SAMPLE_PERIOD;
		
		if (!m_shouldAllocateBuffer)
		{
			if (isOutputPlanar())
			{
				data.reset(new uint8_t[bytes*cc]);
				memset(data.get(), 0, bytes*cc);
			}
			else
			{
				data.reset(new uint8_t[bytes]);
				memset(data.get(), 0, bytes);
			}
		}
		
		if (isOutputPlanar())
		{
			for (UInt32 i = 0; i < cc; i++)
			{
				bufs->mBuffers[i].mData = data.get() + bytes*i;
				bufs->mBuffers[i].mDataByteSize = bytes;
			}
		}
		else
		{
			bufs->mBuffers[0].mData = data.get();
			bufs->mBuffers[0].mDataByteSize = bytes;
		}
	}
	
	m_lastRenderError = render(&flags, &ts, kOutputBus, SAMPLE_PERIOD, bufs);
	
	operator delete(bufs);
}

void AudioUnitALSA::pushDataFromInput()
{
	AudioUnitRenderActionFlags flags;
	AudioTimeStamp ts;
	AudioBufferList* bufs;
	OSStatus err;
	std::unique_ptr<uint8_t[]> data;
	snd_pcm_uframes_t availFrames;
	snd_htimestamp_t alsaTimestamp;
	const AudioStreamBasicDescription& config = m_config[kInputBus].second;
	
	TRACE();
	
	memset(&ts, 0, sizeof(ts));
	
	if (m_shouldAllocateBuffer)
	{
		if (isInputPlanar())
		{
			UInt32 cc = config.mChannelsPerFrame;
			bufs = (AudioBufferList*) operator new(sizeof(AudioBufferList) + (cc-1)*sizeof(AudioBuffer));
			bufs->mNumberBuffers = cc;
		}
		else
		{
			bufs = (AudioBufferList*) operator new(sizeof(AudioBufferList));
			bufs->mNumberBuffers = 1;
		}
		
		if (isInputPlanar())
		{
			UInt32 cc = config.mChannelsPerFrame;
			UInt32 bytesPerChannel = config.mBytesPerFrame * SAMPLE_PERIOD;
			
			data.reset(new uint8_t[cc*bytesPerChannel]);
			
			for (UInt32 i = 0; i < cc; i++)
			{
				bufs->mBuffers[i].mNumberChannels = 1;
				bufs->mBuffers[i].mDataByteSize = bytesPerChannel;
				bufs->mBuffers[i].mData = data.get() + i*bytesPerChannel;
			}
		}
		else
		{
			bufs->mBuffers[0].mNumberChannels = config.mChannelsPerFrame;
			bufs->mBuffers[0].mDataByteSize = config.mBytesPerFrame * SAMPLE_PERIOD;
	
			data.reset(new uint8_t[bufs->mBuffers[0].mDataByteSize]);
			bufs->mBuffers[0].mData = data.get();
		}
		
		//render(&flags, &ts, kInputBus, SAMPLE_PERIOD, bufs);
	}
	else
		bufs = nullptr;
	
	// TODO: fill in AudioTimeStamp based on alsaTimestamp
	snd_pcm_htimestamp(m_pcmOutput, &availFrames, &alsaTimestamp);
	
	m_outputCallback.inputProc(m_outputCallback.inputProcRefCon, &flags, &ts, kInputBus, SAMPLE_PERIOD, bufs);
	
	operator delete(bufs);
}

OSStatus AudioUnitALSA::render(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	if (inBusNumber == kOutputBus)
		return renderOutput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
	else if (inBusNumber == kInputBus)
		return renderInput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
	else
		return paramErr;
}

OSStatus AudioUnitALSA::renderOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	if (!isOutputPlanar())
		return renderInterleavedOutput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
	else
		return renderPlanarOutput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
}

OSStatus AudioUnitALSA::renderInterleavedOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	int wr, sampleCount;
	const AudioStreamBasicDescription& config = m_config[kOutputBus].first;
	
	for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
	{
		LOG << "Writing " << ioData->mBuffers[i].mDataByteSize << " bytes into sound card\n";
		
		sampleCount = ioData->mBuffers[i].mDataByteSize / config.mBytesPerFrame;

do_write:
		wr = snd_pcm_writei(m_pcmOutput, ioData->mBuffers[i].mData, sampleCount);
		if (wr < 0)
		{
			if (wr == -EINTR || wr == -EPIPE)
			{
				snd_pcm_recover(m_pcmOutput, wr, false);
				goto do_write;
			}
			else
			{
				ERROR() << "snd_pcm_writei() failed: " << snd_strerror(wr);
				return kAudioUnitErr_NoConnection;
			}
		}
		else if (wr < sampleCount)
			ERROR() << "snd_pcm_writei(): not all data written?";
	}
	
	return noErr;
}

OSStatus AudioUnitALSA::renderPlanarOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	std::unique_ptr<void*[]> bufferPointers (new void*[ioData->mNumberBuffers]);
	int size, sampleCount, wr;
	const AudioStreamBasicDescription& config = m_config[kOutputBus].first;
	
	if (ioData->mNumberBuffers != config.mChannelsPerFrame)
	{
		ERROR() << "Incorrect buffer count for planar audio, only " << ioData->mNumberBuffers;
		return paramErr;
	}
	
	size = ioData->mBuffers[0].mDataByteSize;
	for (UInt32 i = 1; i < ioData->mNumberBuffers; i++)
	{
		if (size != ioData->mBuffers[i].mDataByteSize)
		{
			ERROR() << "Bad buffer size in buffer " << i;
			return paramErr;
		}
	}
	
	for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
		bufferPointers[i] = ioData->mBuffers[i].mData;
	
	sampleCount = size / config.mBytesPerFrame;
	
do_write:
	wr = snd_pcm_writen(m_pcmOutput, bufferPointers.get(), sampleCount);
	if (wr < 0)
	{
		if (wr == -EINTR || wr == -EPIPE)
		{
			snd_pcm_recover(m_pcmOutput, wr, false);
			goto do_write;
		}
		else
		{
			ERROR() << "snd_pcm_writen() failed: " << snd_strerror(wr);
			return kAudioUnitErr_NoConnection;
		}
	}
	else if (wr < sampleCount)
		ERROR() << "snd_pcm_writen(): not all data written?";
	
	return noErr;
}

OSStatus AudioUnitALSA::renderInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	if (!m_outputCallback.inputProc)
		return noErr; // We don't push, we should be polled
	
	if (!isInputPlanar())
		return renderInterleavedInput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
	else
		return renderPlanarInput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
}

OSStatus AudioUnitALSA::renderInterleavedInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	int wr, sampleCount;
	const AudioStreamBasicDescription& config = m_config[kInputBus].second;
	
	for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
	{
		LOG << "Reading up to " << ioData->mBuffers[i].mDataByteSize << " bytes from sound card\n";
		
		sampleCount = ioData->mBuffers[i].mDataByteSize / config.mBytesPerFrame;

do_write:
		wr = snd_pcm_writei(m_pcmOutput, ioData->mBuffers[i].mData, sampleCount);
		if (wr < 0)
		{
			if (wr == -EINTR || wr == -EPIPE)
			{
				snd_pcm_recover(m_pcmOutput, wr, false);
				goto do_write;
			}
			else
			{
				ERROR() << "snd_pcm_writei() failed: " << snd_strerror(wr);
				return kAudioUnitErr_NoConnection;
			}
		}
		
		ioData->mBuffers[i].mDataByteSize = wr * config.mBytesPerFrame;
	}
	
	return noErr;
}

OSStatus AudioUnitALSA::renderPlanarInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	std::unique_ptr<void*[]> bufferPointers (new void*[ioData->mNumberBuffers]);
	int size, sampleCount, wr;
	const AudioStreamBasicDescription& config = m_config[kInputBus].second;
	
	if (ioData->mNumberBuffers != config.mChannelsPerFrame)
	{
		ERROR() << "Incorrect buffer count for planar audio, only " << ioData->mNumberBuffers;
		return paramErr;
	}
	
	size = ioData->mBuffers[0].mDataByteSize;
	for (UInt32 i = 1; i < ioData->mNumberBuffers; i++)
	{
		if (size != ioData->mBuffers[i].mDataByteSize)
		{
			ERROR() << "Bad buffer size in buffer " << i;
			return paramErr;
		}
	}
	
	for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
		bufferPointers[i] = ioData->mBuffers[i].mData;
	
	sampleCount = size / config.mBytesPerFrame;
	
do_write:
	wr = snd_pcm_readn(m_pcmInput, bufferPointers.get(), sampleCount);
	if (wr < 0)
	{
		if (wr == -EINTR || wr == -EPIPE)
		{
			snd_pcm_recover(m_pcmInput, wr, false);
			goto do_write;
		}
		else
		{
			ERROR() << "snd_pcm_writen() failed: " << snd_strerror(wr);
			return kAudioUnitErr_NoConnection;
		}
	}
	
	for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
		ioData->mBuffers[i].mDataByteSize = sampleCount * config.mBytesPerFrame;
	
	return noErr;
}

void AudioUnitALSA::startOutput()
{
	std::unique_ptr<struct pollfd[]> pollfds;
	int count, err;
	
	if (!m_inputUnit.sourceAudioUnit)
		throw std::runtime_error("No input unit set");
	
	err = snd_pcm_prepare(m_pcmOutput);
	if (err != 0)
		throwAlsaError("snd_pcm_prepare() failed", err);
	
	count = snd_pcm_poll_descriptors_count(m_pcmOutput);
	pollfds.reset(new struct pollfd[count]);
	
	if (snd_pcm_poll_descriptors(m_pcmOutput, pollfds.get(), count) != count)
		throw std::runtime_error("snd_pcm_poll_descriptors() failed");
	
	LOG << "ALSA descriptor count: " << count << std::endl;
	startDescriptors(pollfds.get(), count);
}

void AudioUnitALSA::startDescriptors(const struct pollfd* pollfds, int count)
{
	for (int i = 0; i < count; i++)
	{
		dispatch_source_t source;
		const struct pollfd& cur = pollfds[i];
		
		if (cur.events & POLLIN)
		{
			source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, cur.fd, 0, g_audioQueue);
			dispatch_source_set_event_handler(source, ^{
				processAudioEvent(cur, POLLIN);
			});
			dispatch_resume(source);
			m_sources.push_back(source);
		}
		
		if (cur.events & POLLOUT)
		{
			source = dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE, cur.fd, 0, g_audioQueue);
			dispatch_source_set_event_handler(source, ^{
				processAudioEvent(cur, POLLOUT);
			});
			dispatch_resume(source);
			m_sources.push_back(source);
		}
		
	}
}

void AudioUnitALSA::startInput()
{
	int err;
	
	STUB();
	
	err = snd_pcm_prepare(m_pcmInput);
	if (err != 0)
		throwAlsaError("snd_pcm_prepare() failed", err);
}

OSStatus AudioUnitALSA::start()
{
	int err;
	
	TRACE();
	
	try
	{
		if (m_pcmInput)
			startInput();

		if (m_pcmOutput)
			startOutput();

	}
	catch (const std::exception& e)
	{
		if (m_pcmInput)
			snd_pcm_drop(m_pcmInput);
		if (m_pcmOutput)
			snd_pcm_drop(m_pcmOutput);
		
		ERROR() << e.what();
		return kAudioUnitErr_FailedInitialization;
	}
	
	return noErr;
}

OSStatus AudioUnitALSA::stop()
{
	TRACE();
	
	for (dispatch_source_t src : m_sources)
		dispatch_source_cancel(src);
	m_sources.clear();
	
	if (m_pcmInput)
		snd_pcm_drop(m_pcmInput);
	if (m_pcmOutput)
		snd_pcm_drop(m_pcmOutput);
	return noErr;
}

