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

#include "leb.h"
#include <stdint.h>
#include <stdexcept>

uint64_t uleb128(const uint8_t*& p)
{
	uint64_t r = 0;
	int s = 0;
	do
	{
		r |= (uint64_t)(*p & 0x7f) << s;
		s += 7;
	}
	while (*p++ >= 0x80);
	return r;
}

int64_t sleb128(const uint8_t*& p)
{
	int64_t r = 0;
	int s = 0;
	for (;;)
	{
		uint8_t b = *p++;
		if (b < 0x80)
		{
			if (b & 0x40)
				r -= (0x80 - b) << s;
			else
				r |= (b & 0x3f) << s;
			break;
		}
		r |= (b & 0x7f) << s;
		s += 7;
	}
	return r;
}

std::vector<uint8_t> uleb128(uint64_t v)
{
	if (!v)
		return std::vector<uint8_t>({ 0 });

	std::vector<uint8_t> rv;
	int bitsUsed = 0, bytesUsed;
	for (int i = 0; i < 64; i++)
	{
		if (v & (1ll << i))
			bitsUsed = i+1;
	}

	// round it up to 7 bits
	bytesUsed = (bitsUsed+6) / 7;

	for (int i = 0; i < bytesUsed; i++)
	{
		uint8_t enc = (v >> (i*7)) & 0x7f;
		if (i+1 < bytesUsed)
			enc |= 0x80;
		rv.push_back(enc);
	}

	return rv;
}


void LEBStream::add(uint64_t v)
{
	std::vector<uint8_t> enc = uleb128(v);
	m_data.insert(m_data.end(), enc.begin(), enc.end());
}

size_t LEBStream::count() const
{
	size_t count = 0;

	for (size_t i = 0; i < m_data.size(); i++)
	{
		if (!(m_data[i] & 0x80))
			count++;
	}

	return count;
}

uint64_t LEBStream::pop()
{
	if (!bytes())
		throw std::underflow_error("LEBStream::pop()");

	size_t start = 0;
	for (size_t i = 0; i < m_data.size()-1; i++)
	{
		if (!(m_data[i] & 0x80))
			start = i+1;
	}

	const uint8_t* data = &m_data[start];
	uint64_t v = uleb128(data);
	m_data.resize(start);
	return v;
}

LEBStreamIterator LEBStream::begin() const
{
	return LEBStreamIterator(data(), 0, bytes());
}

LEBStreamIterator LEBStream::end() const
{
	return LEBStreamIterator(data(), bytes(), bytes());
}

LEBStreamIterator::LEBStreamIterator(const uint8_t* data, size_t bytes, size_t pos)
: m_data(data), m_bytes(bytes), m_pos(pos)
{
}

uint64_t LEBStreamIterator::operator*()
{
	if (m_pos >= m_bytes)
		return 0;
	
	const uint8_t* data = m_data + m_pos;
	return uleb128(data);
}

LEBStreamIterator& LEBStreamIterator::operator++(int)
{
	while (m_data[m_pos] & 0x80 && m_pos < m_bytes)
		m_pos++;
	return *this;
}

bool LEBStreamIterator::operator!=(const LEBStreamIterator& that)
{
	return m_pos != that.m_pos;
}

