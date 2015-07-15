/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"

using namespace std;

ThreadPool::ThreadPool(size_t numThreads) {
	workers = vector<worker_t>(numThreads);
	available_threads.reset(new semaphore(numThreads));
	functions_queue_sem.reset(new semaphore(0));
	wait_semaphore.reset(new semaphore(0));
	all_finished = 0;
	num_active_threads = 0;
	thread dt([this]() -> void {
		this->dispatcher();
	});
	dt.detach();
	for (size_t workerID = 0; workerID < numThreads; workerID++) {
		workers[workerID].ready_to_exec.reset(new semaphore(0)); 
		workers[workerID].available = false;
		workers[workerID].thunk = NULL;
	}
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
	all_finished++;
	f_queue_lock.lock();
	functions_queue.push(thunk);
	f_queue_lock.unlock();
	functions_queue_sem->signal();
}

void ThreadPool::wait() {
	wait_semaphore->wait();
	return;
}

void ThreadPool::dispatcher() {
	while (true) {
		functions_queue_sem->wait();
		available_threads->wait();
		workers_lock.lock();
		bool found = false;
		for (size_t i = 0; i < num_active_threads; i++) {
			if (workers[i].available) {
				workers[i].available = false;
				f_queue_lock.lock();
				workers[i].thunk = functions_queue.front();
				functions_queue.pop();
				f_queue_lock.unlock();
				workers[i].ready_to_exec->signal();
				found = true;
				break;
			}
		}
		if (!found) {
			workers[num_active_threads].available = false;
			f_queue_lock.lock();
			workers[num_active_threads].thunk = functions_queue.front();
			functions_queue.pop();
			f_queue_lock.unlock();
			workers[num_active_threads].ready_to_exec->signal();	

			thread wt([this](size_t num_active_threads) -> void {
				this->worker(num_active_threads);
			}, num_active_threads);
			wt.detach();
			num_active_threads++;

		}
		workers_lock.unlock();
	}
}

void ThreadPool::worker(size_t id) {
	while (true) {
		workers_lock.lock();
		unique_ptr<semaphore>& sem_copy = workers[id].ready_to_exec; 
		workers_lock.unlock();
		sem_copy->wait();
		workers[id].thunk();
		workers_lock.lock();
		workers[id].available = true;
		workers[id].thunk = NULL;
		workers_lock.unlock();
		available_threads->signal();
		all_finished--;
		if (all_finished == 0) wait_semaphore->signal();
	}
}

