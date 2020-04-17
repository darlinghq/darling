/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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

#ifndef _CA_CONSUMABLE_BUFFER_H
#define _CA_CONSUMABLE_BUFFER_H
#include <vector>
#include <stdexcept>
#include <cstring>
#include <stdint.h>

class ConsumableBuffer
{
public:
	void push(const void* mem, size_t bytes)
	{
		const uint8_t* ptr = static_cast<const uint8_t*>(mem);
		m_data.insert(m_data.end(), ptr, ptr + bytes);
	}
	size_t size() const
	{
		return m_data.size();
	}
	const uint8_t* data()
	{
		return m_data.data();
	}
	void consume(size_t bytes)
	{
		if (bytes > size())
			throw std::logic_error("ConsumableBuffer::consume() bytes > size()");

		std::memmove(m_data.data(), m_data.data() + bytes, m_data.size() - bytes);
		m_data.resize(m_data.size() - bytes);
	}
private:
	std::vector<uint8_t> m_data;
};

#endif
