#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Checks what AVAudioPlayer sends to the audio output: runs tests/avaudioplayer-play.m in the guest with a script
# of steps, records the output on a private PulseAudio null sink (in stereo) and analyses the recording. Checked:
# the tone is heard at the right level and pitch, the volume, the pan (left and right channel), the channels of
# the sound, loops, seeking (before and during playback), pause and resume, a fade, the rate (the duration, not the
# pitch), and playAtTime:.
#
# Needs a host with PulseAudio (or PipeWire's pulse server): only the streams of the player program are
# moved to the null sink, the default sink stays as it is.
#
#   avaudioplayer-sink.py --player /guest/path/avaudioplayer-play [--shell "darling shell"] [--match avaudioplayer-play]
#
# --match is a part of the stream's application name (the name of the program). Exit status 0 if all passed.
import argparse, math, struct, subprocess, sys, threading, time

RATE = 44100


def sh(*cmd):
    return subprocess.run(cmd, capture_output=True, text=True).stdout


class Recorder:
    """A private null sink whose monitor is recorded as stereo, with the arrival time of the chunks."""

    def __init__(self, match):
        self.match = match

    def sink_inputs(self):
        inputs, cur = [], None
        for line in sh('pactl', 'list', 'sink-inputs').splitlines():
            line = line.strip()
            if line.startswith('Sink Input #'):
                cur = {'id': line.split('#')[1], 'name': ''}
                inputs.append(cur)
            elif cur is not None and line.startswith('Sink:'):
                cur['sink'] = line.split()[1]
            elif cur is not None and line.startswith('application.name'):
                cur['name'] = line.split('"')[1]
        return inputs

    def run(self, command):
        """Runs the command; returns (left samples, right samples, wall time of sample 0, output lines)."""
        module = sh('pactl', 'load-module', 'module-null-sink', 'sink_name=avap_sink', 'rate=%d' % RATE, 'channels=2').strip()
        stop = threading.Event()
        chunks = []
        mine = {line.split()[1]: line.split()[0] for line in sh('pactl', 'list', 'short', 'sinks').splitlines()}['avap_sink']
        parec = subprocess.Popen(['parec', '-d', 'avap_sink.monitor', '--format=s16le', '--rate=%d' % RATE, '--channels=2',
                                  '--latency-msec=10', '--raw'], stdout=subprocess.PIPE)

        def reader():
            while not stop.is_set():
                data = parec.stdout.read(RATE // 100 * 4)
                if not data:
                    break
                chunks.append((time.time(), data))

        def mover():
            while not stop.is_set():
                for si in self.sink_inputs():
                    if self.match in si['name'] and si.get('sink') != mine:
                        sh('pactl', 'move-sink-input', si['id'], 'avap_sink')
                time.sleep(0.02)

        threads = [threading.Thread(target=reader), threading.Thread(target=mover)]
        for t in threads:
            t.start()
        try:
            time.sleep(1.0)
            result = subprocess.run(command, capture_output=True, text=True, timeout=120)
            time.sleep(1.0)
        finally:
            stop.set()
            parec.terminate()
            for t in threads:
                t.join(timeout=3)
            sh('pactl', 'unload-module', module)
        left, right, stamps = [], [], []
        for t, data in chunks:
            n = len(data) // 4
            stamps.append((len(left), t))
            values = struct.unpack('<%dh' % (n * 2), data[:n * 4])
            left.extend(values[0::2])
            right.extend(values[1::2])
        offsets = sorted(t - n / RATE for n, t in stamps[len(stamps) // 4:])
        return left, right, offsets[len(offsets) // 2], result.stdout.splitlines()


def _run_queue(self, command, accept):
    """Records a command whose stream has another name; a few attempts like play()."""
    saved = self.match
    self.match = self.queue_match
    try:
        for attempt in range(3):
            result = self.run(command)
            if accept(result[0], result[1]):
                break
    finally:
        self.match = saved
    return result


Recorder.run_queue = _run_queue


def windows(samples):
    """[(start s, rms, Hz)] per 10 ms."""
    win = RATE // 100
    out = []
    for i in range(0, len(samples) - win, win):
        w = samples[i:i + win]
        rms = math.sqrt(sum(x * x for x in w) / win) / 32768
        crossings = sum(1 for k in range(1, win) if (w[k - 1] < 0) != (w[k] < 0))
        out.append((i / RATE, rms, crossings * RATE / win / 2))
    return out


def segments(samples, floor=0.02):
    """Runs of sound with their pitch: [(start s, end s, max rms, Hz)]; gaps shorter than 60 ms are bridged."""
    runs, cur = [], None
    for start, rms, f in windows(samples):
        if rms <= floor:
            continue
        band = 0 if f < 650 else 1 if f < 1300 else 2
        if cur and cur['band'] == band and start - cur['end'] <= 0.06:
            cur['end'] = start + 0.01
            cur['rms'] = max(cur['rms'], rms)
            cur['f'].append(f)
        else:
            if cur:
                runs.append(cur)
            cur = {'start': start, 'end': start + 0.01, 'rms': rms, 'f': [f], 'band': band}
    if cur:
        runs.append(cur)
    out = []
    for r in runs:
        if r['end'] - r['start'] >= 0.03:
            f = sorted(r['f'])
            out.append((r['start'], r['end'], r['rms'], f[len(f) // 2]))
    return out


def level(samples):
    return max((s[2] for s in segments(samples)), default=0)


def total(samples):
    return sum(e - s for s, e, _, _ in segments(samples))


failures = 0


def check(cond, text):
    global failures
    print(('PASS ' if cond else 'FAIL ') + text)
    if not cond:
        failures += 1


def near(value, expected, tolerance):
    return abs(value - expected) <= tolerance


def tone_pitches(samples, parts=2):
    """The sound as a whole (gaps of up to 60 ms bridged: a short dropout of the output is not a break) and the frequency of each of
    `parts` equal parts of it: (seconds of sound, [Hz of each part]). The frequency of a part is the median of its 50 ms windows,
    each counted from the rising zero crossings; a window with a dropout in it (a run of near-silence) is left out. The length of
    the sound is from its first to its last sample above the threshold."""
    threshold = 1500
    idx = [i for i, v in enumerate(samples) if abs(v) > threshold]
    if len(idx) < 100:
        return 0.0, [0.0] * parts
    runs, start, prev = [], idx[0], idx[0]
    for i in idx[1:]:
        if i - prev > RATE * 0.06:
            runs.append((start, prev))
            start = i
        prev = i
    runs.append((start, prev))
    first, last = max(runs, key=lambda r: r[1] - r[0])
    win = RATE // 20
    freqs = []
    for a in range(first, last - win, win):
        w = samples[a:a + win]
        longest = run = 0
        for v in w:
            run = run + 1 if abs(v) < 60 else 0
            longest = max(longest, run)
        if longest < 40:
            freqs.append((a, sum(1 for k in range(1, win) if w[k - 1] < 0 <= w[k]) * 20.0))
    out = []
    for p in range(parts):
        lo, hi = first + (last - first) * p // parts, first + (last - first) * (p + 1) // parts
        # windows in the middle of the part: a window that straddles a change of pitch tells nothing
        mid = sorted(f for a, f in freqs if lo <= a and a + win <= hi)
        out.append(mid[len(mid) // 2] if mid else 0.0)
    return (last - first) / RATE, out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--player', required=True, help='the guest path of avaudioplayer-play')
    ap.add_argument('--queue-player', help='the guest path of audioqueue-drain-play (optional)')
    ap.add_argument('--shell', default='darling shell')
    ap.add_argument('--match', default='avaudioplayer-play')
    args = ap.parse_args()
    rec = Recorder(args.match)
    rec.queue_match = 'audioqueue-drain-play'

    def play(sound, *steps, attempts=3, accept=None):
        """Runs a script; a loaded host can starve the output for a moment, so it gets a few attempts."""
        for attempt in range(attempts):
            cmd = args.shell.split() + [args.player, sound] + list(steps)
            left, right, offset, out = rec.run(cmd)
            if accept is None or accept(left, right):
                break
        return left, right, offset, out

    # The tone: 440 Hz, amplitude 0.5 (rms 0.354), one second, on both channels
    for sound in ('sine', 'sine-file', 'mono'):
        left, right, _, out = play(sound, 'play', 'wait:1.6', accept=lambda l, r: near(total(l), 1.0, 0.2))
        for name, ch in (('left', left), ('right', right)):
            segs = segments(ch)
            check(len(segs) == 1 and near(segs[0][3], 440, 30), '%s: %s channel is one run of 440 Hz (%s)' % (sound, name, ', '.join('%.0f Hz %.2f-%.2f' % (f, s, e) for s, e, _, f in segs)))
            check(near(total(ch), 1.0, 0.1), '%s: %s channel: all of the one second of sound (%.2f s)' % (sound, name, total(ch)))
            check(near(level(ch), 0.354, 0.06), '%s: %s channel: the level of a 0.5 sine (rms %.3f)' % (sound, name, level(ch)))
        check(any(l.startswith('DONE') and l.endswith(' 1') for l in out), '%s: the delegate was told that the sound finished' % sound)

    # volume
    left, right, _, _ = play('sine', 'vol:0.5', 'play', 'wait:1.6', accept=lambda l, r: near(total(l), 1.0, 0.2))
    check(near(level(left), 0.177, 0.04) and near(level(right), 0.177, 0.04), 'volume 0.5 halves the level (%.3f, %.3f)' % (level(left), level(right)))
    left, right, _, _ = play('sine', 'vol:0', 'play', 'wait:1.6')
    check(not segments(left) and not segments(right), 'volume 0 is silent')

    # pan: -1 is left only, 1 is right only; the channels of the sound are kept
    left, right, _, _ = play('sine', 'pan:-1', 'play', 'wait:1.6', accept=lambda l, r: near(total(l), 1.0, 0.2))
    check(near(level(left), 0.354, 0.06) and level(right) < 0.02, 'pan -1: left %.3f, right %.3f' % (level(left), level(right)))
    left, right, _, _ = play('sine', 'pan:1', 'play', 'wait:1.6', accept=lambda l, r: near(total(r), 1.0, 0.2))
    check(near(level(right), 0.354, 0.06) and level(left) < 0.02, 'pan 1: left %.3f, right %.3f' % (level(left), level(right)))
    left, right, _, _ = play('sine', 'pan:0.5', 'play', 'wait:1.6', accept=lambda l, r: near(total(r), 1.0, 0.2))
    check(near(level(right), 0.354, 0.06) and near(level(left), 0.177, 0.05), 'pan 0.5: left %.3f, right %.3f' % (level(left), level(right)))
    left, right, _, _ = play('left', 'play', 'wait:1.6', accept=lambda l, r: near(total(l), 1.0, 0.2))
    check(near(level(left), 0.354, 0.06) and level(right) < 0.02, 'a sound with only a left channel is heard on the left (%.3f, %.3f)' % (level(left), level(right)))
    left, right, _, _ = play('right', 'play', 'wait:1.6', accept=lambda l, r: near(total(r), 1.0, 0.2))
    check(near(level(right), 0.354, 0.06) and level(left) < 0.02, 'a sound with only a right channel is heard on the right (%.3f, %.3f)' % (level(left), level(right)))

    # loops
    left, _, _, _ = play('short', 'loops:2', 'play', 'wait:2.0', accept=lambda l, r: near(total(l), 1.2, 0.2))
    check(near(total(left), 1.2, 0.2), 'two loops: three plays of 0.4 s are heard (%.2f s)' % total(left))
    left, _, _, out = play('short', 'loops:-1', 'play', 'wait:2.4', 'stop', accept=lambda l, r: total(l) > 2.1)
    check(total(left) > 2.1, 'infinite loops: sound for the whole 2.4 s (%.2f s)' % total(left))
    check(not any(l.startswith('DONE') for l in out), 'infinite loops: no end before stop')

    # seeking: before play, and during it
    left, _, _, _ = play('tones', 'seek:1.2', 'play', 'wait:1.6', accept=lambda l, r: near(total(l), 0.8, 0.2))
    segs = segments(left)
    check(near(total(left), 0.8, 0.2) and all(near(f, 880, 60) for _, _, _, f in segs), 'seek to 1.2 s before play: 0.8 s of 880 Hz (%.2f s)' % total(left))
    left, _, _, _ = play('tones', 'play', 'wait:0.4', 'seek:1.0', 'wait:1.6', accept=lambda l, r: len(segments(l)) == 2)
    segs = segments(left)
    check(len(segs) == 2 and near(segs[0][3], 440, 40) and near(segs[1][3], 880, 60) and near(segs[0][1] - segs[0][0], 0.4, 0.2) and near(segs[1][1] - segs[1][0], 1.0, 0.2),
          'seek during playback: 440 Hz for about 0.4 s, then 880 Hz for a second (%s)' % ', '.join('%.0f Hz %.2f-%.2f' % (f, s, e) for s, e, _, f in segs))

    # the end of a sound is heard in full: the output is not stopped while it still holds the last of the sound
    left, _, _, _ = play('tones', 'play', 'wait:2.6', accept=lambda l, r: len(segments(l)) == 2)
    segs = segments(left)
    check(len(segs) == 2 and near(segs[0][1] - segs[0][0], 1.0, 0.08) and near(segs[1][1] - segs[1][0], 1.0, 0.08),
          'both seconds of the sound are heard in full (%s)' % ', '.join('%.0f Hz %.2f-%.2f' % (f, s, e) for s, e, _, f in segs))

    # pause and resume: the sound goes on from where it was paused
    left, _, _, _ = play('tones', 'play', 'wait:0.5', 'pause', 'wait:0.8', 'play', 'wait:2.0', accept=lambda l, r: len(segments(l)) >= 2)
    segs = segments(left)
    low = [s for s in segs if near(s[3], 440, 40)]
    high = [s for s in segs if near(s[3], 880, 60)]
    low_time = sum(e - s for s, e, _, _ in low)
    check(len(low) == 2 and near(low_time, 1.0, 0.25) and len(high) == 1 and near(high[0][1] - high[0][0], 1.0, 0.2) and
          low[1][0] - low[0][1] > 0.6, 'pause and resume: 440 Hz for 0.5 s, a gap, the rest of the 440 Hz and the 880 Hz (%s)' % ', '.join('%.0f Hz %.2f-%.2f' % (f, s, e) for s, e, _, f in segs))

    # a fade
    left, _, _, _ = play('long', 'play', 'fade:0:1.0', 'wait:2.0', accept=lambda l, r: segments(l))
    wins = windows(left)
    start = next((t for t, r, _ in wins if r > 0.02), 0)
    early = max((r for t, r, _ in wins if start < t < start + 0.15), default=0)
    middle = max((r for t, r, _ in wins if start + 0.45 < t < start + 0.6), default=0)
    late = max((r for t, r, _ in wins if start + 1.2 < t < start + 1.8), default=0)
    check(early > 0.25 and 0.08 < middle < 0.26 and late < 0.02, 'a fade to 0 over a second: %.3f at the start, %.3f half way, %.3f after it' % (early, middle, late))

    # rate: the duration changes, the pitch does not
    left, _, _, _ = play('tones', 'rate:2', 'play', 'wait:2.0', accept=lambda l, r: near(tone_pitches(l)[0], 1.0, 0.3))
    seconds, parts = tone_pitches(left, 4)
    check(near(seconds, 1.0, 0.3), 'rate 2: the two seconds take one (%.2f s)' % seconds)
    check(near(parts[0], 440, 40) and near(parts[3], 880, 80), 'rate 2 keeps the pitch: 440 Hz, then 880 Hz (%.0f Hz, %.0f Hz)' % (parts[0], parts[3]))
    left, _, _, _ = play('short', 'rate:0.5', 'play', 'wait:1.6', accept=lambda l, r: near(tone_pitches(l)[0], 0.8, 0.2))
    seconds, (low, high) = tone_pitches(left)
    check(near(seconds, 0.8, 0.2), 'rate 0.5: 0.4 s take 0.8 (%.2f s)' % seconds)
    check(near(low, 440, 40) and near(high, 440, 40), 'rate 0.5 keeps the pitch (%.0f Hz, %.0f Hz)' % (low, high))
    # a rate changed while playing: no break, the pitch stays
    left, _, _, _ = play('tones', 'play', 'wait:0.4', 'rate:2', 'wait:2.0', accept=lambda l, r: near(tone_pitches(l)[0], 1.2, 0.3))
    seconds, parts = tone_pitches(left, 4)
    check(near(seconds, 1.2, 0.3) and near(parts[0], 440, 40) and near(parts[3], 880, 80),
          'rate 2 set after 0.4 s: 0.4 s plus 0.8 s of sound, 440 Hz then 880 Hz (%.2f s; %.0f Hz ... %.0f Hz)' % (seconds, parts[0], parts[3]))

    # playAtTime: the sound starts later
    left, _, offset, out = play('sine', 'playat:0.8', 'wait:2.2', accept=lambda l, r: segments(l))
    segs = segments(left)
    called = float([l for l in out if l.startswith('PLAY')][0].split()[1])
    check(len(segs) == 1 and near((offset + segs[0][0]) - called, 0.8, 0.2), 'playAtTime: 0.8 s ahead: heard %.2f s after the call' % ((offset + segs[0][0]) - called if segs else -1))

    # The same at the level of AudioQueue, which AVAudioPlayer plays through: a queue told to stop when it has played what it
    # holds does not cut off the last of the sound
    if args.queue_player:
        def whole(l, r):
            return near(total(l), 1.0, 0.1)
        left, right, offset, out = rec.run_queue(args.shell.split() + [args.queue_player], whole)
        check(near(total(left), 1.0, 0.1) and near(total(right), 1.0, 0.1), 'AudioQueue: all of the one second is heard before the queue stops (%.2f s)' % total(left))
        started = [float(l.split()[1]) for l in out if l.startswith('START')]
        stopped = [float(l.split()[1]) for l in out if l.startswith('STOPPED')]
        segs = segments(left)
        if started and stopped and segs:
            check(stopped[0] >= offset + segs[0][1] - 0.05, 'AudioQueue: the queue is reported stopped when the sound has been heard (%+.0f ms)' % ((stopped[0] - (offset + segs[0][1])) * 1000))

    print('RESULT FAIL (%d)' % failures if failures else 'RESULT PASS')
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
