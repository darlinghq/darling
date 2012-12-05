#ifndef STLUTILS_H
#define STLUTILS_H
#include <string>
#include <map>
#include <cassert>
#include <vector>

bool string_startsWith(const std::string& str, const std::string& what);
bool string_endsWith(const std::string& str, const std::string& what);
std::vector<std::string> string_explode(const std::string& str, char delim, bool keepEmpty = true);

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

