extern "C" int bazData;

class BazUser {
public:
	BazUser() { }
	~BazUser() { bazData = 0; }
};


BazUser b;


int bar()
{
	return bazData;
}

