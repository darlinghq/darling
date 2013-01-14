#ifndef ASLMSG_H
#define ASLMSG_H
#include <map>
#include <string>

class ASLMsg
{
public:
	ASLMsg(int type);

	const char* get(const char* key) const;
	const char* get(int index) const;
	bool set(const char* key, const char* value);
	bool set(const char* key, int op, const char* value);
	bool unset(const char* key);

	int type() const { return m_type; }
private:
	struct MapValue
	{
		std::string value;
		int op;
	};
	std::map<std::string, MapValue> m_pairs;
	int m_type;
};

#endif

