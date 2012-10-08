#ifndef TOPOLOGYSORT_H
#define TOPOLOGYSORT_H
#include <map>
#include <set>
#include <vector>
#include <cassert>

// GNUStep ObjC runtime doesn't have "future classes", hence we need a TopologySort

template <typename T, typename GetDepCb>
void topology_visit(T* t, std::vector<T*>& elems, GetDepCb getDependency, std::set<T*>& visited)
{
	if (!visited.count(t))
	{
		visited.insert(t);
		auto deps = getDependency(t);

		for (T* d : deps)
			topology_visit(d, elems, getDependency, visited);

		elems.push_back(t);
	}	
}

template <typename T, typename GetDepCb>
void topology_sort(std::vector<T*>& elems, GetDepCb getDependency)
{
	std::set<T*> visited;
	std::vector<T*> input = std::move(elems);
	
	elems.clear();
	
	for (T* t : input)
		topology_visit(t, elems, getDependency, visited);
}

template <typename T, typename GetDepCb>
void topology_sort(const std::set<T*>& elems, std::vector<T*>& sorted, GetDepCb getDependency)
{
	std::set<T*> visited;
	for (T* t : elems)
		topology_visit(t, sorted, getDependency, visited);
}

#endif
