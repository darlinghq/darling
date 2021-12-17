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

#ifndef _DARLINGSERVER_SERVER_HPP_
#define _DARLINGSERVER_SERVER_HPP_

#include <string>
#include <sys/epoll.h>
#include <thread>

#include <darlingserver/message.hpp>
#include <darlingserver/workers.hpp>
#include <darlingserver/call.hpp>
#include <darlingserver/registry.hpp>

namespace DarlingServer {
	// NOTE: server instances MUST be created with `new` rather than as a normal local/stack variable
	class Server {
	private:
		int _listenerSocket;
		std::string _prefix;
		std::string _socketPath;
		int _epollFD;
		MessageQueue _inbox;
		MessageQueue _outbox;
		WorkQueue<Message> _workQueue;
		bool _canRead = false;
		bool _canWrite = true;
		int _wakeupFD;

		void _worker(DarlingServer::Message message);

	public:
		Server(std::string prefix);
		~Server();

		Server(const Server&) = delete;
		Server& operator=(const Server&) = delete;
		Server(Server&&) = delete;
		Server& operator=(Server&&) = delete;

		void start();

		void monitorProcess(std::shared_ptr<Process> process);

		std::string prefix() const;

		static Server& sharedInstance();
	};
};

#endif // _DARLINGSERVER_SERVER_HPP_
