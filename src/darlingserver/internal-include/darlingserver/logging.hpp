/**
 * This file is part of Darling.
 *
 * Copyright (C) 2021 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DARLINGSERVER_LOGGING_HPP_
#define _DARLINGSERVER_LOGGING_HPP_

#include <fstream>
#include <string>
#include <sstream>

namespace DarlingServer {
	static struct EndLog {} endLog;

	class Log {
	private:
		std::string _category;

		enum class Type {
			Debug,
			Info,
			Warning,
			Error,
		};

		void _log(Type type, std::string message);
		static std::string _typeToString(Type type);

	public:
		Log(std::string category);

		Log(const Log&) = delete;
		Log& operator=(const Log&) = delete;
		Log(Log&&) = delete;
		Log& operator=(Log&&) = delete;

		class Stream;
		friend class Stream;

		Stream debug();
		Stream info();
		Stream warning();
		Stream error();
	};

	class Log::Stream {
		friend class Log;

	private:
		Type _type;
		Log& _log;
		std::ostringstream _buffer;

		Stream(Type type, Log& log);

	public:
		Stream(const Stream&) = delete;
		Stream& operator=(const Stream&) = delete;
		Stream(Stream&&) = delete;
		Stream& operator=(Stream&&) = delete;

		template<class T>
		Stream& operator<<(T value) {
			_buffer << value;
			return *this;
		};

		template<>
		Stream& operator<<<EndLog>(EndLog value);
	};
};

#endif // _DARLINGSERVER_LOGGING_HPP_
