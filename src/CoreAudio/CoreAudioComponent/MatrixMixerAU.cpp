/*
This file is part of Darling.

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "AUBase.h"
#include "AUInputElement.h"
#include "AUOutputElement.h"
#include "AUPlugInDispatch.h"
#include <AudioToolbox/AudioUnitParameters.h>
#include <map>
#include <mutex>
#include <cmath>
#include <cstring>

// Float PCM matrix mixing. Channel indices are flattened across each scope's buses.
class MatrixMixerAU : public AUBase {
    std::mutex mMutex;
    std::map<UInt32, Float32> mGains;
    std::map<UInt32, Float32> mInputEnabled, mOutputEnabled;
    UInt32 channels(AudioUnitScope scope) {
        auto& buses = GetScope(scope);
        UInt32 count = 0;
        for (UInt32 b = 0; b < buses.GetNumberOfElements(); ++b)
            count += buses.GetIOElement(b)->GetStreamFormat().mChannelsPerFrame;
        return count;
    }
    static Float32 value(const std::map<UInt32, Float32>& values, UInt32 key, Float32 fallback = 0) {
        auto it = values.find(key);
        return it == values.end() ? fallback : it->second;
    }
    bool validGain(UInt32 element) {
        UInt32 input = element >> 16, output = element & 0xffff;
        return (input == 0xffff || input < channels(kAudioUnitScope_Input)) &&
               (output == 0xffff || output < channels(kAudioUnitScope_Output));
    }
    static Float32* sample(AudioBufferList& buffers, UInt32 channel, UInt32& stride) {
        for (UInt32 b = 0; b < buffers.mNumberBuffers; ++b) {
            AudioBuffer& buffer = buffers.mBuffers[b];
            if (channel < buffer.mNumberChannels) {
                stride = buffer.mNumberChannels;
                return buffer.mData ? static_cast<Float32*>(buffer.mData) + channel : nullptr;
            }
            channel -= buffer.mNumberChannels;
        }
        return nullptr;
    }
public:
    MatrixMixerAU(AudioComponentInstance instance) : AUBase(instance, 1, 1) {}
    bool CanScheduleParameters() const override { return false; }
    bool StreamFormatWritable(AudioUnitScope, AudioUnitElement) override { return !IsInitialized(); }
    bool BusCountWritable(AudioUnitScope scope) override {
        return scope == kAudioUnitScope_Input || scope == kAudioUnitScope_Output;
    }
    bool ValidFormat(AudioUnitScope scope, AudioUnitElement, const CAStreamBasicDescription& f) override {
        if (!f.IsPCM() || !f.IsFloat() || f.mBitsPerChannel != 32 ||
            !f.mChannelsPerFrame || f.mChannelsPerFrame >= 0xffff || f.mSampleRate <= 0 ||
            (f.mFormatFlags & kAudioFormatFlagIsBigEndian) || f.mFramesPerPacket != 1)
            return false;
        UInt32 bytes = 4 * (f.IsInterleaved() ? f.mChannelsPerFrame : 1);
        return f.mBytesPerFrame == bytes && f.mBytesPerPacket == bytes;
    }
    OSStatus Initialize() override {
        if (!Inputs().GetNumberOfElements() || !Outputs().GetNumberOfElements() ||
            channels(kAudioUnitScope_Input) >= 0xffff || channels(kAudioUnitScope_Output) >= 0xffff)
            return kAudioUnitErr_FormatNotSupported;
        // Only buses that are in use have to agree on the rate: a client may declare more
        // buses than it configures, and inputs with no source are never pulled.
        double rate = GetOutput(0)->GetStreamFormat().mSampleRate;
        for (UInt32 b = 0; b < Inputs().GetNumberOfElements(); ++b) {
            const double r = GetInput(b)->GetStreamFormat().mSampleRate;
            if (GetInput(b)->IsActive() && r != rate) {
                return kAudioUnitErr_FormatNotSupported;
            }
        }
        return AUBase::Initialize();
    }
    OSStatus SetParameter(AudioUnitParameterID id, AudioUnitScope scope, AudioUnitElement element,
                          AudioUnitParameterValue gain, UInt32) override {
        std::lock_guard<std::mutex> lock(mMutex);
        if (!std::isfinite(gain)) return kAudioUnitErr_InvalidParameter;
        if (id == kMatrixMixerParam_Volume) {
            if (scope == kAudioUnitScope_Input) {
                if (element >= channels(scope)) return kAudioUnitErr_InvalidElement;
                element = (element << 16) | 0xffff;
            } else if (scope == kAudioUnitScope_Output) {
                if (element >= channels(scope)) return kAudioUnitErr_InvalidElement;
                element |= 0xffff0000;
            } else if (scope != kAudioUnitScope_Global) return kAudioUnitErr_InvalidScope;
            if (!validGain(element)) return kAudioUnitErr_InvalidElement;
            mGains[element] = gain;
            return noErr;
        }
        if (id == kMatrixMixerParam_Enable &&
            (scope == kAudioUnitScope_Input || scope == kAudioUnitScope_Output)) {
            if (element >= GetScope(scope).GetNumberOfElements()) return kAudioUnitErr_InvalidElement;
            (scope == kAudioUnitScope_Input ? mInputEnabled : mOutputEnabled)[element] = gain != 0;
            return noErr;
        }
        return kAudioUnitErr_InvalidParameter;
    }
    OSStatus GetParameter(AudioUnitParameterID id, AudioUnitScope scope, AudioUnitElement element,
                          AudioUnitParameterValue& gain) override {
        std::lock_guard<std::mutex> lock(mMutex);
        if (id == kMatrixMixerParam_Volume) {
            if (scope == kAudioUnitScope_Input || scope == kAudioUnitScope_Output) {
                if (element >= channels(scope)) return kAudioUnitErr_InvalidElement;
                element = scope == kAudioUnitScope_Input ? (element << 16) | 0xffff : element | 0xffff0000;
            } else if (scope != kAudioUnitScope_Global) return kAudioUnitErr_InvalidScope;
            if (!validGain(element)) return kAudioUnitErr_InvalidElement;
            gain = value(mGains, element);
            return noErr;
        }
        if (id == kMatrixMixerParam_Enable &&
            (scope == kAudioUnitScope_Input || scope == kAudioUnitScope_Output)) {
            if (element >= GetScope(scope).GetNumberOfElements()) return kAudioUnitErr_InvalidElement;
            gain = value(scope == kAudioUnitScope_Input ? mInputEnabled : mOutputEnabled, element, 1);
            return noErr;
        }
        return kAudioUnitErr_InvalidParameter;
    }
    OSStatus RenderBus(AudioUnitRenderActionFlags& flags, const AudioTimeStamp& time,
                       UInt32 bus, UInt32 frames) override {
        auto& output = GetOutput(bus)->GetBufferList();
        for (UInt32 b = 0; b < output.mNumberBuffers; ++b)
            if (output.mBuffers[b].mData) memset(output.mBuffers[b].mData, 0, output.mBuffers[b].mDataByteSize);
        UInt32 outputOffset = 0;
        for (UInt32 b = 0; b < bus; ++b) outputOffset += GetOutput(b)->GetStreamFormat().mChannelsPerFrame;
        UInt32 inputOffset = 0;
        bool silent = true;
        for (UInt32 b = 0; b < Inputs().GetNumberOfElements(); ++b) {
            auto* input = GetInput(b);
            UInt32 inputChannels = input->GetStreamFormat().mChannelsPerFrame;
            // Pull without holding the parameter lock: callbacks may change parameters.
            if (input->IsActive()) {
                AudioUnitRenderActionFlags inputFlags = 0;
                OSStatus status = input->PullInput(inputFlags, time, b, frames);
                if (status) return status;
                auto& data = input->GetBufferList();
                std::lock_guard<std::mutex> lock(mMutex);
                if (value(mInputEnabled, b, 1) && value(mOutputEnabled, bus, 1) &&
                    !(inputFlags & kAudioUnitRenderAction_OutputIsSilence)) {
                    for (UInt32 i = 0; i < inputChannels; ++i) {
                        UInt32 inputStride = 0;
                        Float32* src = sample(data, i, inputStride);
                        for (UInt32 o = 0; o < GetOutput(bus)->GetStreamFormat().mChannelsPerFrame; ++o) {
                            UInt32 outputStride = 0;
                            Float32* dst = sample(output, o, outputStride);
                            Float32 gain = value(mGains, 0xffffffff) *
                                value(mGains, ((inputOffset+i) << 16) | 0xffff) *
                                value(mGains, 0xffff0000 | (outputOffset+o)) *
                                value(mGains, ((inputOffset+i) << 16) | (outputOffset+o));
                            if (!src || !dst || !gain) continue;
                            for (UInt32 f = 0; f < frames; ++f) dst[f*outputStride] += src[f*inputStride] * gain;
                            silent = false;
                        }
                    }
                }
            }
            inputOffset += inputChannels;
        }
        if (silent) flags |= kAudioUnitRenderAction_OutputIsSilence;
        else flags &= ~kAudioUnitRenderAction_OutputIsSilence;
        return noErr;
    }
};
#pragma GCC visibility push(default)
AUDIOCOMPONENT_ENTRY(AUBaseFactory, MatrixMixerAU);
#pragma GCC visibility pop
