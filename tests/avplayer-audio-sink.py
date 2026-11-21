#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Checks what AVPlayer sends to the audio output: plays the audio fixtures of tests/avplayer-fixtures in the
# guest with tests/avplayer-audio-play.m, records them on a private PulseAudio null sink and analyses the
# recording. Checked: every codec is heard as the 440 Hz and 880 Hz tones of the fixture (about one
# second each, without gaps), the volume and muting, the rate (its duration, not the pitch), a seek, and the audio and video of a clip
# with a beep and a flash arriving together (the flash frame is shown when the beep is heard).
#
# Needs a host with PulseAudio (or PipeWire's pulse server): only the streams of the player program are
# moved to the null sink, the default sink stays as it is. The guest program is built with
# guest-cc.sh (see the header of avplayer-audio-play.m).
#
#   avplayer-audio-sink.py --player /guest/path/avplayer-audio-play --fixtures /guest/path/tests/avplayer-fixtures \
#       [--shell "darling shell"] [--match avplayer-audio-play]
#
# --match is a part of the stream's application name (the name of the program). Exit status 0 if all passed.
import argparse, json, math, os, struct, subprocess, sys, threading, time

RATE = 44100


def sh(*cmd):
    return subprocess.run(cmd, capture_output=True, text=True).stdout


