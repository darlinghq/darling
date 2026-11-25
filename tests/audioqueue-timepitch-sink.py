#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Checks what an AudioQueue with the time/pitch effect sends to the audio output: runs tests/audioqueue-timepitch-play.c in
# the guest (a stereo tone, left 440 Hz and right 660 Hz, two seconds), records the output on a private PulseAudio null sink and
# measures the duration and the frequency of each channel, and the AudioQueueGetCurrentTime sample time the program prints.
# Checked: the Spectral and TimeDomain algorithms change the duration with the play rate and keep the pitch, Varispeed changes
# the pitch with the rate, Pitch of +-1200 cents doubles or halves the frequency with Spectral (and does nothing with TimeDomain,
# and moves the speed with Varispeed), the effect does nothing when it is not enabled or when it is bypassed, a play rate changed
# while playing takes effect without a break, and the sample time moves at the play rate.
#
# Needs a host with PulseAudio (or PipeWire's pulse server): only the streams of the program are moved to the null sink, the
# default sink stays as it is.
#
#   audioqueue-timepitch-sink.py --player /guest/path/audioqueue-timepitch-play [--shell "darling shell"]
#
# Exit status 0 if all passed.
import argparse, struct, subprocess, sys, threading, time

RATE = 44100
SINK = 'aqtp_sink'
MATCH = 'audioqueue-timepitch-play'


def sh(*cmd):
    return subprocess.run(cmd, capture_output=True, text=True).stdout


def sink_inputs():
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


