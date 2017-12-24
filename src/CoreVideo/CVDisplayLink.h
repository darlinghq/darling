#ifndef CVDISPLAYLINK_H
#define CVDISPLAYLINK_H
#include "CoreVideo.h"

class CVDisplayLink
{
public:
	CVDisplayLink();
	virtual ~CVDisplayLink();

	CVReturn start();
	CVReturn stop();
	bool isRunning();

	CVReturn setDisplay(CGLContextObj cglContext, CGLPixelFormatObj cglPixelFormat);
private:
	bool m_bRunning;
};

#endif

