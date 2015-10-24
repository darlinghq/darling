
// TEST-OPTIONS: -arch x86_64 -framework Foundation
// TEST-OPTIONS: -arch x86_64 -framework Foundation -Wl,-no_compact_unwind


#include <Foundation/Foundation.h>

// private SPI for AppKit
extern int _NSAddAltHandler2(void (*proc)(NSException* exc, void* context), void* context);


int foo() 
{
	@throw [NSException exceptionWithName:NSGenericException reason:@"many" userInfo:nil];
}

static void handler(NSException* exc, void* context)
{
	int* paltState = (int*)context;
	*paltState = 1;
}


int main()
{
   NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

  	int state = 1;
	int altState = 0;
	@try {
		_NSAddAltHandler2(handler, &altState);
		state = 2;
		foo();
		state = 3;
	}
	@catch(id exception) {
		if ( state != 2 )
			return 1;
		if ( altState == 0 )
			return 1;
		if ( [[exception name] isEqualToString:NSGenericException] ) 
			state = 4;
	}

	if ( state == 4 )
		return 0;
	else
		return 1;
}

