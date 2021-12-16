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

#include <darlingserver/server.hpp>
#include <sys/socket.h>
#include <stdexcept>
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <system_error>
#include <thread>
#include <array>
#include <darlingserver/registry.hpp>

static DarlingServer::Server* sharedInstancePointer = nullptr;

DarlingServer::Server::Server(std::string prefix):
	_prefix(prefix),
	_socketPath(_prefix + "/var/run/darlingserver.sock"),
	_workQueue(std::bind(&Server::_worker, this, std::placeholders::_1))
{
	sharedInstancePointer = this;

	// remove the old socket (if it exists)
	unlink(_socketPath.c_str());

	// create the socket
	_listenerSocket = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (_listenerSocket < 0) {
		throw std::system_error(errno, std::generic_category(), "Failed to create socket");
	}

	int passCred = 1;
	if (setsockopt(_listenerSocket, SOL_SOCKET, SO_PASSCRED, &passCred, sizeof(passCred)) < 0) {
		throw std::system_error(errno, std::generic_category(), "Failed to set SO_PASSCRED on socket");
	}

	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';
	strncpy(addr.sun_path, _socketPath.c_str(), sizeof(addr.sun_path) - 1);

	if (bind(_listenerSocket, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		throw std::system_error(errno, std::generic_category(), "Failed to bind socket");
	}

	_epollFD = epoll_create1(EPOLL_CLOEXEC);
	if (_epollFD < 0) {
		throw std::system_error(errno, std::generic_category(), "Failed to create epoll context");
	}

	struct epoll_event settings;
	settings.data.ptr = this;
	settings.events = EPOLLIN | EPOLLOUT;

	if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, _listenerSocket, &settings) < 0) {
		throw std::system_error(errno, std::generic_category(), "Failed to add listener socket to epoll context");
	}
};

DarlingServer::Server::~Server() {
	close(_epollFD);
	close(_listenerSocket);
	unlink(_socketPath.c_str());
};

void DarlingServer::Server::start() {
	while (true) {
		struct epoll_event events[16];
		int ret = epoll_wait(_epollFD, events, 16, -1);

		if (ret < 0) {
			if (errno == EINTR) {
				continue;
			}

			throw std::system_error(errno, std::generic_category(), "Failed to wait on epoll context");
		}

		for (size_t i = 0; i < ret; ++i) {
			struct epoll_event* event = &events[i];

			if (event->data.ptr == this) {
				if (event->events & EPOLLIN) {
					_inbox.receiveMany(_listenerSocket);

					// TODO: receive messages directly onto the work queue
					while (auto msg = _inbox.pop()) {
						_workQueue.push(std::move(msg.value()));
					}
				}

				if (event->events & EPOLLOUT) {
					_outbox.sendMany(_listenerSocket);
				}
			} else if (event->events & EPOLLIN) {
				std::shared_ptr<Process>& process = *reinterpret_cast<std::shared_ptr<Process>*>(event->data.ptr);

				if (epoll_ctl(_epollFD, EPOLL_CTL_DEL, process->_pidfd, NULL) < 0) {
					throw std::system_error(errno, std::generic_category(), "Failed to remove process handle from epoll context");
				}

				process->_unregisterThreads();
				processRegistry().unregisterEntry(process);

				delete &process;
			}
		}
	}
};

void DarlingServer::Server::monitorProcess(std::shared_ptr<Process> process) {
	struct epoll_event settings;
	settings.data.ptr = new std::shared_ptr<Process>(process);
	settings.events = EPOLLIN;

	if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, process->_pidfd, &settings) < 0) {
		throw std::system_error(errno, std::generic_category(), "Failed to add process descriptor to epoll context");
	}
};

DarlingServer::Server& DarlingServer::Server::sharedInstance() {
	return *sharedInstancePointer;
};

std::string DarlingServer::Server::prefix() const {
	return _prefix;
};

void DarlingServer::Server::_worker(DarlingServer::Message message) {
	auto call = DarlingServer::Call::callFromMessage(std::move(message), _outbox);
	call->processCall();
};
