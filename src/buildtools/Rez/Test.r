/*#include "Types.r"


*/

#include "/home/wolfgang/Projects/Retro68/CExamples/Sample.r"

type 'TEST' {
	integer zero, one, two, answer = 42, missed;
	longint;
	integer = (after - before) / 8;
	integer = $$CountOf(foo);
	before:
	array foo {
		integer = $$ArrayIndex(foo);
		integer;
		integer;
	};
	string;
	after:
	;
};

resource 'TEST' (128) {
	answer,
	0x1234,
	{ 1, 2; 3, 4; },
	"Hello, "
	"world: "
	$"Abcd 1234";
};


