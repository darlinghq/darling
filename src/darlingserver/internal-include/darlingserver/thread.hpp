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

#ifndef _DARLINGSERVER_THREAD_HPP_
#define _DARLINGSERVER_THREAD_HPP_

#include <memory>
#include <sys/types.h>
#include <mutex>
#include <shared_mutex>

#include <darlingserver/message.hpp>

namespace DarlingServer {
	class Process;
	class Call;

	class Thread: public std::enable_shared_from_this<Thread> {
		friend class Process;

	private:
		pid_t _tid;
		pid_t _nstid;
		std::weak_ptr<Process> _process;
		std::shared_ptr<Call> _pendingCall;
		Address _address;
		mutable std::shared_mutex _rwlock;

	public:
		using ID = pid_t;
		using NSID = ID;

		Thread(std::shared_ptr<Process> process, NSID nsid);
		~Thread() noexcept(false);

		void registerWithProcess();

		Thread(const Thread&) = delete;
		Thread& operator=(const Thread&) = delete;
		Thread(Thread&&) = delete;
		Thread& operator=(Thread&&) = delete;

		std::shared_ptr<Process> process() const;

		std::shared_ptr<Call> pendingCall() const;
		void setPendingCall(std::shared_ptr<Call> newPendingCall);

		/**
		 * The TID of this Thread as seen from darlingserver's namespace.
		 */
		ID id() const;

		/**
		 * The TID of this Thread as seen from within the container (i.e. launchd's namespace).
		 */
		NSID nsid() const;

		Address address() const;
		void setAddress(Address address);
	};
};

#endif // _DARLINGSERVER_THREAD_HPP_
