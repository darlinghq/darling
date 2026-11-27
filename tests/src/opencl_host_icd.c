/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * The core OpenCL API is forwarded to the host's ICD loader. Checks the
 * documented argument validation and, when the host has an OpenCL platform
 * installed, a platform/device/context/queue/buffer round trip through it.
 * Without any host platform the documented ICD loader result
 * (CL_PLATFORM_NOT_FOUND_KHR, zero platforms) is expected and the round trip
 * is skipped. The expected results come from the Khronos OpenCL 1.2
 * specification (platform layer, runtime and buffer objects).
 */
#include <OpenCL/OpenCL.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void check(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
}

int main(void)
{
	cl_platform_id platform = NULL;
	cl_uint count = 99;
	cl_int err;

	/* A non-zero pointer with zero entries, and no output at all, are invalid. */
	check("clGetPlatformIDs(0, ptr) is CL_INVALID_VALUE",
		clGetPlatformIDs(0, &platform, NULL) == CL_INVALID_VALUE);
	check("clGetPlatformIDs(0, NULL, NULL) is CL_INVALID_VALUE",
		clGetPlatformIDs(0, NULL, NULL) == CL_INVALID_VALUE);

	err = clGetPlatformIDs(0, NULL, &count);
	if (err == CL_PLATFORM_NOT_FOUND_KHR) {
		check("no host platform: zero platforms reported", count == 0);
		check("clGetDeviceIDs(NULL platform) is CL_INVALID_PLATFORM",
			clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 0, NULL, &count) == CL_INVALID_PLATFORM);
		printf("SKIP: no OpenCL platform on the host\n");
		return failures ? 1 : 0;
	}
	check("clGetPlatformIDs counts platforms", err == CL_SUCCESS && count >= 1);
	if (err != CL_SUCCESS || count < 1)
		return 1;

	check("clGetPlatformIDs returns a platform", clGetPlatformIDs(1, &platform, NULL) == CL_SUCCESS && platform != NULL);

	char text[256] = "";
	size_t length = 0;
	check("platform name size", clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &length) == CL_SUCCESS && length > 1);
	check("platform name", clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof text, text, &length) == CL_SUCCESS
		&& strlen(text) + 1 == length);
	check("platform version starts with OpenCL ",
		clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sizeof text, text, NULL) == CL_SUCCESS
		&& strncmp(text, "OpenCL ", 7) == 0);
	check("short platform info buffer is CL_INVALID_VALUE",
		clGetPlatformInfo(platform, CL_PLATFORM_NAME, 1, text, NULL) == CL_INVALID_VALUE);

	check("clGetDeviceIDs(invalid type) is CL_INVALID_DEVICE_TYPE",
		clGetDeviceIDs(platform, 0, 0, NULL, &count) == CL_INVALID_DEVICE_TYPE);
	check("clGetDeviceIDs(0 entries, ptr) is CL_INVALID_VALUE",
		clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, (cl_device_id[1]){ NULL }, NULL) == CL_INVALID_VALUE);

	cl_device_id device = NULL;
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &count);
	check("clGetDeviceIDs finds a device", err == CL_SUCCESS && count >= 1 && device != NULL);
	if (err != CL_SUCCESS)
		return 1;

	cl_device_type type = 0;
	check("device type", clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof type, &type, NULL) == CL_SUCCESS && type != 0);
	check("device name", clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof text, text, NULL) == CL_SUCCESS && text[0] != 0);

	check("clCreateContext(0 devices) is CL_INVALID_VALUE",
		(clCreateContext(NULL, 0, &device, NULL, NULL, &err) == NULL) && err == CL_INVALID_VALUE);

	cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	check("clCreateContext", context != NULL && err == CL_SUCCESS);
	if (context == NULL)
		return 1;

	cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
	check("clCreateCommandQueue", queue != NULL && err == CL_SUCCESS);

	cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, 64, NULL, &err);
	check("clCreateBuffer", buffer != NULL && err == CL_SUCCESS);
	check("clCreateBuffer(size 0) is CL_INVALID_BUFFER_SIZE",
		clCreateBuffer(context, CL_MEM_READ_WRITE, 0, NULL, &err) == NULL && err == CL_INVALID_BUFFER_SIZE);

	if (queue != NULL && buffer != NULL) {
		unsigned char in[64], out[64];
		for (int i = 0; i < 64; i++)
			in[i] = (unsigned char) (i * 7 + 3);
		memset(out, 0, sizeof out);
		check("blocking write", clEnqueueWriteBuffer(queue, buffer, CL_TRUE, 0, sizeof in, in, 0, NULL, NULL) == CL_SUCCESS);
		check("blocking read", clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, sizeof out, out, 0, NULL, NULL) == CL_SUCCESS);
		check("buffer contents round trip", memcmp(in, out, sizeof in) == 0);
		check("clFinish", clFinish(queue) == CL_SUCCESS);
	}

	if (buffer != NULL)
		check("clReleaseMemObject", clReleaseMemObject(buffer) == CL_SUCCESS);
	if (queue != NULL)
		check("clReleaseCommandQueue", clReleaseCommandQueue(queue) == CL_SUCCESS);
	check("clReleaseContext", clReleaseContext(context) == CL_SUCCESS);

	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures ? 1 : 0;
}
