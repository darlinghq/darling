#include "AudioUnitPA.h"
#include <CoreServices/MacErrors.h>
#include <CoreFoundation/CFString.h>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <sstream>
#include <memory>
#include <cstring>
#include "stub.h"
#include <objc/runtime.h>

extern "C" char*** _NSGetArgv(void);

pa_mainloop* AudioUnitPA::m_mainloop = nullptr;
pa_context* AudioUnitPA::m_context = nullptr;
std::thread* AudioUnitPA::m_mainloopThread;

AudioUnitPA::AudioUnitPA()
{
}

AudioUnitPA::~AudioUnitPA()
{
	deinit();
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
		// ERROR() << "pa_stream_new() failed";
		return kAudioUnitErr_FailedInitialization;
	}
	
	// TODO: support recording
	pa_stream_set_state_callback(m_stream, paStreamStateCB, this);
	pa_stream_set_write_callback(m_stream, paStreamWriteCB, this);
	
	pa_stream_connect_playback(m_stream, nullptr, nullptr, PA_STREAM_START_CORKED /*(pa_stream_flags_t) 0*/,
			nullptr, nullptr);
	
	return noErr;
}

int AudioUnitPA::cardIndex() const
{
	return 0;
}

void AudioUnitPA::paStreamStateCB(pa_stream* s, void*)
{
	int state = pa_stream_get_state(s);
	//TRACE() << "state=" << state;
	
	switch (state)
	{
		case PA_STREAM_FAILED:
			//ERROR() << "PA stream error: " << pa_strerror(pa_context_errno(pa_stream_get_context(s)));
			break;
		case PA_STREAM_READY:
			//LOG << "PA stream is ready\n";
			break;
	}
} 

void AudioUnitPA::paStreamWriteCB(pa_stream* s, size_t length, void* self)
{
	AudioUnitPA* This = static_cast<AudioUnitPA*>(self);
	This->requestDataForPlayback(length);
}

void AudioUnitPA::requestDataForPlayback(size_t length)
{
	AudioUnitRenderActionFlags flags;
	AudioTimeStamp ts;
	AudioBufferList* bufs;
	OSStatus err;
	std::unique_ptr<uint8_t[]> data;
	const AudioStreamBasicDescription& config = m_config[kOutputBus].first;
	const UInt32 cc = config.mChannelsPerFrame;

	if (!m_stream)
	{
		// std::cerr << "No stream?!\n";
		return;
	}
	
	//TRACE() << "m_started=" << m_started;
	
	if (!m_started)
	{
		std::unique_ptr<char[]> empty;
		size_t len;
		int ret;
		
		len = config.mBytesPerFrame * (config.mSampleRate / 10);
		empty.reset(new char[len]);
		
		memset(empty.get(), 0, len);
		
		ret = pa_stream_write(m_stream, empty.get(), len,
				nullptr, 0, PA_SEEK_RELATIVE);
		
		// pa_stream_cork(m_stream, true, [](pa_stream*, int, void*) {}, nullptr);
		return;
	}
	
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
			UInt32 bytesPerChannel = length / cc;
			
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
			bufs->mBuffers[0].mDataByteSize = length;
		
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
	
	// snd_pcm_htimestamp(m_pcmOutput, &availFrames, &alsaTimestamp);
	
	// TODO: fill in AudioTimeStamp based on some PA timestamp
	
	err = AudioUnitRender(m_inputUnit.sourceAudioUnit, &flags, &ts, kOutputBus, length / config.mBytesPerFrame, bufs);
	
	if (err != noErr || bufs->mBuffers[0].mDataByteSize == 0)
	{
		// ERROR() << "Render callback failed with error " << err;
		
		// Fill with silence, the error may be temporary
		UInt32 bytes = length;
		
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
	
	//LOG << "Rendering...\n";
	m_lastRenderError = AudioUnitRender(this, &flags, &ts, kOutputBus, length / config.mBytesPerFrame, bufs);
	
	operator delete(bufs);
}

OSStatus AudioUnitPA::render(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	if (inBusNumber == kOutputBus)
		return renderOutput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
	else if (inBusNumber == kInputBus)
		return renderInput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
	else
		return paramErr;
}

OSStatus AudioUnitPA::renderOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	if (!isOutputPlanar())
		return renderInterleavedOutput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
	else
		return renderPlanarOutput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
}

OSStatus AudioUnitPA::renderInterleavedOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	const AudioStreamBasicDescription& config = m_config[kOutputBus].first;
	UInt32 framesSoFar = 0;
	
	for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
	{
		size_t bytes = std::min<size_t>((inNumberFrames - framesSoFar) * config.mBytesPerFrame, ioData->mBuffers[i].mDataByteSize);
		
		if (!bytes)
			break;
		
		//LOG << "AudioUnitPA::renderInterleavedOutput(): data=" << ioData->mBuffers[i].mData << ", bytes=" << bytes << std::endl;
		pa_stream_write(m_stream, ((char*) ioData->mBuffers[i].mData) + framesSoFar * config.mBytesPerFrame, bytes,
				nullptr, 0, PA_SEEK_RELATIVE);
		
		framesSoFar += ioData->mBuffers[i].mDataByteSize / config.mBytesPerFrame;
	}
	
	return noErr;
}

OSStatus AudioUnitPA::renderPlanarOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	// PulseAudio doesn't support planar audio, we have to perform conversion
	return unimpErr;
}

OSStatus AudioUnitPA::renderInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	if (!m_outputCallback.inputProc)
		return noErr; // We don't push, we should be polled
	
	if (!isInputPlanar())
		return renderInterleavedInput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
	else
		return renderPlanarInput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
}

OSStatus AudioUnitPA::renderInterleavedInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	STUB();
	return unimpErr;
}

OSStatus AudioUnitPA::renderPlanarInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	STUB();
	return unimpErr;
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

OSStatus AudioUnitPA::reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	STUB();
	return unimpErr;
}

OSStatus AudioUnitPA::deinit()
{
	TRACE();
	if (!m_stream)
		return kAudioUnitErr_Uninitialized;
	
	pa_stream_disconnect(m_stream);
    pa_stream_unref(m_stream);
    m_stream = nullptr;
	
	return noErr;
}

OSStatus AudioUnitPA::start()
{
	TRACE();
	if (!m_stream)
		return kAudioUnitErr_Uninitialized;
	
	if (pa_stream_is_corked(m_stream))
	{
		m_started = true;
		
		// const AudioStreamBasicDescription& config = m_config[kOutputBus].first;
		pa_stream_cork(m_stream, 0, [](pa_stream*, int, void*) {}, nullptr);
		// requestDataForPlayback(config.mSampleRate / 10 * config.mBytesPerFrame);
	}
	
	return noErr;
}

OSStatus AudioUnitPA::stop()
{
	TRACE();
	if (!m_stream)
		return kAudioUnitErr_Uninitialized;
	
	if (pa_stream_is_corked(m_stream))
		pa_stream_cork(m_stream, 1, [](pa_stream*, int, void*) {}, nullptr);
	
	m_started = false;
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
		
		// ERROR() << e.what();
	}
}

std::string AudioUnitPA::getAppName()
{
	Class classBundle;
	std::string name;
	size_t pos;
	
	classBundle = (Class) objc_getClass("NSBundle");
	
	if (classBundle != nullptr)
	{
		// TODO: get a nice app name
	}
	
	name = (*_NSGetArgv())[0];
	pos = name.rfind('/');
	
	if (pos != std::string::npos)
		name = name.substr(pos+1);
	
	return name;
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
	
	// TRACE() << pa_context_get_state(c);
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
