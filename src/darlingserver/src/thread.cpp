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

#include <darlingserver/thread.hpp>
#include <darlingserver/process.hpp>
#include <filesystem>
#include <fstream>

DarlingServer::Thread::Thread(std::shared_ptr<Process> process, NSID nsid):
	_nstid(nsid),
	_process(process)
{
	_tid = -1;

	for (const auto& entry: std::filesystem::directory_iterator("/proc/" + std::to_string(process->id()) + "/task")) {
		std::ifstream statusFile(entry.path() / "status");
		std::string line;

		while (std::getline(statusFile, line)) {
			if (line.substr(0, sizeof("NSpid") - 1) == "NSpid") {
				auto pos = line.find_last_of('\t');
				std::string id;

				if (pos != line.npos) {
					id = line.substr(pos + 1);
				}

				if (id.empty()) {
					throw std::runtime_error("Failed to parse thread ID");
				}

				if (std::stoi(id) != _nstid) {
					continue;
				}

				_tid = std::stoi(entry.path().filename().string());

				break;
			}
		}
	}

	if (_tid == -1) {
		throw std::runtime_error("Failed to find thread ID within darlingserver's namespace");
	}
};

void DarlingServer::Thread::registerWithProcess() {
	auto process = _process.lock();
	std::unique_lock lock(process->_rwlock);
	process->_threads.push_back(shared_from_this());
};

DarlingServer::Thread::~Thread() noexcept(false) {
	auto process = _process.lock();
	if (!process) {
		// the process is unregistering us
		return;
	}

	std::unique_lock lock(process->_rwlock);
	auto it = process->_threads.begin();
	while (it != process->_threads.end()) {
		if (auto thread = it->lock()) {
			if (thread.get() == this) {
				break;
			}
		}
	}
	if (it == process->_threads.end()) {
		throw std::runtime_error("Thread was not registered with Process");
	}
	process->_threads.erase(it);
};

DarlingServer::Thread::ID DarlingServer::Thread::id() const {
	return _tid;
};

DarlingServer::Thread::NSID DarlingServer::Thread::nsid() const {
	return _nstid;
};

std::shared_ptr<DarlingServer::Process> DarlingServer::Thread::process() const {
	return _process.lock();
};

std::shared_ptr<DarlingServer::Call> DarlingServer::Thread::pendingCall() const {
	std::shared_lock lock(_rwlock);
	return _pendingCall;
};

void DarlingServer::Thread::setPendingCall(std::shared_ptr<Call> newPendingCall) {
	std::unique_lock lock(_rwlock);
	_pendingCall = newPendingCall;
};

DarlingServer::Address DarlingServer::Thread::address() const {
	std::shared_lock lock(_rwlock);
	return _address;
};

void DarlingServer::Thread::setAddress(Address address) {
	std::unique_lock lock(_rwlock);
	_address = address;
};
