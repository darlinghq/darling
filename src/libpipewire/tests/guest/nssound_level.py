#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
# Checks the recording of the null sink made while `nssound_play <dir> levels` played two one-second tones of
# amplitude 0.5: the first at the full volume of the sound, the second at a quarter of it. The recording is a
# 16-bit PCM WAV. Prints the result and exits 0 when both tones are there at the expected level.
import sys, wave, struct

w = wave.open(sys.argv[1], 'rb')
ch, rate, width = w.getnchannels(), w.getframerate(), w.getsampwidth()
assert width == 2, 'expected 16-bit samples'
data = w.readframes(w.getnframes())
n = len(data) // (2 * ch)
samples = struct.unpack('<%dh' % (n * ch), data[:n * ch * 2])
win = rate // 20  # 50 ms windows
peaks = []
for i in range(0, n - win, win):
    peaks.append(max(abs(x) for x in samples[i * ch:(i + win) * ch]) / 32768.0)

# tones are the runs of windows above a threshold
segments, start = [], None
for i, p in enumerate(peaks + [0.0]):
    if p > 0.01 and start is None:
        start = i
    elif p <= 0.01 and start is not None:
        segments.append((start, i)); start = None
segments = [s for s in segments if s[1] - s[0] >= 4]
info = []
for a, b in segments:
    info.append((peaks[a:b] and max(peaks[a:b]), (b - a) * win / rate))
print('recording %.2f s, %d Hz, %d ch; tones (peak, seconds): %s' % (n / rate, rate, ch, ['(%.3f, %.2f)' % t for t in info]))
ok = len(info) == 2
if ok:
    (p1, d1), (p2, d2) = info
    ok = 0.40 <= p1 <= 0.60 and 0.20 <= p2 / p1 <= 0.30 and 0.7 <= d1 <= 1.4 and 0.7 <= d2 <= 1.4
    print('ratio quiet/loud %.3f' % (p2 / p1))
print('LEVELS PASS' if ok else 'LEVELS FAIL')
sys.exit(0 if ok else 1)
