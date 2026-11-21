#!/usr/bin/env python3
# Integration test for MediaPlayer MPRIS multi-app switching and remote commands in Darling.
# usage: python3 tests/mediaplayer-guest-mpris.py

import os
import subprocess
import sys
import time

def run_cmd(cmd):
    return subprocess.run(cmd, shell=True, capture_output=True, text=True)

def main():
    print("=== Darling MediaPlayer MPRIS Integration Test ===")

    # 1. First run the self-contained in-guest unit test
    print("\n--- 1. Running in-guest mediaplayer_mpris unit suite ---")
    res = run_cmd("timeout -s KILL 30 darling shell /Volumes/SystemRoot/tmp/mediaplayer_mpris </dev/null")
    print(res.stdout)
    if res.returncode != 0:
        print(f"FAIL: mediaplayer_mpris exited with code {res.returncode}")
        print(res.stderr)
        sys.exit(1)
    if "Failures: 0" not in res.stdout:
        print("FAIL: unit test reported failures")
        sys.exit(1)
    print("PASS: In-guest unit suite passed cleanly.")

    # 2. Test single app MPRIS exposure and remote commands
    print("\n--- 2. Testing single app MPRIS exposure and remote control ---")
    ready_file = "/tmp/test_single_ready.txt"
    stop_file = "/tmp/test_single_stop.txt"
    for f in (ready_file, stop_file):
        if os.path.exists(f): os.unlink(f)

    guest_ready = "/Volumes/SystemRoot" + ready_file
    guest_stop = "/Volumes/SystemRoot" + stop_file

    proc = subprocess.Popen([
        "darling", "shell",
        "/Volumes/SystemRoot/tmp/test_mpris_player",
        guest_ready, guest_stop,
        "Single Track", "Single Artist"
    ], stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    try:
        # Wait for ready file
        for _ in range(50):
            if os.path.exists(ready_file):
                break
            time.sleep(0.1)
        else:
            print("FAIL: Player did not create ready file")
            sys.exit(1)

        time.sleep(1.0) # allow dbus async registration

        # Verify bus name
        res = run_cmd("busctl --user list")
        if "org.mpris.MediaPlayer2.darling.test_mpris_player" not in res.stdout:
            print("FAIL: Bus name not found on D-Bus!")
            print(res.stdout)
            sys.exit(1)
        print("PASS: Bus name org.mpris.MediaPlayer2.darling.test_mpris_player claimed.")

        # Verify playback status is Playing
        res = run_cmd("busctl --user get-property org.mpris.MediaPlayer2.darling.test_mpris_player /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player PlaybackStatus")
        print(f"PlaybackStatus: {res.stdout.strip()}")
        if 's "Playing"' not in res.stdout:
            print("FAIL: PlaybackStatus is not Playing")
            sys.exit(1)
        print("PASS: Initial PlaybackStatus is Playing.")

        # Verify Metadata title
        res = run_cmd("busctl --user get-property org.mpris.MediaPlayer2.darling.test_mpris_player /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player Metadata")
        if '"xesam:title" s "Single Track"' not in res.stdout or '"xesam:artist" as 1 "Single Artist"' not in res.stdout:
            print("FAIL: Metadata mismatch")
            print(res.stdout)
            sys.exit(1)
        print("PASS: Metadata verified (title, artist as array).")

        # Test Remote Command: Pause
        run_cmd("busctl --user call org.mpris.MediaPlayer2.darling.test_mpris_player /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player Pause")
        time.sleep(0.5)
        res = run_cmd("busctl --user get-property org.mpris.MediaPlayer2.darling.test_mpris_player /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player PlaybackStatus")
        if 's "Paused"' not in res.stdout:
            print("FAIL: Pause command did not update PlaybackStatus to Paused")
            print(res.stdout)
            sys.exit(1)
        print("PASS: Remote command Pause executed successfully.")

        # Test Remote Command: Play
        run_cmd("busctl --user call org.mpris.MediaPlayer2.darling.test_mpris_player /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player Play")
        time.sleep(0.5)
        res = run_cmd("busctl --user get-property org.mpris.MediaPlayer2.darling.test_mpris_player /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player PlaybackStatus")
        if 's "Playing"' not in res.stdout:
            print("FAIL: Play command did not update PlaybackStatus to Playing")
            print(res.stdout)
            sys.exit(1)
        print("PASS: Remote command Play executed successfully.")

        # Test Remote Command: SetPosition
        run_cmd('busctl --user call org.mpris.MediaPlayer2.darling.test_mpris_player /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player SetPosition "ox" /org/mpris/MediaPlayer2/Track/1 90000000')
        time.sleep(0.5)
        res = run_cmd("busctl --user get-property org.mpris.MediaPlayer2.darling.test_mpris_player /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player Position")
        print(f"Position: {res.stdout.strip()}")
        # Check position is around 90s (90000000 us)
        if " 90" not in res.stdout and " 91" not in res.stdout:
            print("FAIL: SetPosition did not update position to ~90s")
            sys.exit(1)
        print("PASS: SetPosition executed successfully.")

    finally:
        with open(stop_file, "w") as f:
            f.write("stop\n")
        proc.wait(timeout=5)
        for f in (ready_file, stop_file):
            if os.path.exists(f): os.unlink(f)

    # 3. Test active player switching between two apps
    print("\n--- 3. Testing active player switching between App A and App B ---")
    alpha_ready = "/tmp/alpha_r.txt"
    alpha_stop = "/tmp/alpha_s.txt"
    beta_ready = "/tmp/beta_r.txt"
    beta_stop = "/tmp/beta_s.txt"
    for f in (alpha_ready, alpha_stop, beta_ready, beta_stop):
        if os.path.exists(f): os.unlink(f)

    # Start App A
    proc_a = subprocess.Popen([
        "darling", "shell",
        "/Volumes/SystemRoot/tmp/player_alpha",
        "/Volumes/SystemRoot" + alpha_ready,
        "/Volumes/SystemRoot" + alpha_stop,
        "Track Alpha", "Artist Alpha"
    ], stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    try:
        for _ in range(50):
            if os.path.exists(alpha_ready): break
            time.sleep(0.1)
        time.sleep(1.0)

        res = run_cmd("busctl --user list")
        if "org.mpris.MediaPlayer2.darling.player_alpha" not in res.stdout:
            print("FAIL: player_alpha bus name not found")
            sys.exit(1)
        print("PASS: player_alpha is active player.")

        # Start App B (Playing)
        proc_b = subprocess.Popen([
            "darling", "shell",
            "/Volumes/SystemRoot/tmp/player_beta",
            "/Volumes/SystemRoot" + beta_ready,
            "/Volumes/SystemRoot" + beta_stop,
            "Track Beta", "Artist Beta"
        ], stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        try:
            for _ in range(50):
                if os.path.exists(beta_ready): break
                time.sleep(0.1)
            time.sleep(1.0)

            res = run_cmd("busctl --user list")
            if "org.mpris.MediaPlayer2.darling.player_beta" not in res.stdout:
                print("FAIL: player_beta did not become active player")
                sys.exit(1)
            if "org.mpris.MediaPlayer2.darling.player_alpha" in res.stdout:
                print("FAIL: player_alpha bus name was not released")
                sys.exit(1)
            print("PASS: player_beta took over as active player and player_alpha was released.")

            # Stop App B -> App A (which is still running and playing) becomes active again
            with open(beta_stop, "w") as f: f.write("stop\n")
            proc_b.wait(timeout=5)
            time.sleep(1.0)

            res = run_cmd("busctl --user list")
            if "org.mpris.MediaPlayer2.darling.player_alpha" not in res.stdout:
                print("FAIL: player_alpha did not resume as active player after player_beta stopped")
                print(res.stdout)
                sys.exit(1)
            print("PASS: player_alpha automatically resumed as active player after player_beta exited.")

        finally:
            if proc_b.poll() is None:
                with open(beta_stop, "w") as f: f.write("stop\n")
                proc_b.wait(timeout=5)

    finally:
        if proc_a.poll() is None:
            with open(alpha_stop, "w") as f: f.write("stop\n")
            proc_a.wait(timeout=5)
        for f in (alpha_ready, alpha_stop, beta_ready, beta_stop):
            if os.path.exists(f): os.unlink(f)

    print("\nALL IN-GUEST INTEGRATION TESTS PASSED!")

if __name__ == "__main__":
    main()
