//
// This file is part of Darling.
//
// Copyright (C) 2026 Lubos Dolezel
//
// Darling is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Darling is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Darling.  If not, see <http://www.gnu.org/licenses/>.

// Typecheck test: the OpenCL types the Swift OpenCL overlay (clSetKernelArgsListAPPLE) uses, and
// their Swift spellings. Not run, only compiled:
//   swiftc -frontend -typecheck tests/src/opencl_import.swift -target x86_64-apple-macosx26.0 -sdk <SDK>

import OpenCL

// The call the overlay makes: va_list through a CVaListPointer.
func setArgs(_ kernel: cl_kernel, _ count: cl_uint, _ args: CVarArg...) -> cl_int {
    return withVaList(args) { clSetKernelArgsVaListAPPLE(kernel, count, $0) }
}

func scalars() -> Int {
    let a: cl_char = -1
    let b: cl_uchar = 255
    let c: cl_short = -2
    let d: cl_ushort = 3
    let e: cl_int = -4
    let f: cl_uint = 5
    let g: cl_long = -6
    let h: cl_ulong = 7
    let i: cl_half = 8
    let j: cl_float = 1.5
    let k: cl_double = 2.5
    let l: cl_bool = cl_bool(CL_TRUE)
    let m: cl_bitfield = 1 << 40
    _ = (a, b, c, d, e, f, g, h, i, j, k, l, m)
    return Int(e) + Int(CL_INVALID_KERNEL) + Int(CL_SUCCESS)
}

func vectors() -> Float {
    var v = cl_float4()
    v.x = 1
    v.s3 = 4
    let w = cl_int2()
    let x = cl_uchar16()
    let y = cl_double2()
    let z = cl_int4()
    // Vectors aligned to more than 16 bytes (such as cl_double4) are not imported by Swift.
    _ = (w, x, y, z)
    return v.x + v.s3
}

func handles(_ k: cl_kernel?, _ p: cl_platform_id?, _ d: cl_device_id?, _ c: cl_context?,
             _ q: cl_command_queue?, _ m: cl_mem?, _ g: cl_program?, _ e: cl_event?, _ s: cl_sampler?) -> cl_int {
    let format = cl_image_format(image_channel_order: 1, image_channel_data_type: 2)
    let region = cl_buffer_region(origin: 0, size: 16)
    var desc = cl_image_desc()
    desc.image_width = 4
    _ = (p, d, c, q, m, g, e, s, format, region, desc)
    let flags: cl_mem_flags = 1
    let device: cl_device_type = 2
    _ = (flags, device)
    guard let kernel = k else { return CL_INVALID_KERNEL }
    var value: Int32 = 1
    return clSetKernelArg(kernel, 0, 4, &value)
}
