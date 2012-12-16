/*
This file is part of Darling.

Copyright (C) 2012 Lubos Dolezel
Copyright (C) 2011 Shinichiro Hamaji

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LEB_H
#define LEB_H
#include <stdint.h>
#include <vector>
#include <stddef.h>

std::vector<uint8_t> uleb128(uint64_t v);
uint64_t uleb128(const uint8_t*& p);

int64_t sleb128(const uint8_t*& p);

class LEBStreamIterator;

class LEBStream
{
public:
	LEBStream();
	void add(uint64_t v);

	// log(N)!
	size_t count() const;

	// log(1)
	inline size_t bytes() const { return m_data.size(); }
	inline const uint8_t* data() const { return &m_data[0]; }

	// reverse of add()
	uint64_t pop();
	
	LEBStreamIterator begin() const;
	LEBStreamIterator end() const;
private:
	std::vector<uint8_t> m_data;
};

class LEBStreamIterator
{
protected:
	LEBStreamIterator(const uint8_t* data, size_t bytes, size_t pos = 0);
public:
	uint64_t operator*();
	LEBStreamIterator& operator++(int);
	bool operator!=(const LEBStreamIterator& that);
private:
	const uint8_t* m_data;
	size_t m_bytes, m_pos;
	
	friend class LEBStream;
};

#endif
