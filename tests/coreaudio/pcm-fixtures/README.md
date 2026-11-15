Copyright (C) 2026 Lubos Dolezel

# Linear PCM files made on macOS

Every file here was made on macOS 26.7.3 (x86_64) with `afconvert` from an audio file that
`make-source.c` writes: 44100 Hz, 16-bit little-endian CAF with a different ramp on each channel
(channel 0 is `frame * 60 - 30000`, channel 1 is `15000 - frame * 30`, channel `c` of more channels is
`frame * (c + 1) * 7 - 2000`, all as 16-bit integers). The stereo and mono files have 1000 frames, the six-channel
files 100. The converted files hold that same audio, so the tests can tell what every sample must be:
integer depths above 16 bits are the 16-bit value shifted up, floating-point samples are the value divided by 32768
and 8-bit samples keep the high byte (unsigned ones with the sign flipped).

`tests/src/audiofile_pcm.c` reads each file, and writes the same audio through AudioFile and ExtAudioFile to
compare the files byte for byte. The CAF files from `afconvert` also have an `info` chunk (the duration and the
source bit depth), which AudioFile does not write, so for those the comparison covers the `desc` and `chan`
chunks and the audio.

| File | Command (`src2.caf` is the stereo source, `src1.caf` mono, `src6.caf` six channels of 100 frames) |
|---|---|
| `pcm16.wav` | `afconvert -f WAVE -d LEI16 src2.caf pcm16.wav` |
| `pcm24.wav` | `afconvert -f WAVE -d LEI24 src2.caf pcm24.wav` |
| `float32.wav` | `afconvert -f WAVE -d LEF32 src2.caf float32.wav` |
| `float64.wav` | `afconvert -f WAVE -d LEF64 src1.caf float64.wav` (mono) |
| `ulaw.wav` | `afconvert -f WAVE -d ulaw src2.caf ulaw.wav` (companded, for the libavformat fallback) |
| `pcm8.wav` | `afconvert -f WAVE -d UI8 src2.caf pcm8.wav` |
| `pcm32-mono.wav` | `afconvert -f WAVE -d LEI32 src1.caf pcm32-mono.wav` |
| `pcm16-6ch.wav` | `afconvert -f WAVE -d LEI16 src6.caf pcm16-6ch.wav` (plain `fmt ` chunk without a layout) |
| `layout51.wav` | `afconvert -f WAVE -d LEI16 -l MPEG_5_1_A src6.caf layout51.wav` (WAVE_FORMAT_EXTENSIBLE, speaker mask 0x60f) |
| `stereo-layout.wav` | `afconvert -f WAVE -d LEI16 -l Stereo src2.caf stereo-layout.wav` |
| `pcm16.aiff`, `pcm24.aiff` | `afconvert -f AIFF -d BEI16 src2.caf pcm16.aiff`, `-d BEI24` |
| `pcm8-mono.aiff` | `afconvert -f AIFF -d BEI8 src1.caf pcm8-mono.aiff` |
| `layout51.aiff` | `afconvert -f AIFF -d BEI16 -l MPEG_5_1_A src6.caf layout51.aiff` (`CHAN` chunk) |
| `twos16.aifc`, `in24.aifc`, `in32.aifc` | `afconvert -f AIFC -d BEI16 src2.caf twos16.aifc`, `-d BEI24`, `-d BEI32` (compression types `twos`, `in24`, `in32`) |
| `int8.aifc` | `afconvert -f AIFC -d BEI8 src1.caf int8.aifc` (compression type `NONE`) |
| `fl32.aifc`, `fl64.aifc` | `afconvert -f AIFC -d BEF32 src2.caf fl32.aifc`, `-d BEF64` |
| `float32.caf` | `afconvert -f caff -d LEF32 src2.caf float32.caf` |
| `pcm24be.caf` | `afconvert -f caff -d BEI24 src2.caf pcm24be.caf` |
| `pcm16-6ch.caf` | `afconvert -f caff -d LEI16 src6.caf pcm16-6ch.caf` |
| `layout51.caf` | `afconvert -f caff -d LEI16 -l MPEG_5_1_A src6.caf layout51.caf` (`chan` chunk) |
| `stereo-layout.caf` | `afconvert -f caff -d LEI16 -l Stereo src2.caf stereo-layout.caf` |

`afconvert` on macOS 26.7.3 does not write little-endian AIFF-C (`sowt`): the test builds one by hand from the
AIFF-C specification instead.
