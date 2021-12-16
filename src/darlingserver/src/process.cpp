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

#include <darlingserver/process.hpp>
#include <darlingserver/registry.hpp>
#include <sys/syscall.h>
#include <unistd.h>

DarlingServer::Process::Process(ID id, NSID nsid):
	_pid(id),
	_nspid(nsid)
{
	_pidfd = syscall(SYS_pidfd_open, _pid, 0);
	if (_pidfd < 0) {
		throw std::system_error(errno, std::generic_category(), "Failed to open pidfd for process");
	}
};

DarlingServer::Process::~Process() {
	close(_pidfd);

	_unregisterThreads();
};

void DarlingServer::Process::_unregisterThreads() {
	std::unique_lock lock(_rwlock);
	while (!_threads.empty()) {
		auto thread = _threads.front().lock();
		lock.unlock();
		if (thread) {
			thread->_process = std::weak_ptr<Process>();
			threadRegistry().unregisterEntry(thread);
		}
		lock.lock();
	}
};

DarlingServer::Process::ID DarlingServer::Process::id() const {
	return _pid;
};

DarlingServer::Process::NSID DarlingServer::Process::nsid() const {
	return _nspid;
};

std::vector<std::shared_ptr<DarlingServer::Thread>> DarlingServer::Process::threads() const {
	std::vector<std::shared_ptr<DarlingServer::Thread>> result;
	std::shared_lock lock(_rwlock);

	for (auto& maybeThread: _threads) {
		if (auto thread = maybeThread.lock()) {
			result.push_back(thread);
		}
	}

	return result;
};

std::string DarlingServer::Process::vchrootPath() const {
	std::shared_lock lock(_rwlock);
	return _vchrootPath;
};

void DarlingServer::Process::setVchrootPath(std::string path) {
	std::unique_lock lock(_rwlock);
	_vchrootPath = path;
};
