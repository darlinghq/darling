/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * @APPLE_FREE_COPYRIGHT@
 */

#ifndef _VIDEO_CONSOLE_H_
#define _VIDEO_CONSOLE_H_

#include <device/device_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define kVCSysctlProgressOptions      "kern.progressoptions"
#define kVCSysctlConsoleOptions       "kern.consoleoptions"
#define kVCSysctlProgressMeterEnable  "kern.progressmeterenable"
#define kVCSysctlProgressMeter        "kern.progressmeter"

enum{
	kVCDarkReboot       = 0x00000001,
	kVCAcquireImmediate = 0x00000002,
	kVCUsePosition      = 0x00000004,
	kVCDarkBackground   = 0x00000008,
	kVCLightBackground  = 0x00000010,
};

enum {
	kDataRotate0   = 0,
	kDataRotate90  = 1,
	kDataRotate180 = 2,
	kDataRotate270 = 3
};

struct vc_progress_user_options {
	uint32_t options;
	// fractional position of middle of spinner 0 (0.0) - 0xFFFFFFFF (1.0)
	uint32_t x_pos;
	uint32_t y_pos;
	uint32_t resv[8];
};
typedef struct vc_progress_user_options vc_progress_user_options;


#if XNU_KERNEL_PRIVATE

void vcputc(int, int, int);

int vcgetc(     int             l,
    int             u,
    boolean_t       wait,
    boolean_t       raw );

void video_scroll_up(   void    *start,
    void    *end,
    void    *dest );

void video_scroll_down( void    *start,  /* HIGH addr */
    void    *end,                        /* LOW addr */
    void    *dest );                     /* HIGH addr */

struct vc_info {
	unsigned int    v_height;       /* pixels */
	unsigned int    v_width;        /* pixels */
	unsigned int    v_depth;
	unsigned int    v_rowbytes;
	unsigned long   v_baseaddr;
	unsigned int    v_type;
	char            v_name[32];
	uint64_t        v_physaddr;
	unsigned int    v_rows;         /* characters */
	unsigned int    v_columns;      /* characters */
	unsigned int    v_rowscanbytes; /* Actualy number of bytes used for display per row*/
	unsigned int    v_scale;
	unsigned int    v_rotate;
	unsigned int    v_reserved[3];
};

struct vc_progress_element {
	unsigned int        version;
	unsigned int        flags;
	unsigned int        time;
	unsigned char       count;
	unsigned char       res[3];
	int                 width;
	int                 height;
	int                 dx;
	int                 dy;
	int                 transparent;
	unsigned int        res2[3];
};
typedef struct vc_progress_element vc_progress_element;

extern struct vc_progress_user_options vc_user_options;

void vc_progress_initialize( vc_progress_element * desc,
    const unsigned char * data1x,
    const unsigned char * data2x,
    const unsigned char * data3x,
    const unsigned char * clut );

void vc_progress_set(boolean_t enable, uint32_t vc_delay);

void vc_display_icon( vc_progress_element * desc, const unsigned char * data );

int vc_display_lzss_icon(uint32_t dst_x, uint32_t dst_y,
    uint32_t image_width, uint32_t image_height,
    const uint8_t *compressed_image,
    uint32_t       compressed_size,
    const uint8_t *clut);

#if !CONFIG_EMBEDDED

extern void vc_enable_progressmeter(int new_value);
extern void vc_set_progressmeter(int new_value);
extern int vc_progressmeter_enable;
extern int vc_progressmeter_value;
extern void vc_progress_setdiskspeed(uint32_t speed);

#endif /* !CONFIG_EMBEDDED */

#endif /* XNU_KERNEL_PRIVATE */

#ifdef __cplusplus
}
#endif

#endif /* _VIDEO_CONSOLE_H_ */
