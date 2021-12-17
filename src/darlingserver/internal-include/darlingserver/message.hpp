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

#ifndef _DARLINGSERVER_MESSAGE_HPP_
#define _DARLINGSERVER_MESSAGE_HPP_

#include <sys/socket.h>
#include <sys/un.h>
#include <vector>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <functional>

namespace DarlingServer {
	class Address {
	private:
		struct sockaddr_un _address;
		size_t _size;

	public:
		Address();
		Address(const struct sockaddr_un& rawAddress, size_t addressSize = sizeof(struct sockaddr_un));

		struct sockaddr_un& raw();
		const struct sockaddr_un& raw() const;

		size_t rawSize() const;
		void setRawSize(size_t newRawSize);
	};

	/**
	 * A C++ wrapper for local (Unix) SOCK_DGRAM/SOCK_SEQPACKET socket messages, with support for file descriptors (SCM_RIGHTS) and credentials (SCM_CREDENTIALS).
	 */
	class Message {
	private:
		struct msghdr _header;
		struct iovec _dataDescriptor;
		std::vector<uint8_t> _buffer;
		struct cmsghdr* _controlHeader = nullptr;
		Address _socketAddress;

		void _initWithOther(Message&&);
		void _cleanupSelf();

		struct cmsghdr* _credentialsHeader();
		const struct cmsghdr* _credentialsHeader() const;
		struct cmsghdr* _descriptorHeader();
		const struct cmsghdr* _descriptorHeader() const;
		size_t _descriptorSpace() const;

		void _ensureCredentialsHeader();
		void _ensureDescriptorHeader(size_t descriptorSpace);

	public:
		/**
		 * Default-initializes a Message.
		 *
		 * By default, the data buffer holds 256 bytes and the control data buffer has space for 4 descriptors.
		 *
		 * If this Message is being used to receive a message from a socket, you must preallocate
		 * the buffers for the recvmsg/recvmmsg call. The buffers are already preallocated to the
		 * sizes specified by in this constructor, however, if you wish to grow the data buffer,
		 * you can call pushData() with a null buffer pointer and a non-zero size. If you wish to
		 * grow the control data buffer, you can call pushDescriptor() with a descriptor value of `-1`.
		 */
		Message(size_t bufferSpace = 256, size_t descriptorSpace = 4);
		~Message();

		Message(Message&&);
		Message& operator=(Message&&);

		Message(const Message&) = delete;
		Message& operator=(const Message&) = delete;

		struct msghdr& rawHeader();
		const struct msghdr& rawHeader() const;

		std::vector<uint8_t>& data();
		const std::vector<uint8_t>& data() const;

		Address address() const;
		void setAddress(Address address);

		bool copyCredentialsOut(struct ucred& outputCredentials) const;
		void copyCredentialsIn(const struct ucred& inputCredentials);

		pid_t pid() const;
		void setPID(pid_t pid);

		uid_t uid() const;
		void setUID(uid_t uid);

		gid_t gid() const;
		void setGID(gid_t gid);

		/**
		 * Returns an array of all the descritors currently owned by this Message.
		 *
		 * Note that the descriptors are still owned by this Message during and after this call.
		 * Callers should NOT close the descriptors they receive in the returned vector.
		 *
		 * See extractDescriptor() for a method that will transfer ownership of a descriptor to the caller.
		 */
		std::vector<int> descriptors() const;

		/**
		 * Acquires ownership of the given descriptor.
		 *
		 * Note that callers should NOT close the descriptor after a call to this method.
		 * This Message becomes the owner of the descriptor and will take care of closing it.
		 */
		void pushDescriptor(int descriptor);

		/**
		 * Extracts a single descriptor from this Message, transferring ownership of it to the caller.
		 *
		 * @returns The descriptor that was extracted, or `-1` if it was not found.
		 */
		int extractDescriptor(int descriptor);

		/**
		 * Like extractDescriptor(), but extracts it based on its index in the vector returned by descriptors().
		 */
		int extractDescriptorAtIndex(size_t index);

		/**
		 * Gives up ownership of the descriptors previously held by this Message and acquires ownership
		 * of the descriptors passed in the given vector.
		 *
		 * Note that this method does not return the old descriptors in any way,
		 * so if the caller wishes to know what they were, they must call descriptors() before calling this method.
		 */
		void replaceDescriptors(const std::vector<int>& newDescriptors);
	};

	/**
	 * A thread-safe Message queue with methods for sending and receiving messages.
	 */
	class MessageQueue {
	private:
		std::deque<Message> _messages;
		std::mutex _lock;
		std::function<void()> _messageArrivalNotificationCallback = nullptr;

	public:
		void setMessageArrivalNotificationCallback(std::function<void()> messageArrivalNotificationCallback);

		void push(Message&& message);
		std::optional<Message> pop();

		bool sendMany(int socket);
		bool receiveMany(int socket);
	};
};

#endif // _DARLINGSERVER_MESSAGE_HPP_
