#include "property.h"
#include "../libobjc/class.h" // For a bug workaround only
#include "../libobjc/properties.h" // ditto

// Fixed in GNUstep r35658
void bug_gnustepFixPropertyCount(Class c)
{
	if (c->properties->count == 0)
		c->properties->count = 1;
}
