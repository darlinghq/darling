#include "TopologySort.h"
#include <iostream>
#include <cassert>

using namespace std;

struct Obj
{
	Obj(int n) : num(n), done(false) {}
	int num;
	bool done;
	set<Obj*> deps;
};

int main()
{
	vector<Obj*> objs;

	for (size_t i = 0; i < 5; i++)
		objs.push_back(new Obj(i));

	objs[1]->deps.insert(objs[0]);
	objs[1]->deps.insert(objs[2]);
	objs[0]->deps.insert(objs[3]);
	objs[4]->deps.insert(objs[1]);

	topology_sort<Obj>(objs, [](Obj* o) { return o->deps; });

	for (Obj* o : objs)
	{
		o->done = true;
		for (Obj* d : o->deps)
			assert(d->done);
		std::cout << o->num << std::endl;
	}

	return 0;
}

