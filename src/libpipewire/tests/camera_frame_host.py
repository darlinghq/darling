#!/usr/bin/env python3
# Host test for the camera frame converter. No PipeWire or CoreVideo headers
# are needed: the converter is a self-contained C module, so it can be compiled
# and exercised against known pixel values on the build host.
# usage: python3 src/libpipewire/tests/camera_frame_host.py [REPO_ROOT]

import os
import shutil
import subprocess
import sys
import tempfile

root = sys.argv[1] if len(sys.argv) > 1 else os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..'))

if shutil.which('cc') is None:
    print('SKIP: cc is not installed')
    sys.exit(0)

PROBE = r'''
#include "DarlingCameraFrame.h"
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static void packed_rgb(void)
{
    const uint8_t src[4 * 4] = {
        10, 20, 30, 255, 40, 50, 60, 255,
        70, 80, 90, 255, 100, 110, 120, 255,
    };
    const uint32_t ss = 8;
    uint8_t bgra[16] = {0}, rgba[16] = {0}, rgb24[12] = {0};
    const uint8_t *sp[4] = { src, NULL, NULL, NULL };
    const uint8_t *dp[4] = { NULL, NULL, NULL, NULL };
    uint32_t ss4[4] = { ss, 0, 0, 0 }, ds4[4] = { 8, 0, 0, 0 };

    dp[0] = bgra;
    CHECK(DarlingCameraConvertFrame(sp, ss4, DARLING_CAMERA_PIXEL_BGRX,
        dp, ds4, DARLING_CAMERA_OUTPUT_BGRA, 2, 2) == 0);
    CHECK(bgra[0] == 10 && bgra[1] == 20 && bgra[2] == 30 && bgra[3] == 255);
    CHECK(bgra[12] == 100 && bgra[14] == 120);

    dp[0] = rgba;
    CHECK(DarlingCameraConvertFrame(sp, ss4, DARLING_CAMERA_PIXEL_BGRX,
        dp, ds4, DARLING_CAMERA_OUTPUT_RGBA, 2, 2) == 0);
    CHECK(rgba[0] == 30 && rgba[1] == 20 && rgba[2] == 10 && rgba[3] == 255);

    ds4[0] = 6;
    dp[0] = rgb24;
    CHECK(DarlingCameraConvertFrame(sp, ss4, DARLING_CAMERA_PIXEL_BGRX,
        dp, ds4, DARLING_CAMERA_OUTPUT_RGB24, 2, 2) == 0);
    CHECK(rgb24[0] == 30 && rgb24[1] == 20 && rgb24[2] == 10);
    CHECK(rgb24[9] == 120 && rgb24[10] == 110 && rgb24[11] == 100);
}

static void planar_sources(void)
{
    const uint8_t y[8] = { 81, 81, 81, 81, 145, 145, 145, 145 };
    const uint8_t cb[2] = { 90, 90 }, cr[2] = { 240, 240 };
    const uint8_t uv[4] = { 90, 240, 90, 240 };
    const uint8_t sp_i420_planes[4][8] = {
        { 81, 81, 81, 81, 145, 145, 145, 145 }, { 90, 90, 0, 0, 0, 0, 0, 0 },
        { 240, 240, 0, 0, 0, 0, 0, 0 }, { 0 }
    };
    const uint8_t sp_nv12_planes[4][8] = {
        { 81, 81, 81, 81, 145, 145, 145, 145 }, { 90, 240, 90, 240, 0, 0, 0, 0 }, { 0 }, { 0 }
    };
    const uint8_t *pi[4] = { sp_i420_planes[0], sp_i420_planes[1], sp_i420_planes[2], NULL };
    const uint8_t *pn[4] = { sp_nv12_planes[0], sp_nv12_planes[1], NULL, NULL };
    const uint32_t si[4] = { 4, 2, 2, 0 }, sn[4] = { 4, 4, 0, 0 };
    const uint8_t *dp[4] = { NULL, NULL, NULL, NULL };
    uint32_t ds[4] = { 8, 0, 0, 0 };
    uint8_t out[16];

    (void) y; (void) cb; (void) cr; (void) uv;
    memset(out, 0, sizeof(out));
    dp[0] = out;
    CHECK(DarlingCameraConvertFrame(pi, si, DARLING_CAMERA_PIXEL_I420,
        dp, ds, DARLING_CAMERA_OUTPUT_BGRA, 2, 2) == 0);
    /* Studio-range BT.601: these chroma values are a saturated red. */
    CHECK(out[2] > 200 && out[1] < 40 && out[0] < 40);
    CHECK(out[10] >= out[2] && out[9] < 100 && out[8] < 100);

    memset(out, 0, sizeof(out));
    CHECK(DarlingCameraConvertFrame(pn, sn, DARLING_CAMERA_PIXEL_NV12,
        dp, ds, DARLING_CAMERA_OUTPUT_BGRA, 2, 2) == 0);
    CHECK(out[2] > 200 && out[1] < 40 && out[0] < 40);
    CHECK(out[10] >= out[2] && out[9] < 100 && out[8] < 100);
}

static void yuv_packed_and_planar_output(void)
{
    const uint8_t yuy2[8] = { 81, 90, 145, 240, 81, 90, 145, 240 };
    const uint8_t uyvy[8] = { 90, 81, 240, 145, 90, 81, 240, 145 };
    uint8_t input[8] = { 0 };
    const uint8_t *p[4] = { input, NULL, NULL, NULL };
    const uint32_t s[4] = { 4, 0, 0, 0 };
    uint8_t bgra[16] = {0}, outY[8] = {0}, outUV[8] = {0}, outV[8] = {0};
    const uint8_t *dp[4] = { NULL, NULL, NULL, NULL };
    uint32_t ds[4] = { 4, 2, 2, 0 };

    memcpy(input, yuy2, sizeof(yuy2));
    dp[0] = bgra;
    ds[0] = 8;
    CHECK(DarlingCameraConvertFrame(p, s, DARLING_CAMERA_PIXEL_YUY2,
        dp, ds, DARLING_CAMERA_OUTPUT_BGRA, 2, 2) == 0);
    CHECK(bgra[2] > 200 && bgra[1] < 40);

    memcpy(input, uyvy, sizeof(uyvy));
    memset(bgra, 0, sizeof(bgra));
    CHECK(DarlingCameraConvertFrame(p, s, DARLING_CAMERA_PIXEL_UYVY,
        dp, ds, DARLING_CAMERA_OUTPUT_BGRA, 2, 2) == 0);
    CHECK(bgra[2] > 200 && bgra[1] < 40);

    dp[0] = outY; dp[1] = outUV; dp[2] = outV;
    ds[0] = 4; ds[1] = 4; ds[2] = 4;
    CHECK(DarlingCameraConvertFrame(p, s, DARLING_CAMERA_PIXEL_UYVY,
        dp, ds, DARLING_CAMERA_OUTPUT_I420, 2, 2) == 0);
    CHECK(outY[0] >= 65 && outY[0] <= 100);
    CHECK(outUV[0] > 80 && outUV[0] < 100 && outV[0] > 230);

    memset(outUV, 0, sizeof(outUV));
    dp[1] = outUV;
    ds[1] = 4;
    CHECK(DarlingCameraConvertFrame(p, s, DARLING_CAMERA_PIXEL_UYVY,
        dp, ds, DARLING_CAMERA_OUTPUT_NV12, 2, 2) == 0);
    CHECK(outUV[0] > 80 && outUV[0] < 100 && outUV[1] > 230);

    /* 420f must use full-range coefficients, not the video-range values. */
    {
        const uint8_t red[8] = { 0, 0, 255, 255, 0, 0, 255, 255 };
        const uint8_t *rp[4] = { red, NULL, NULL, NULL };
        const uint32_t rs[4] = { 8, 0, 0, 0 };
        uint8_t fullY[8] = {0}, fullUV[8] = {0};
        dp[0] = fullY; dp[1] = fullUV; dp[2] = NULL;
        ds[0] = 4; ds[1] = 4; ds[2] = 0;
        CHECK(DarlingCameraConvertFrame(rp, rs, DARLING_CAMERA_PIXEL_BGRX,
            dp, ds, DARLING_CAMERA_OUTPUT_NV12_FULL, 2, 2) == 0);
        CHECK(fullY[0] == 77 && fullUV[0] == 85 && fullUV[1] == 255);
    }
}

static void rejects_bad_arguments(void)
{
    const uint8_t src[4] = { 0, 0, 0, 0 };
    const uint8_t *sp[4] = { src, NULL, NULL, NULL };
    const uint32_t ss[4] = { 4, 0, 0, 0 };
    const uint8_t *dp[4] = { src, NULL, NULL, NULL };
    const uint32_t ds[4] = { 4, 0, 0, 0 };

    CHECK(DarlingCameraConvertFrame(sp, ss, DARLING_CAMERA_PIXEL_BGRX,
        dp, ds, DARLING_CAMERA_OUTPUT_BGRA, 0, 2) == -1);
    CHECK(DarlingCameraConvertFrame(sp, ss, DARLING_CAMERA_PIXEL_I420,
        dp, ds, DARLING_CAMERA_OUTPUT_BGRA, 2, 2) == -1);
}

int main(void)
{
    packed_rgb();
    planar_sources();
    yuv_packed_and_planar_output();
    rejects_bad_arguments();
    if (failures)
        printf("%d camera frame checks failed\n", failures);
    else
        printf("camera frame conversion checks passed\n");
    return failures ? 1 : 0;
}
'''

with tempfile.TemporaryDirectory(prefix='darling-camera-frame-') as tmp:
    source = os.path.join(root, 'src', 'frameworks', 'AVFoundation', 'src', 'camera',
                          'DarlingCameraFrame.c')
    header = os.path.dirname(source)
    with open(os.path.join(tmp, 'probe.c'), 'w') as f:
        f.write(PROBE)
    build = subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-I', header, os.path.join(tmp, 'probe.c'), source,
                            '-o', os.path.join(tmp, 'probe')],
                           capture_output=True, text=True)
    if build.returncode != 0:
        sys.stdout.write(build.stdout + build.stderr)
        sys.exit(1)
    run = subprocess.run([os.path.join(tmp, 'probe')], capture_output=True, text=True)
    sys.stdout.write(run.stdout + run.stderr)
    sys.exit(run.returncode)
