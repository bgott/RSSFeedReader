/**
 * File: thread-pool.h
 * -------------------
 * This class defines the ThreadPool class, which accepts a collection
 * of thunks (which are zero-argument functions that don't return a value)
 * and schedules them in a FIFO manner to be executed by a constant number
 * of child threads that exist solely to invoke previously scheduled thunks.
 */

#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>     // for size_t
#include <functional>  // for the function template used in the schedule signature
#include <queue>
#include <thread>
#include <vector>
#include "semaphore.h" 
#include "ostreamlock.h" 
#include <algorithm>
#include <iomanip>
#include <atomic>
using namespace std;


class ThreadPool {
 public:

/**
 * Constructs a ThreadPool configured to spawn up to the specified
 * number of threads.
 */
 	ThreadPool(size_t numThreads);

/**
 * Schedules the provided thunk (which is something that can
 * be invoked as a zero-argument function without a return value)
 * to be executed by one of the ThreadPool's threads as soon as
 * all previously scheduled thunks have been handled.
 */
 	void schedule(const std::function<void(void)>& thunk);

/**
 * Blocks and waits until all previously scheduled thunks
 * have been executed in full.
 */
 	void wait();
  
 private:
 	struct worker_t{
		bool available;
		unique_ptr<semaphore> ready_to_exec;
		function<void(void)> thunk;
	}typedef worker_t;

	mutex workers_lock;
	mutex f_queue_lock;

	atomic_int all_finished;
	size_t num_active_threads;

 	unique_ptr<semaphore> available_threads;
 	unique_ptr<semaphore> functions_queue_sem;
 	unique_ptr<semaphore> wait_semaphore;
 	
 	vector<worker_t> workers;
 	queue<function<void(void)> > functions_queue;
 	
 	void dispatcher();
 	void worker(size_t id);
 	

/**
 * ThreadPools are the type of thing that shouldn't be cloneable, since it's
 * not clear what it means to clone a ThreadPool (should copies of all outstanding
 * functions to be executed be copied?).
 *
 * In order to prevent cloning, we remove the copy constructor and the
 * assignment operator.  By doing so, the compiler will ensure we never clone
 * a ThreadPool.
 */
  ThreadPool(const ThreadPool& original) = delete;
  ThreadPool& operator=(const ThreadPool& rhs) = delete;
};

#endif
