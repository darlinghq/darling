

class test
{
public:
	virtual void aa() {}

	int		f;
};

class testsub : public test
{
public:
			testsub() : g(0) {}
	virtual void aa() {}

	int		g;
};


class testsubother : public test
{
public:
			testsubother() : h(0) {}
	virtual void aa() {}

	int		h;
};

extern test* maketestsub();

extern bool istestsub(test* t);