class Recorder:
    """A private null sink whose monitor is recorded in 10 ms chunks with their arrival times."""

    def __init__(self, match):
        self.match = match
        self.module = None

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
        """Runs the command; returns (samples, wall time of sample 0, command output)."""
        self.module = sh('pactl', 'load-module', 'module-null-sink', 'sink_name=avpa_sink', 'rate=%d' % RATE, 'channels=2').strip()
        stop = threading.Event()
        chunks = []
        mine = {line.split()[1]: line.split()[0] for line in sh('pactl', 'list', 'short', 'sinks').splitlines()}['avpa_sink']
        parec = subprocess.Popen(['parec', '-d', 'avpa_sink.monitor', '--format=s16le', '--rate=%d' % RATE, '--channels=1',
                                  '--latency-msec=10', '--raw'], stdout=subprocess.PIPE)

        def reader():
            while not stop.is_set():
                data = parec.stdout.read(RATE // 100 * 2)
                if not data:
                    break
                chunks.append((time.time(), data))

        def mover():
            while not stop.is_set():
                # The session manager can move a stream away when the sinks of the host change: put it back.
                for si in self.sink_inputs():
                    if self.match in si['name'] and si.get('sink') != mine:
                        sh('pactl', 'move-sink-input', si['id'], 'avpa_sink')
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
            sh('pactl', 'unload-module', self.module)
        samples, stamps = [], []
        for t, data in chunks:
            n = len(data) // 2
            stamps.append((len(samples), t))
            samples.extend(struct.unpack('<%dh' % n, data[:n * 2]))
        # The wall time of sample n is offset + n / RATE; the median over the later chunks smooths the jitter.
        offsets = sorted(t - n / RATE for n, t in stamps[len(stamps) // 4:])
        return samples, offsets[len(offsets) // 2], result.stdout


def segments(samples):
    """Runs of sound with their frequency band: [(start s, end s, rms, Hz)], gaps shorter than 40 ms are bridged."""
    win = RATE // 100
    wins = []
    for i in range(0, len(samples) - win, win):
        w = samples[i:i + win]
        rms = math.sqrt(sum(x * x for x in w) / win) / 32768
        crossings = sum(1 for k in range(1, win) if (w[k - 1] < 0) != (w[k] < 0))
        wins.append((i, rms, crossings * RATE / win / 2))
    runs, cur = [], None
    for i, rms, f in wins:
        if rms <= 0.02:
            continue
        band = 0 if f < 650 else 1 if f < 1300 else 2
        if cur and cur['band'] == band and i - cur['end'] <= 4 * win:
            cur['end'] = i + win
            cur['rms'] = max(cur['rms'], rms)
            cur['f'].append(f)
        else:
            if cur:
                runs.append(cur)
            cur = {'start': i, 'end': i + win, 'rms': rms, 'f': [f], 'band': band}
    if cur:
        runs.append(cur)
    out = []
    for r in runs:
        if r['end'] - r['start'] >= 3 * win:
            f = sorted(r['f'])
            out.append((r['start'] / RATE, r['end'] / RATE, r['rms'], f[len(f) // 2]))
    return out


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
    ap.add_argument('--player', required=True, help='the guest path of avplayer-audio-play')
    ap.add_argument('--fixtures', required=True, help='the guest path of tests/avplayer-fixtures')
    ap.add_argument('--shell', default='darling shell')
    ap.add_argument('--match', default='avplayer-audio-play')
    args = ap.parse_args()
    rec = Recorder(args.match)

    def play(fixture, *player_args):
        cmd = args.shell.split() + [args.player, os.path.join(args.fixtures, fixture)] + [str(a) for a in player_args]
        return rec.run(cmd)

    tones = ['tones-aac.m4a', 'tones-mp3.mp3', 'tones-ac3.ac3', 'tones-eac3.eac3', 'tones-flac.flac', 'tones-alac.m4a',
             'tones-opus.opus', 'tones-pcm.wav']
    for name in tones:
        # A loaded host can starve the output for a moment: a fixture gets three attempts.
        for attempt in range(3):
            samples, _, _ = play(name, 4)
            segs = segments(samples)
            total = sum(e - s for s, e, _, _ in segs)
            low = [s for s in segs if s[3] < 650]
            high = [s for s in segs if 650 <= s[3] < 1300]
            order = len(low) >= 1 and len(high) >= 1 and low[0][1] <= high[0][0] + 0.05
            level = max((s[2] for s in segs), default=0)
            if order and 1.7 < total < 2.4 and len(segs) <= 3 and near(level, 0.354, 0.1):
                break
        check(order, '%s: the 440 Hz tone, then the 880 Hz tone (%s)' % (name, ', '.join('%.0f Hz %.2f-%.2f' % (f, s, e) for s, e, _, f in segs)))
        check(1.7 < total < 2.4, '%s: about two seconds of sound (%.2f s)' % (name, total))
        check(len(segs) <= 3, '%s: without gaps (%d runs)' % (name, len(segs)))
        check(near(level, 0.354, 0.1), '%s: the level of a 0.5 sine (rms %.2f)' % (name, level))

    # volume, mute, rate, seek
    samples, _, _ = play('tones-mp3.mp3', 4, 1, 0.5)
    full = max((s[2] for s in segments(samples)), default=0)
    check(near(full, 0.177, 0.06), 'volume 0.5 halves the level (rms %.3f)' % full)
    samples, _, _ = play('tones-mp3.mp3', 4, 1, 1, 1)
    check(not segments(samples), 'a muted player is silent')
    # The default algorithm (Spectral) and TimeDomain keep the pitch at another rate, Varispeed lets it follow.
    for algorithm, label, hz in (('default', 'the default algorithm', (440, 880)), ('TimeDomain', 'TimeDomain', (440, 880)),
                                 ('LowQualityZeroLatency', 'LowQualityZeroLatency', (440, 880)), ('Varispeed', 'Varispeed', (880, 1760))):
        for attempt in range(3):
            samples, _, _ = play('tones-mp3.mp3', 4, 2, 1, 0, -1, algorithm)
            seconds, parts = tone_pitches(samples, 4)
            low, high = parts[0], parts[3]
            if near(seconds, 1.0, 0.25) and near(low, hz[0], 0.1 * hz[0]) and near(high, hz[1], 0.1 * hz[1]):
                break
        check(near(seconds, 1.0, 0.25), 'rate 2, %s: the two seconds take one (%.2f s)' % (label, seconds))
        check(near(low, hz[0], 0.1 * hz[0]) and near(high, hz[1], 0.1 * hz[1]), 'rate 2, %s: %d Hz, then %d Hz (%.0f Hz, %.0f Hz)' % (label, hz[0], hz[1], low, high))
    # a rate set while playing, and the algorithm changed while playing: the sound goes on at the position
    for attempt in range(3):
        samples, _, _ = play('tones-mp3.mp3', 4, 1, 1, 0, -1, 'default', 2)
        seconds, parts = tone_pitches(samples, 3)
        if near(seconds, 1.5 + 0.25, 0.4) and near(parts[2], 880, 90):
            break
    check(near(seconds, 1.5 + 0.25, 0.4) and near(parts[0], 440, 45) and near(parts[2], 880, 90),
          'rate 2 set after 1.5 s: 1.5 s plus the rest at double speed, the pitch stays (%.2f s; %.0f Hz ... %.0f Hz)' % (seconds, parts[0], parts[2]))
    for attempt in range(3):
        samples, _, _ = play('tones-mp3.mp3', 5, 0.8, 1, 0, -1, 'default', 'none', 'Varispeed')
        seconds, parts = tone_pitches(samples, 5)
        if parts[4] < 800 and seconds > 2.0:
            break
    check(parts[4] < 800 and seconds > 2.0,
          'the algorithm changed to Varispeed while playing at rate 0.8: the 880 Hz tone ends up lower, at about 700 Hz (%.2f s; %.0f Hz at the end)' % (seconds, parts[4]))
    samples, _, _ = play('tones-mp3.mp3', 4, 1, 1, 0, 1.3)
    segs = segments(samples)
    total = sum(e - s for s, e, _, _ in segs)
    check(near(total, 0.7, 0.2) and all(near(f, 880, 50) for _, _, _, f in segs), 'seek to 1.3 s: 0.7 s of the 880 Hz tone (%.2f s)' % total)

    # a flash frame and a beep that belong together
    samples, offset, out = play('avsync-h264-aac.mp4', 3)
    segs = segments(samples)
    flash = [float(l.split()[2]) for l in out.splitlines() if l.startswith('FRAME') and int(l.split()[1]) > 128]
    beeps = [s for s in segs if near(s[3], 1000, 100)]
    check(len(flash) == 1 and len(beeps) == 1, 'one flash frame and one beep (%d, %d)' % (len(flash), len(beeps)))
    if flash and beeps:
        delay = (offset + beeps[0][0]) - flash[0]
        check(abs(delay) < 0.1, 'the beep is heard %+.0f ms after the flash frame is shown' % (delay * 1000))

    print('RESULT FAIL (%d)' % failures if failures else 'RESULT PASS')
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
