#!/usr/bin/env python3
# Host test for the BGRA conversion used by every screen-capture consumer.
# usage: python3 src/libpipewire/tests/screen_capture_conversion_host.py [REPO_ROOT]

import os
import shutil
import subprocess
import sys
import tempfile

root = sys.argv[1] if len(sys.argv) > 1 else os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..'))

if shutil.which("cc") is None or shutil.which("pkg-config") is None:
    print("SKIP: a C compiler and pkg-config are required")
    sys.exit(0)

PROBE = r"""
#include "screen_capture_backend.h"
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static void fill(struct darling_screen_capture_frame *frame,
                 enum darling_pw_video_format format, uint32_t width, uint32_t height,
                 uint32_t stride, void *pixels)
{
    memset(frame, 0, sizeof(*frame));
    frame->format = format;
    frame->width = width;
    frame->height = height;
    frame->planes[0] = pixels;
    frame->strides[0] = stride;
    frame->sizes[0] = stride * height;
}

int main(void)
{
    unsigned char bgrx[16] = {
        0x11, 0x22, 0x33, 0xff, 0x44, 0x55, 0x66, 0xff,
        0x77, 0x88, 0x99, 0xff, 0xaa, 0xbb, 0xcc, 0xff,
    };
    unsigned char rgbx[16] = {
        0x33, 0x22, 0x11, 0xff, 0x66, 0x55, 0x44, 0xff,
        0x99, 0x88, 0x77, 0xff, 0xcc, 0xbb, 0xaa, 0xff,
    };
    unsigned char xrgb[16] = {
        0xff, 0x11, 0x22, 0x33, 0xff, 0x44, 0x55, 0x66,
        0xff, 0x77, 0x88, 0x99, 0xff, 0xaa, 0xbb, 0xcc,
    };
    unsigned char bgr24[12] = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
        0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
    };
    unsigned char out[64];
    struct darling_screen_capture_frame frame;
    uint32_t i;

    fill(&frame, DARLING_PW_VIDEO_FORMAT_BGRX, 2, 2, 8, bgrx);
    memset(out, 0, sizeof(out));
    CHECK(darling_screen_capture_convert_bgra(&frame, out, 8, 2, 2) == 0);
    CHECK(memcmp(out, bgrx, 16) == 0);

    fill(&frame, DARLING_PW_VIDEO_FORMAT_RGBX, 2, 2, 8, rgbx);
    CHECK(darling_screen_capture_convert_bgra(&frame, out, 8, 2, 2) == 0);
    CHECK(out[0] == 0x11 && out[1] == 0x22 && out[2] == 0x33);
    CHECK(out[4] == 0x44 && out[5] == 0x55 && out[6] == 0x66);

    fill(&frame, DARLING_PW_VIDEO_FORMAT_XRGB, 2, 2, 8, xrgb);
    CHECK(darling_screen_capture_convert_bgra(&frame, out, 8, 2, 2) == 0);
    CHECK(out[0] == 0x33 && out[1] == 0x22 && out[2] == 0x11);

    fill(&frame, DARLING_PW_VIDEO_FORMAT_BGR24, 2, 2, 6, bgr24);
    CHECK(darling_screen_capture_convert_bgra(&frame, out, 8, 2, 2) == 0);
    for (i = 0; i < 2; i++) {
        CHECK(out[i * 8 + 0] == bgr24[i * 6 + 0]);
        CHECK(out[i * 8 + 1] == bgr24[i * 6 + 1]);
        CHECK(out[i * 8 + 2] == bgr24[i * 6 + 2]);
        CHECK(out[i * 8 + 3] == 255);
    }

    /* Nearest-neighbour scaling repeats source rows and columns. */
    fill(&frame, DARLING_PW_VIDEO_FORMAT_BGRX, 2, 2, 8, bgrx);
    memset(out, 0xff, sizeof(out));
    CHECK(darling_screen_capture_convert_bgra(&frame, out, 16, 4, 4) == 0);
    for (i = 0; i < 16; i++) {
        uint32_t dy = i / 4, dx = i % 4;
        uint32_t sy = dy / 2, sx = dx / 2;
        CHECK(memcmp(out + dy * 16 + dx * 4, bgrx + sy * 8 + sx * 4, 4) == 0);
    }

    fill(&frame, DARLING_PW_VIDEO_FORMAT_I420, 2, 2, 8, bgrx);
    CHECK(darling_screen_capture_convert_bgra(&frame, out, 8, 2, 2) < 0);
    frame.strides[0] = 4;
    CHECK(darling_screen_capture_convert_bgra(&frame, out, 8, 2, 2) < 0);
    CHECK(darling_screen_capture_convert_bgra(NULL, out, 8, 2, 2) < 0);
    CHECK(darling_screen_capture_convert_bgra(&frame, NULL, 8, 2, 2) < 0);

    if (failures == 0)
        puts("PASS: screen capture BGRA conversion");
    return failures == 0 ? 0 : 1;
}
"""

with tempfile.TemporaryDirectory(prefix="screen-capture-conversion-") as tmp:
    source = os.path.join(tmp, "probe.c")
    with open(source, "w") as f:
        f.write(PROBE)
    flags = subprocess.run(
        ["pkg-config", "--cflags", "--libs", "libpipewire-0.3", "dbus-1"],
        capture_output=True, text=True)
    if flags.returncode != 0:
        print("SKIP: PipeWire and D-Bus development files are not installed")
        sys.exit(0)
    build = subprocess.run([
        "cc", "-std=gnu11", "-Wall", "-D_GNU_SOURCE",
        f"-I{os.path.join(root, 'src/libpipewire/include')}",
        f"-I{os.path.join(root, 'src/libpipewire')}",
        f"-I{os.path.join(root, 'src/libdarlingdbus/include')}",
        source,
        os.path.join(root, "src/libpipewire/screen_capture_backend.c"),
        os.path.join(root, "src/libpipewire/screencast_portal.c"),
        os.path.join(root, "src/libpipewire/portal_common.c"),
        os.path.join(root, "src/libpipewire/pipe_client.c"),
        os.path.join(root, "src/libdarlingdbus/darling_dbus.c"),
        *flags.stdout.split(), "-ldl", "-lpthread",
        "-o", os.path.join(tmp, "probe"),
    ], capture_output=True, text=True)
    if build.returncode != 0:
        print(build.stdout + build.stderr)
        sys.exit("FAIL: the screen capture conversion probe does not build")
    run = subprocess.run([os.path.join(tmp, "probe")], capture_output=True, text=True, timeout=30)
    sys.stdout.write(run.stdout + run.stderr)
    if run.returncode != 0:
        sys.exit("FAIL: screen capture conversion regression")
