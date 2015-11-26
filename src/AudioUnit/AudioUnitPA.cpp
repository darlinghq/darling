#include "AudioUnitPA.h"
#include <CoreServices/MacErrors.h>
#include <CoreFoundation/CFString.h>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <sstream>
#include <memory>
#include <util/debug.h>

extern "C" char*** _NSGetArgv(void);

pa_mainloop* AudioUnitPA::m_mainloop = nullptr;
pa_context* AudioUnitPA::m_context = nullptr;
std::thread* AudioUnitPA::m_mainloopThread;

AudioUnitPA::AudioUnitPA()
{
}

AudioUnitPA::~AudioUnitPA()
{
}

static std::string CFStringToStdString(CFStringRef str)
{
	CFIndex length, maxSize;
	std::unique_ptr<char[]> buffer;
	
	length = CFStringGetLength(str);
	maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
	buffer.reset(new char[maxSize]);
	
	if (CFStringGetCString(str, buffer.get(), maxSize, kCFStringEncodingUTF8))
		return std::string(buffer.get());
	else
		return std::string();
}

OSStatus AudioUnitPA::init()
{
	static std::once_flag once;
	OSStatus ret;
	
	std::call_once(once, initializePA);
	if (m_context == nullptr)
		return kAudioUnitErr_FailedInitialization;
	
	if (m_enableOutput)
		ret = initOutput();
	
	// TODO: support recording
	return ret;
}

OSStatus AudioUnitPA::initOutput()
{
	std::string streamName;
	pa_sample_spec spec;
	
	if (m_contextName != nullptr)
		streamName = CFStringToStdString(m_contextName);
	else
		streamName = "AudioUnit";
	
	spec = paSampleSpecForASBD(m_config[kOutputBus].second);
	m_stream = pa_stream_new(m_context, streamName.c_str(), &spec, nullptr);
	
	if (m_stream == nullptr)
	{
		ERROR() << "pa_stream_new() failed";
		return kAudioUnitErr_FailedInitialization;
	}
	
	// TODO: support recording
	pa_stream_set_state_callback(m_stream, paStreamStateCB, this);
	pa_stream_set_write_callback(m_stream, paStreamWriteCB, this);
	
	pa_stream_connect_playback(m_stream, nullptr, nullptr, PA_STREAM_START_CORKED,
			nullptr, nullptr);
	
	return noErr;
}

void AudioUnitPA::paStreamStateCB(pa_stream* s, void*)
{
	
}

void AudioUnitPA::paStreamWriteCB(pa_stream* s, size_t length, void* self)
{
	AudioUnitPA* This = static_cast<AudioUnitPA*>(self);
	
	
}

pa_sample_spec AudioUnitPA::paSampleSpecForASBD(const AudioStreamBasicDescription& asbd)
{
	const bool isFloat = asbd.mFormatFlags & kAudioFormatFlagIsFloat;
	const bool isSigned = asbd.mFormatFlags & kAudioFormatFlagIsSignedInteger;
	const bool isBE = asbd.mFormatFlags & kAudioFormatFlagIsBigEndian;
	
	pa_sample_spec spec;
	
	spec.rate = asbd.mSampleRate;
	spec.channels = asbd.mChannelsPerFrame;
	
	if (asbd.mFormatID == kAudioFormatLinearPCM)
	{
		if (isFloat)
			spec.format = isBE ? PA_SAMPLE_FLOAT32BE : PA_SAMPLE_FLOAT32LE;
		else
		{
			switch (asbd.mBitsPerChannel)
			{
				case 8:
					m_convertUnsignedSigned = isSigned;
					spec.format = PA_SAMPLE_U8;
					break;
				case 16:
					m_convertUnsignedSigned = !isSigned;
					
					spec.format = isBE ? PA_SAMPLE_S16BE : PA_SAMPLE_S16LE;
					break;
				case 24:
					m_convertUnsignedSigned = !isSigned;
					
					spec.format = isBE ? PA_SAMPLE_S24BE : PA_SAMPLE_S24LE;
					break;
				case 32:
					m_convertUnsignedSigned = !isSigned;
					
					spec.format = isBE ? PA_SAMPLE_S32BE : PA_SAMPLE_S32LE;
					break;
			}
		}
	}
	else
		throw std::runtime_error("Unsupported mFormatID value");
	
	if (!pa_sample_spec_valid(&spec))
		throw std::logic_error("Invalid pa_sample_spec constructed");
	
	return spec;
}

