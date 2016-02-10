// Define byte offset of key within context structure.
#define	ContextKey			0

/*	Define byte offset of key length within context structure.  The number
	stored there is the number of bytes from the start of the first round key
	to the start of the last round key.  That is 16 less than the number of
	bytes in the entire key.
*/
#define	ContextKeyLength	240