def record(command):
    """Runs the command while recording a null sink; returns (left, right, the command's output lines)."""
    module = sh('pactl', 'load-module', 'module-null-sink', 'sink_name=' + SINK, 'rate=%d' % RATE, 'channels=2').strip()
    mine = {line.split()[1]: line.split()[0] for line in sh('pactl', 'list', 'short', 'sinks').splitlines()}[SINK]
    stop = threading.Event()
    chunks = []
    parec = subprocess.Popen(['parec', '-d', SINK + '.monitor', '--format=s16le', '--rate=%d' % RATE, '--channels=2',
                              '--latency-msec=10', '--raw'], stdout=subprocess.PIPE)

    def reader():
        while not stop.is_set():
            data = parec.stdout.read(RATE // 100 * 4)
            if not data:
                break
            chunks.append(data)

    def mover():
        while not stop.is_set():
            for si in sink_inputs():
                if MATCH in si['name'] and si.get('sink') != mine:
                    sh('pactl', 'move-sink-input', si['id'], SINK)
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
    left, right = [], []
    for data in chunks:
        n = len(data) // 4
        values = struct.unpack('<%dh' % (n * 2), data[:n * 4])
        left.extend(values[0::2])
        right.extend(values[1::2])
    return left, right, result.stdout.splitlines()


def measure(x, threshold=1500):
    """(seconds of sound, Hz): the sound runs from the first to the last sample above the threshold, gaps of up to 60 ms bridged
    (a short dropout); the frequency is the median over the middle of the sound of 100 ms windows, each counted from its rising
    zero crossings, a window with a dropout in it (a run of near-silence) left out."""
    idx = [i for i, v in enumerate(x) if abs(v) > threshold]
    if len(idx) < 100:
        return 0.0, 0.0
    runs, start, prev = [], idx[0], idx[0]
    for i in idx[1:]:
        if i - prev > RATE * 0.06:
            runs.append((start, prev))
            start = i
        prev = i
    runs.append((start, prev))
    first, last = max(runs, key=lambda r: r[1] - r[0])
    win = RATE // 10
    freqs = []
    for a in range(first + (last - first) // 5, last - (last - first) // 5 - win, win):
        w = x[a:a + win]
        longest = run = 0
        for v in w:
            run = run + 1 if abs(v) < 60 else 0
            longest = max(longest, run)
        if longest < 40:
            freqs.append(sum(1 for k in range(1, win) if w[k - 1] < 0 <= w[k]) * 10.0)
    freqs.sort()
    return (last - first) / RATE, freqs[len(freqs) // 2] if freqs else 0.0


failures = 0


def check(cond, text):
    global failures
    print(('PASS ' if cond else 'FAIL ') + text)
    if not cond:
        failures += 1


def near(value, expected, tolerance):
    return abs(value - expected) <= tolerance


def hz_tol(hz):
    """3 %, but not less than the resolution of the measurement (10 Hz) and a bit."""
    return max(0.03 * hz, 12.0)


def slope(out, lo, hi):
    """Sample time per second of wall time between the wall times lo and hi (seconds after START)."""
    start = [float(l.split()[1]) for l in out if l.startswith('START')]
    pts = [(float(l.split()[1]) - start[0], float(l.split()[2])) for l in out if l.startswith('CT')] if start else []
    pts = [p for p in pts if lo <= p[0] <= hi]
    if len(pts) < 3:
        return None
    return (pts[-1][1] - pts[0][1]) / (pts[-1][0] - pts[0][0])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--player', required=True, help='the guest path of audioqueue-timepitch-play')
    ap.add_argument('--shell', default='darling shell')
    args = ap.parse_args()

    def run(case, expect_seconds, hz_left, hz_right, *params, slope_range=None, expect_slope=None):
        """One case, with a few attempts: a loaded host can starve the output for a moment."""
        text = ' '.join(params)
        for attempt in range(3):
            left, right, out = record(args.shell.split() + [args.player] + list(params))
            (dl, fl), (dr, fr) = measure(left), measure(right)
            if near(dl, expect_seconds, 0.12 * expect_seconds + 0.2) and near(fl, hz_left, hz_tol(hz_left)):
                break
        tol = 0.12 * expect_seconds + 0.2
        check(near(dl, expect_seconds, tol) and near(dr, expect_seconds, tol),
              '%s: the sound lasts %.2f s (left %.2f, right %.2f)' % (case, expect_seconds, dl, dr))
        check(near(fl, hz_left, hz_tol(hz_left)) and near(fr, hz_right, hz_tol(hz_right)),
              '%s: left %.0f Hz and right %.0f Hz (heard %.1f and %.1f Hz)' % (case, hz_left, hz_right, fl, fr))
        if expect_slope is not None:
            s = slope(out, *slope_range)
            check(s is not None and near(s, expect_slope, 0.12 * expect_slope),
                  '%s: the sample time moves at %.0f frames per second (%s)' % (case, expect_slope, 'no data' if s is None else '%.0f' % s))
        return out

    # without the effect the rate and pitch do nothing; the sample time moves with the time
    run('no effect, rate 2, pitch 1200', 2.0, 440, 660, 'none', '2', '1200', slope_range=(0.8, 1.6), expect_slope=44100)
    # Spectral and TimeDomain: the duration follows the rate, the pitch stays
    for alg, name in (('spec', 'Spectral'), ('tido', 'TimeDomain')):
        for rate in (0.5, 1.5, 2.0):
            lo, hi = 0.8, 0.4 + 0.85 * 2.0 / rate  # after the sound has started, before it ends
            run('%s rate %.1f' % (name, rate), 2.0 / rate, 440, 660, alg, str(rate), '0',
                slope_range=(lo, hi), expect_slope=44100 * rate)
    # Varispeed: the pitch follows the rate
    run('Varispeed rate 1.5', 2.0 / 1.5, 660, 990, 'vspd', '1.5', '0')
    run('Varispeed rate 2', 1.0, 880, 1320, 'vspd', '2', '0')
    run('Varispeed rate 0.5', 4.0, 220, 330, 'vspd', '0.5', '0')
    # pitch, in cents
    run('Spectral pitch +1200', 2.0, 880, 1320, 'spec', '1', '1200')
    run('Spectral pitch -1200', 2.0, 220, 330, 'spec', '1', '-1200')
    run('Spectral rate 1.5, pitch +1200', 2.0 / 1.5, 880, 1320, 'spec', '1.5', '1200')
    run('TimeDomain pitch +1200 (not applied)', 2.0, 440, 660, 'tido', '1', '1200')
    run('Varispeed pitch +1200', 1.0, 880, 1320, 'vspd', '1', '1200')
    # the bypass
    run('Spectral rate 2, bypassed', 2.0, 440, 660, 'spec', '2', '0', 'bypass')
    # a rate changed while playing: 0.8 s at rate 1, the other 1.2 s of sound at rate 2
    run('Spectral rate 1, changed to 2 after 0.8 s', 1.4, 440, 660, 'spec', '1', '0', 'change', '0.8', '2')
    # and back to the normal rate: what the effect holds is handed over to the plain output without a break
    run('Spectral rate 2, changed to 1 after 0.4 s', 1.6, 440, 660, 'spec', '2', '0', 'change', '0.4', '1')

    print('RESULT FAIL (%d)' % failures if failures else 'RESULT PASS')
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
