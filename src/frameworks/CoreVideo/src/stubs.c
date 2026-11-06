#include <stdio.h>

void* CVBufferRelease(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferCreateWithPlanarBytes(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}
void* CVPixelBufferGetBaseAddress(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferGetBaseAddressOfPlane(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferGetBytesPerRowOfPlane(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferGetHeight(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferGetIOSurface(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferGetPlaneCount(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferGetWidth(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferLockBaseAddress(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferUnlockBaseAddress(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVBufferCopyAttachment(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVBufferHasAttachment(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVBufferSetAttachment(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferCreateWithIOSurface(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferGetBytesPerRow(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferGetDataSize(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferGetHeightOfPlane(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferGetPixelFormatType(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferGetTypeID(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferIsPlanar(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferPoolCreate(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferPoolCreatePixelBuffer(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferPoolCreatePixelBufferWithAuxAttributes(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

void* CVPixelBufferPoolFlush(void)
{
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

struct CVStubRect { double x, y, w, h; };

struct CVStubRect CVImageBufferGetCleanRect(void* imageBuffer)
{
	struct CVStubRect r = { 0, 0, 0, 0 };
	printf("STUB: %s\n", __PRETTY_FUNCTION__);
	return r;
}
