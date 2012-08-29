#ifndef STLUTILS_H
#define STLUTILS_H
#include <string>
#include <map>
#include <cassert>

bool string_startsWith(const std::string& str, const std::string& what)
{
	if (str.size() < what.size())
		return false;
	else
		return str.compare(0, what.size(), what) == 0;
}

bool string_endsWith(const std::string& str, const std::string& what)
{
	if (str.size() < what.size())
        return false;
    else
		return str.compare(str.size()-what.size(), what.size(), what) == 0;
}

template<typename K, typename V, typename KK> bool map_contains(const std::map<K,V>& map, const KK& k)
{
	return map.find(k) != map.end();
}

template<typename K, typename V, typename KK> const V& map_get(const std::map<K,V>& map, const KK& k)
{
	auto it = map.find(k);
	assert(it != map.end());
	return it->second;
}

#endif