OSStatus AudioUnitPA::deinit()
{
	if (!m_stream)
		return kAudioUnitErr_Uninitialized;
	
	pa_stream_disconnect(m_stream);
    pa_stream_unref(m_stream);
    m_stream = nullptr;
	
	return noErr;
}

OSStatus AudioUnitPA::start()
{
	if (!m_stream)
		return kAudioUnitErr_Uninitialized;
	
	if (pa_stream_is_corked(m_stream))
		pa_stream_cork(m_stream, false, [](pa_stream*, int, void*) {}, nullptr);
		
	return noErr;
}

OSStatus AudioUnitPA::stop()
{
	if (!m_stream)
		return kAudioUnitErr_Uninitialized;
	
	if (pa_stream_is_corked(m_stream))
		pa_stream_cork(m_stream, true, [](pa_stream*, int, void*) {}, nullptr);
	
	return noErr;
}

struct Completion
{
	std::condition_variable cond;
	std::mutex mutex;
	bool complete = false;
};

void AudioUnitPA::initializePA()
{
	try
	{
		pa_mainloop_api* mainloopApi;
		const char* appName;
		Completion comp;
		static int retval;
		
		if ((m_mainloop = pa_mainloop_new()) == nullptr)
			throw std::runtime_error("Failed to create PA mainloop");
		
		mainloopApi = pa_mainloop_get_api(m_mainloop);
		
		// TODO: Detect if this is an app in a bundle and get a nice app name
		appName = (*_NSGetArgv())[0];
		
		if ((m_context = pa_context_new(mainloopApi, appName)) == nullptr)
			throw std::runtime_error("Failed to create PA context");
		
		pa_context_set_state_callback(m_context, paContextStateCB, &comp);
		
		if (pa_context_connect(m_context, nullptr, pa_context_flags_t(0), nullptr) < 0)
		{
			std::stringstream ss;
			ss << "pa_context_connect() failed: " << pa_strerror(pa_context_errno(m_context));
			throw std::runtime_error(ss.str());
		}
		
		m_mainloopThread = new std::thread(pa_mainloop_run, m_mainloop, &retval);
		
		std::unique_lock<std::mutex> lk(comp.mutex);
		comp.cond.wait(lk, [&]{ return comp.complete; });
		
		if (m_context == nullptr)
			throw std::runtime_error("Failed to connect context");
	}
	catch (const std::exception& e)
	{
		pa_mainloop_free(m_mainloop);
		m_mainloop = nullptr;
		
		ERROR() << e.what();
	}
}

void AudioUnitPA::deinitializePA()
{
	if (m_context != nullptr)
	{
		pa_context_unref(m_context);
		m_context = nullptr;
	}
	if (m_mainloop != nullptr)
	{
		pa_mainloop_api* mainloopApi;
		
		mainloopApi = pa_mainloop_get_api(m_mainloop);
		mainloopApi->quit(mainloopApi, 0);
		
		m_mainloopThread->join();
		delete m_mainloopThread;
		
		pa_mainloop_free(m_mainloop);
	}
}

void AudioUnitPA::paContextStateCB(pa_context* c, void* priv)
{
	Completion* comp = static_cast<Completion*>(priv);
	
	switch (pa_context_get_state(c))
	{
		case PA_CONTEXT_READY:
		{
			std::unique_lock<std::mutex> lk(comp->mutex);
			comp->complete = true;
			comp->cond.notify_one();
			break;
		}
			
		case PA_CONTEXT_FAILED:
			// Indicate failure
			pa_context_unref(m_context);
			m_context = nullptr;
			
			comp->complete = true;
			comp->cond.notify_one();
			break;
		default:
			break;
	}
}
