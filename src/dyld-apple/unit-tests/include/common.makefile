# stuff to include in every test Makefile

SHELL = /bin/sh

# set default OS to be current Mac OS X 
OS_NAME ?= MacOSX
ifeq "$(OS_NAME)" "iPhoneOS"
	OS_VERSION ?= 3.1
	ifeq "$(findstring -$(OS_VERSION)-,-3.0-3.1-3.2-4.0-4.1-4.2-4.3-)" ""
		OS_LION_FEATURES = 1
	endif
	ARCH ?= armv7
	VALID_ARCHS ?= armv7
else
	OS_VERSION ?= 10.7
	ifeq "$(findstring -$(OS_VERSION)-,-10.4-10.5-10.6-)" ""
		OS_LION_FEATURES = 1
	endif
	ARCH ?= x86_64
	VALID_ARCHS ?= "i386 x86_64"
endif

IOSROOT	= 

ifeq "$(OS_NAME)" "iPhoneOS"
	#IOSROOT		= $(shell xcodebuild -version -sdk iphoneos.internal Path)
	IOSROOT		= /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS9.0.Internal.sdk
	CC			= $(shell xcrun -sdk iphoneos.internal -find cc)  -arch ${ARCH} -miphoneos-version-min=$(OS_VERSION) -isysroot $(IOSROOT)
	CXX			= $(shell xcrun -sdk iphoneos.internal -find c++) -arch ${ARCH} -miphoneos-version-min=$(OS_VERSION) -isysroot $(IOSROOT)
	LIPO		= $(shell xcrun -sdk iphoneos.internal -find lipo)
	STRIP		= $(shell xcrun -sdk iphoneos.internal -find strip)
	INSTALL_NAME_TOOL = $(shell xcrun -sdk iphoneos.internal -find install_name_tool)
else
  ifeq "$(OSX_SDK_ROOT)" ""
    OSX_SDK_ROOT		= $(shell xcodebuild -version -sdk macosx.internal Path)
  endif
	CC			= $(shell xcrun -find cc)  -arch ${ARCH} -mmacosx-version-min=$(OS_VERSION) -isysroot $(OSX_SDK_ROOT)
	CXX			= $(shell xcrun -find c++) -arch ${ARCH} -mmacosx-version-min=$(OS_VERSION) -isysroot $(OSX_SDK_ROOT)
	LIPO		= $(shell xcrun -find lipo)
	STRIP		= $(shell xcrun -find strip)
	INSTALL_NAME_TOOL = $(shell xcrun -find install_name_tool)
endif

CCFLAGS		= -Wall -std=c99
CXXFLAGS	= -Wall 

RM      = rm
RMFLAGS = -rf

SAFE_RUN	= ${TESTROOT}/bin/fail-if-non-zero.pl
PASS_IFF	= ${TESTROOT}/bin/pass-iff-exit-zero.pl
PASS_IFF_FAILURE = $(TESTROOT)/bin/exit-non-zero-pass.pl

ifeq ($(ARCH),armv7)
  CCFLAGS += -mno-thumb
  CXXFLAGS += -mno-thumb
  override FILEARCH = arm
else
  FILEARCH = $(ARCH)
endif

ifeq ($(ARCH),thumb)
  CCFLAGS += -mthumb
  CXXFLAGS += -mthumb
  override ARCH = armv6
  override FILEARCH = arm
else
  FILEARCH = $(ARCH)
endif

ifeq ($(ARCH),thumb2)
  CCFLAGS += -mthumb
  CXXFLAGS += -mthumb
  override ARCH = armv7
  override FILEARCH = arm
else
  FILEARCH = $(ARCH)
endif

