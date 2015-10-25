
// TEST-OPTIONS: -arch x86_64 -framework Foundation
// TEST-OPTIONS: -arch x86_64 -framework Foundation -Wl,-no_compact_unwind


#include <Foundation/Foundation.h>

int foo() 
{
	@throw [NSException exceptionWithName:NSGenericException reason:@"many" userInfo:nil];
}


int main()
{
   NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

  	int state = 1;
	@try {
		state = 2;
		foo();
		state = 3;
	}
	@catch(id exception) {
		if ( state != 2 )
			return 1;
		if ( [[exception name] isEqualToString:NSGenericException] ) 
			state = 4;
	}

	if ( state == 4 )
		return 0;
	else
		return 1;
}

