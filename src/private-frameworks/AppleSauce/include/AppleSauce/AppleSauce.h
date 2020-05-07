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

#ifndef _AppleSauce_H_
#define _AppleSauce_H_

#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <xpc/xpc.h>
#include <dispatch/dispatch.h>

/**
 * NOTE(@facekapow):
 * these method signatures are pretty much guesses.
 * good guesses, but guesses nonetheless
 */

namespace applesauce {
	namespace xpc {
		class object {
		public:
			std::string to_string() const;
		};
		class dict {};
		class connection {};

		char const* dyn_cast_or_default(object const&, char const*);
		std::string dyn_cast_or_default(object const&, std::string const&);
		std::vector<uint8_t> dyn_cast_or_default(object const&, std::vector<uint8_t> const&);
		bool dyn_cast_or_default(object const&, bool);
		double dyn_cast_or_default(object const&, double);
		int dyn_cast_or_default(object const&, int);
		unsigned int dyn_cast_or_default(object const&, unsigned int);
		long dyn_cast_or_default(object const&, long);
		unsigned long dyn_cast_or_default(object const&, unsigned long);
		long long dyn_cast_or_default(object const&, long long);
		unsigned long long dyn_cast_or_default(object const&, unsigned long long);

		namespace auto_reply {
			dict const& create_from_dict(dict const&, connection const&);
			dict const& create_from_reply(dict const&);
			dict const& create_from_request(dict const&);
		};
	};
	namespace dispatch {
		namespace v1 {
			class queue {};

			dispatch_time_t walltime(std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<long long, std::micro>>);
		};
	};
};

namespace docopt {
	class value {};

	std::string const& docopt_parse(std::string const&, std::vector<std::string> const&, bool, bool, bool);
	std::string const& docopt(std::string const&, std::vector<std::string> const&, bool, std::string const&, bool);
	std::ostream& operator<<(std::ostream&, value const&);
};

#endif
