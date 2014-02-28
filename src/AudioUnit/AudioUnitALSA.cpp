#include "config.h"
#include "AudioUnitALSA.h"
#include "AudioUnitProperties.h"
#include <CoreServices/MacErrors.h>
#include <util/debug.h>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <dispatch/dispatch.h>

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

OSStatus AudioUnitALSA::init()
{
	snd_pcm_hw_params_t* hw_params = nullptr;
	snd_pcm_sw_params_t* sw_params = nullptr;
	
	try
	{
		if (m_pcmOutput || m_pcmInput)
			return kAudioUnitErr_Initialized;
		
		//if (!m_renderCallback.inputProc)
		//	throw std::runtime_error("No render callback set");
		
		if (m_configOutputPlayback.mFormatFlags & kAudioFormatFlagIsNonInterleaved)
		{
			ERROR() << "Non-interleaved audio not supported yet\n";
			return kAudioUnitErr_FailedInitialization;
		}


		if (m_enableInput && snd_pcm_open(&m_pcmInput, m_cardName, SND_PCM_STREAM_CAPTURE, 0))
		{
			deinit();

			LOG << "Failed to initialize capture PCM " << m_cardName << std::endl;
			return kAudioUnitErr_FailedInitialization;
		}
		
		if (m_enableOutput)
		{
			// Configure ALSA based on m_configOutputPlayback
			int err;
			unsigned int rate = m_configOutputPlayback.mSampleRate;
			
			err = snd_pcm_open(&m_pcmOutput, m_cardName, SND_PCM_STREAM_PLAYBACK, 0);
			if (err)
				throwAlsaError("Failed to initialize playback PCM", err);
			
			err = snd_pcm_hw_params_malloc(&hw_params);
			if (err)
				throwAlsaError("Failed to alloc hw params", err);
			
			err = snd_pcm_hw_params_any(m_pcmOutput, hw_params);
			if (err)
				throwAlsaError("Failed to init hw params", err);
			
			// TODO: non-interleaved access
			err = snd_pcm_hw_params_set_access(m_pcmOutput, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
			if (err)
				throwAlsaError("Failed to set interleaved access", err);
			
			err = snd_pcm_hw_params_set_format(m_pcmOutput, hw_params, alsaFormatForASBD(m_configOutputPlayback));
			if (err)
				throwAlsaError("Failed to set format", err);
			
			err = snd_pcm_hw_params_set_rate_near(m_pcmOutput, hw_params, &rate, 0);
			if (err)
				throwAlsaError("Failed to set sample rate", err);
			
			err = snd_pcm_hw_params_set_channels(m_pcmOutput, hw_params, m_configOutputPlayback.mChannelsPerFrame);
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
			
			err = snd_pcm_sw_params_set_avail_min(m_pcmOutput, sw_params, 4096); // TODO: 4096?
			if (err)
				throwAlsaError("snd_pcm_sw_params_set_avail_min() failed", err);
			
			err = snd_pcm_sw_params_set_start_threshold(m_pcmOutput, sw_params, 0U);
			if (err)
				throwAlsaError("snd_pcm_sw_params_set_start_threshold() failed", err);
			
			err = snd_pcm_sw_params(m_pcmOutput, sw_params);
			if (err)
				throwAlsaError("Failed to set SW parameters", err);
			
			snd_pcm_sw_params_free(sw_params);
			sw_params = nullptr;
		}
	}
	catch (const std::exception& e)
	{
		ERROR() << e.what();
		
		if (hw_params != nullptr)
			snd_pcm_hw_params_free(hw_params);
		if (sw_params != nullptr)
			snd_pcm_sw_params_free(sw_params);
		
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

	pfd.revents = POLLIN;

	err = snd_pcm_poll_descriptors_revents(m_pcmOutput, &pfd, 1, &revents);
	if (err != 0)
		ERROR() << "snd_pcm_poll_descriptors_revents() failed: " << snd_strerror(err);
	
	if (revents == POLLIN)
	{
		ERROR() << "Audio capture unsupported";
	}
	else if (revents == POLLOUT)
	{
		requestDataForPlayback();
	}
}

void AudioUnitALSA::requestDataForPlayback()
{
	AudioUnitRenderActionFlags flags;
	AudioTimeStamp ts;
	AudioBufferList bufs;
	OSStatus err;
	std::unique_ptr<uint8_t[]> data;
	
	TRACE();
	
	memset(&ts, 0, sizeof(ts));
	
	// TODO: non-interleaved audio
	bufs.mNumberBuffers = 1;
	bufs.mBuffers[0].mNumberChannels = m_configOutputPlayback.mChannelsPerFrame;
	bufs.mBuffers[0].mDataByteSize = m_configOutputPlayback.mBytesPerFrame * 4096;
	
	data.reset(new uint8_t[bufs.mBuffers[0].mDataByteSize]);
	bufs.mBuffers[0].mData = data.get();
	
	err = m_renderCallback.inputProc(m_renderCallback.inputProcRefCon, &flags, &ts, 0 /* playback */, 4096, &bufs);
	if (err != noErr)
	{
		ERROR() << "Render callback failed with error " << err;
		stop();
	}
	else
	{
		int wr;
		
		// TODO: non-interleaved audio
		wr = snd_pcm_writei(m_pcmOutput, bufs.mBuffers[0].mData, bufs.mBuffers[0].mDataByteSize);
		if (wr != bufs.mBuffers[0].mDataByteSize)
		{
			if (wr < 0)
				ERROR() << "snd_pcm_writei() failed: " << snd_strerror(wr);
			else
				ERROR() << "snd_pcm_writei(): not all data written?";
		}
	}
}

OSStatus AudioUnitALSA::start()
{
	int err;
	
	TRACE();
	
	try
	{
		if (!m_renderCallback.inputProc)
			throw std::runtime_error("No render callback set");
	
		if (m_pcmInput)
		{
			err = snd_pcm_prepare(m_pcmInput);
			if (err != 0)
			{
				LOG << "snd_pcm_prepare(m_pcmInput) failed with error: " << snd_strerror(err) << std::endl;
				return kAudioUnitErr_FailedInitialization;
			}
		}

		if (m_pcmOutput)
		{
			std::unique_ptr<struct pollfd[]> pollfds;
			int count;
			
			err = snd_pcm_prepare(m_pcmOutput);
			if (err != 0)
				throwAlsaError("snd_pcm_prepare failed", err);
			
			count = snd_pcm_poll_descriptors_count(m_pcmOutput);
			pollfds.reset(new struct pollfd[count]);
			
			if (snd_pcm_poll_descriptors(m_pcmOutput, pollfds.get(), count) != count)
				throw std::runtime_error("snd_pcm_poll_descriptors() failed");
			
			LOG << "ALSA descriptor count: " << count << std::endl;
			
			for (int i = 0; i < count; i++)
			{
				dispatch_source_t source;
				struct pollfd cur = pollfds[i];
				
				if (cur.events & POLLIN)
				{
					source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, pollfds[i].fd, 0, g_audioQueue);
					dispatch_source_set_event_handler(source, ^{
						processAudioEvent(cur, POLLIN);
					});
					dispatch_resume(source);
					m_sources.push_back(source);
				}
				
				if (cur.events & POLLOUT)
				{
					source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, pollfds[i].fd, 0, g_audioQueue);
					dispatch_source_set_event_handler(source, ^{
						processAudioEvent(cur, POLLOUT);
					});
					dispatch_resume(source);
					m_sources.push_back(source);
				}
				
			}
		}

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
	for (dispatch_source_t src : m_sources)
		dispatch_source_cancel(src);
	m_sources.clear();
	
	if (m_pcmInput)
		snd_pcm_drop(m_pcmInput);
	if (m_pcmOutput)
		snd_pcm_drop(m_pcmOutput);
	return noErr;
}

