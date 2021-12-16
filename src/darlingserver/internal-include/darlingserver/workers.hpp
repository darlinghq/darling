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

#ifndef _DARLINGSERVER_WORKERS_HPP_
#define _DARLINGSERVER_WORKERS_HPP_

#include <queue>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <chrono>
#include <optional>

namespace DarlingServer {
	template<class WorkItem>
	class WorkQueue {
	private:
		std::mutex _queueMutex;
		std::queue<WorkItem> _workItems;
		std::function<void(WorkItem)> _workerFunction;
		std::unordered_map<std::thread::id, std::thread> _workerThreads;
		size_t _threadsAvailable = 0;
		std::condition_variable _cv;
		bool _dying = false;

		static constexpr unsigned int minimumThreads() {
			return 2;
		};

		static unsigned int maximumThreads() {
			unsigned int hw = std::thread::hardware_concurrency();
			if (hw > 0 && hw - 1 > minimumThreads()) {
				// minus one for the main thread
				return hw - 1;
			} else {
				return minimumThreads();
			}
		};

		static constexpr std::chrono::seconds temporaryWorkerWaitPeriod() {
			return std::chrono::seconds(15);
		};

		void worker(bool permanent) {
			std::optional<WorkItem> workItem = std::nullopt;
			std::unique_lock lock(_queueMutex, std::defer_lock);

			while (true) {
				// acquire the lock for the condition variable
				lock.lock();

				// we're going to wait for work, so we're available
				++_threadsAvailable;

				bool waitResult;
				auto predicate = [&, this]() {
					// if we're dying, stop waiting
					if (_dying) {
						--_threadsAvailable;
						return true;
					}

					// if we have a work item ready, take it and stop waiting
					if (_workItems.size() > 0) {
						workItem = std::move(_workItems.front());
						_workItems.pop();
						--_threadsAvailable;
						return true;
					}

					// otherwise, let's keep waiting
					return false;
				};

				if (permanent) {
					// permanent workers just wait until there's work available (or the queue is being destroyed)
					_cv.wait(lock, predicate);
					waitResult = true;
				} else {
					// temporary workers wait for a certain period of time for work; if there's no work available after that period, they die
					waitResult = _cv.wait_for(lock, temporaryWorkerWaitPeriod(), predicate);
				}

				// if the queue is being destroyed, let the thread die
				if (_dying) {
					return;
				}

				// if we failed to find any work items, let's die
				if (!waitResult) {
					auto thisThread = std::move(_workerThreads[std::this_thread::get_id()]);
					_workerThreads.erase(thisThread.get_id());
					thisThread.detach();
					return;
				}

				// drop the lock to perform the work
				lock.unlock();

				// perform the work
				_workerFunction(std::move(workItem.value()));
				workItem = std::nullopt;
			}
		};

	public:
		WorkQueue(std::function<void(WorkItem)> workerFunction):
			_workerFunction(workerFunction)
		{
			std::scoped_lock lock(_queueMutex);

			// start the permanent worker threads
			for (size_t i = 0; i < minimumThreads(); ++i) {
				std::thread worker = std::thread(&WorkQueue::worker, this, true);
				_workerThreads[worker.get_id()] = std::move(worker);
			}
		};
		~WorkQueue() {
			// tell all the threads that we're being destroyed
			std::unique_lock lock(_queueMutex);
			_dying = true;
			lock.unlock();
			_cv.notify_all();

			// now wait for all of the threads to die
			for (auto& [id, thread]: _workerThreads) {
				thread.join();
			}
		};

		WorkQueue(const WorkQueue&) = delete;
		WorkQueue& operator=(const WorkQueue&) = delete;
		WorkQueue(WorkQueue&&) = delete;
		WorkQueue& operator=(WorkQueue&&) = delete;

		void push(WorkItem workItem) {
			std::unique_lock lock(_queueMutex);
			_workItems.push(std::move(workItem));

			// if there are no threads available to perform the work AND we have less than the
			// maximum number of worker threads currently active, spawn a temporary worker thread.
			if (_threadsAvailable == 0 && _workerThreads.size() < maximumThreads()) {
				std::thread worker = std::thread(&WorkQueue::worker, this, false);
				_workerThreads[worker.get_id()] = std::move(worker);

				// note that we don't need to notify anyone in this case, since we want the new worker thread
				// to take care of the new work item (it's not a problem if another thread gets to it first, though).
			} else {
				// otherwise, notify one of the available threads that there's work available
				lock.unlock();
				_cv.notify_one();
			}
		};
	};
};

#endif // _DARLINGSERVER_WORKERS_HPP_
