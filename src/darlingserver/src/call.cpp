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

#define _GNU_SOURCE 1
#include <darlingserver/call.hpp>
#include <darlingserver/server.hpp>
#include <sys/uio.h>

std::shared_ptr<DarlingServer::Call> DarlingServer::Call::callFromMessage(Message&& requestMessage, MessageQueue& replyQueue) {
	if (requestMessage.data().size() < sizeof(dserver_rpc_callhdr_t)) {
		throw std::invalid_argument("Message buffer was too small for call header");
	}

	dserver_rpc_callhdr_t* header = reinterpret_cast<dserver_rpc_callhdr_t*>(requestMessage.data().data());
	std::shared_ptr<Call> result = nullptr;
	std::shared_ptr<Process> process = nullptr;
	std::shared_ptr<Thread> thread = nullptr;

	// first, make sure we know this call number
	switch (header->number) {
		DSERVER_VALID_CALLNUM_CASES
			break;

		default:
			throw std::invalid_argument("Invalid call number");
	}

	// now let's lookup (and possibly create) the process and thread making this call
	process = processRegistry().registerIfAbsent(header->pid, [&]() {
		auto tmp = std::make_shared<Process>(requestMessage.pid(), header->pid);
		Server::sharedInstance().monitorProcess(tmp);
		return tmp;
	});
	thread = threadRegistry().registerIfAbsent(header->tid, [&]() {
		auto tmp = std::make_shared<Thread>(process, header->tid);
		tmp->setAddress(requestMessage.address());
		tmp->registerWithProcess();
		return tmp;
	});

	// finally, let's construct the call class

	#define CALL_CASE(_callName, _className) \
		case dserver_callnum_ ## _callName: { \
			if (requestMessage.data().size() < sizeof(dserver_rpc_call_ ## _callName ## _t)) { \
				throw std::invalid_argument("Message buffer was too small for dserver_call_" #_callName "_t"); \
			} \
			result = std::make_shared<_className>(replyQueue, thread, reinterpret_cast<dserver_rpc_call_ ## _callName ## _t*>(header), std::move(requestMessage)); \
		} break;

	switch (header->number) {
		DSERVER_CONSTRUCT_CASES

		default:
			throw std::invalid_argument("Invalid call number");
	}

	#undef CALL_CASE

	thread->setPendingCall(result);

	return result;
};

DarlingServer::Call::Call(MessageQueue& replyQueue, std::shared_ptr<Thread> thread, Address replyAddress):
	_replyQueue(replyQueue),
	_thread(thread),
	_replyAddress(replyAddress)
	{};

DarlingServer::Call::~Call() {};

/**
 * Note that you MUST NOT have any local variables referencing `this` when this method is called.
 *
 * This Call object MAY be destroyed upon the return of this method.
 */
void DarlingServer::Call::_stopPending() {
	// we're done processing this call; dump it
	if (auto thread = _thread.lock()) {
		thread->setPendingCall(nullptr);
	}
};

void DarlingServer::Call::Checkin::processCall() {
	// the Call creation already took care of registering the process and thread
	_sendReply(0);
	_stopPending();
};

void DarlingServer::Call::Checkout::processCall() {
	int code = 0;

	if (auto thread = _thread.lock()) {
		if (auto process = thread->process()) {
			threadRegistry().unregisterEntry(thread);

			if (thread->id() == process->id()) {
				processRegistry().unregisterEntry(process);
			}
		} else {
			code = -ESRCH;
		}
	} else {
		code = -ESRCH;
	}

	_sendReply(code);
	_stopPending();
};

void DarlingServer::Call::VchrootPath::processCall() {
	int code = 0;
	size_t fullLength = 0;

	if (auto thread = _thread.lock()) {
		if (auto process = thread->process()) {
			if (_body.buffer_size > 0) {
				struct iovec local;
				struct iovec remote;
				auto tmpstr = process->vchrootPath().substr(0, _body.buffer_size - 1);
				auto len = std::min(tmpstr.length() + 1, _body.buffer_size);

				fullLength = process->vchrootPath().length();

				// note that, despite the const cast, this is safe because the local iovec data is not modified by the call
				local.iov_base = const_cast<char*>(tmpstr.c_str());
				local.iov_len = len;

				remote.iov_base = _body.buffer;
				remote.iov_len = len;

				if (process_vm_writev(process->id(), &local, 1, &remote, 1, 0) < 0) {
					code = -errno;
				}
			}
		} else {
			code = -ESRCH;
		}
	} else {
		code = -ESRCH;
	}

	_sendReply(code, fullLength);
	_stopPending();
};
