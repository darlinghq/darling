# PipeWire backend tests

Tests of the optional PipeWire camera, screen capture and audio backends against a private PipeWire session.

`session.sh start <dir>` starts the session (own session bus, PipeWire, WirePlumber without hardware monitors, a
null sink, a video test source with the Camera role, a headless KWin with `--no-lockscreen`, the desktop portal
with its KDE backend and a fullscreen test pattern: four quadrants red, green, blue and white, plus a blinking
square so that the compositor keeps producing frames). `session.sh stop <dir>` ends it. It prints the environment
the guest programs need. Run the guest with `darling shell env <that environment> <program>`.

The guest programs are built with `guest-cc.sh` (see darling-skills, darling-guest-testing):

| Program | Checks |
|---|---|
| `avcapture_camera.m expect-cameras` | the camera is listed with formats and a video data output gets frames of the negotiated size |
| `avcapture_camera.m expect-none` | without PipeWire no camera is listed |
| `screen_capture.m expect-image` / `expect-none` | `CGDisplayCreateImage` returns an image of the display size with non-uniform content / returns NULL |
| `screen_pattern.m avc` / `sck` | AVCaptureScreenInput / ScreenCaptureKit deliver BGRA frames whose quadrant centres have the colours of the pattern (`sck` is x86_64 only) |
| `screen_pattern.m avc-crop` / `sck-crop` | the crop rectangle / source rectangle and the output size select one quadrant (blue / green) of the pattern at half scale |
| `screen_no_daemon.m` | without a portal or PipeWire server, a CGDisplayStream fails to start, `startRunning` returns and an SCStream start completes with an error, each within a fraction of a second (run it with `XDG_RUNTIME_DIR` pointing at an empty directory) |
| `hal_play.c` | the HAL output device calls the IOProc at the real-time rate, reports a latency of a few thousand frames at most, and stops cleanly |
| `hal_volume.c` | the default output device reports and sets the volume and mute of the default sink of the server, and its property listeners fire when they are changed from outside (`wpctl`) |
| `nssound_play.m` (run by `nssound_run.sh`) | `NSSound` plays through the HAL: the delegate hears YES on the main thread when a sound ends and NO when it is stopped, pause/resume/stop/loops/`currentTime` behave, AIFF and CAF files play, and `nssound_level.py` checks on the recording of the null sink that two tones come out at the level of the sound times its `volume`. `NSSOUND_BACKEND=pulse` makes the guest use the pulse backend of the HAL (through PipeWire's pulse server) instead of the native PipeWire one. It starts the session with `session.sh start <dir> audio`, which leaves out the compositor, the portal and the test pattern |

`afplay <wav>` must play to the null sink and exit when the file ends (it sleeps for the reported output
latency before it exits, so a wrong latency shows as a long delay).

The portal asks for the user's consent: `keys.sh <dir> <delay> <count> <kwin-keys>` presses Return on the
private KWin (the injector is `tests/kwin-keys.c` of darling-cocotron, built as its header says). It refuses to
send anything to a KWin that was not started with `--no-lockscreen`. Never point it at a real session.
