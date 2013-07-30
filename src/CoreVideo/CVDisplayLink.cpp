#include "CVDisplayLink.h"
#include <util/debug.h>

CVDisplayLink::CVDisplayLink()
: m_bRunning(false)
{
	STUB();
}

CVDisplayLink::~CVDisplayLink()
{
}

CVReturn CVDisplayLink::start()
{
	STUB();
	if (m_bRunning)
		return kCVReturnDisplayLinkAlreadyRunning;

	m_bRunning = true;

	return kCVReturnSuccess;
}

CVReturn CVDisplayLink::stop()
{
	STUB();

	if (!m_bRunning)
		return kCVReturnDisplayLinkNotRunning;

	m_bRunning = false;

	return kCVReturnSuccess;
}

bool CVDisplayLink::isRunning()
{
	return m_bRunning;
}

CVReturn CVDisplayLink::setDisplay(CGLContextObj cglContext, CGLPixelFormatObj cglPixelFormat)
{
	STUB();

	return kCVReturnError;
}

