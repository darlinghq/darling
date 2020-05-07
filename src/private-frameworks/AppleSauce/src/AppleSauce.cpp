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

#include <AppleSauce/AppleSauce.h>
#include <iostream>

static bool verbose = false;

__attribute__((constructor))
static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
};

std::string applesauce::xpc::object::to_string() const {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return std::string();
};

applesauce::xpc::dict const& applesauce::xpc::auto_reply::create_from_dict(dict const& dictionary, connection const&) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return dictionary;
};

applesauce::xpc::dict const& applesauce::xpc::auto_reply::create_from_reply(dict const& reply) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return reply;
};

applesauce::xpc::dict const& applesauce::xpc::auto_reply::create_from_request(dict const& request) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return request;
};

char const* applesauce::xpc::dyn_cast_or_default(object const&, char const* def) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return def;
};

std::string applesauce::xpc::dyn_cast_or_default(object const&, std::string const& def) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return def;
};

std::vector<uint8_t> applesauce::xpc::dyn_cast_or_default(object const&, std::vector<uint8_t> const& def) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return def;
};

bool applesauce::xpc::dyn_cast_or_default(object const&, bool def) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return def;
};

double applesauce::xpc::dyn_cast_or_default(object const&, double def) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return def;
};

int applesauce::xpc::dyn_cast_or_default(object const&, int def) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return def;
};

unsigned int applesauce::xpc::dyn_cast_or_default(object const&, unsigned int def) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return def;
};

long applesauce::xpc::dyn_cast_or_default(object const&, long def) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return def;
};

unsigned long applesauce::xpc::dyn_cast_or_default(object const&, unsigned long def) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return def;
};

long long applesauce::xpc::dyn_cast_or_default(object const&, long long def) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return def;
};

unsigned long long applesauce::xpc::dyn_cast_or_default(object const&, unsigned long long def) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return def;
};

dispatch_time_t applesauce::dispatch::v1::walltime(std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<long long, std::micro>>) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return 0;
};

std::string const& docopt::docopt_parse(std::string const& str, std::vector<std::string> const&, bool, bool, bool) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return str;
};

std::string const& docopt::docopt(std::string const& str, std::vector<std::string> const&, bool, std::string const&, bool) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return str;
};

std::ostream& docopt::operator<<(std::ostream& stream, docopt::value const&) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return stream;
};
