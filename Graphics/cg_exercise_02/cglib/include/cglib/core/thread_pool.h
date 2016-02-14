#pragma once

/*
 * A very simple thread pool. It runs a given number of jobs concurrently with a fixed thread budget.
 */

#include <cglib/core/thread_local_data.h>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <sstream>

class ThreadPool
{
	public:
		ThreadPool(unsigned max_threads = -1);
		~ThreadPool();
		bool done() const;
		void terminate();
		void force_kill();

		inline int num_jobs() const
		{
			return m_numJobs.load();
		}

		inline int jobs_done() const
		{
			return m_jobsDone.load();
		}

		template <class TLD = void>
		void run(
			// Number of instances to run.
			int num_jobs, 
			// The kernel to run.
			// Parameters for the kernel are jobId, thread local data.
			std::function<void(int, ThreadLocalData* tld, std::atomic<bool>&)> kernel
		);

        bool enough_progress() const {
            return (num_jobs() == 0 || float(jobs_done())/num_jobs() > 0.1);
        }

		void
		wait()
		{
			for(auto &i: m_threads) {
				if(i->joinable()) {
						i->join();
				}
			}
		}

		void poll_exceptions()
		{
			if (m_hasException.load())
			{
				terminate();

				std::ostringstream os;
				os << "\n";
				for (auto it = m_exceptionMsg.begin(); it != m_exceptionMsg.end(); ++it)
				{
					os << *it << "\n";
				}
				throw std::runtime_error(os.str());
			}
		}

		bool kill_at_timeout(int timeout);

	private:
		void run_internal(
			int num_jobs,
			std::function<void(int, ThreadLocalData* tld, std::atomic<bool>&)> kernel,
			std::function<void(int, std::unique_ptr<ThreadLocalData>& tld)> tldAlloc
		);

	private:
		std::vector<std::unique_ptr<std::thread>>     m_threads;
		std::function<void(int, ThreadLocalData*, std::atomic<bool>&)>    m_kernel;
		std::vector<std::unique_ptr<ThreadLocalData>> m_tld;
		std::atomic<int>                              m_numJobs;
		std::atomic<int>                              m_currentJob;
		std::atomic<int>                              m_jobsDone;
		std::atomic<bool>                             m_terminate;
		std::atomic<bool>                             m_hasException;
		std::vector<std::string>                      m_exceptionMsg;
		std::mutex                                    m_exceptionMutex;
};

template <class TLD>
inline void ThreadPool::run(
	int num_jobs, 
	std::function<void(int, ThreadLocalData* tld, std::atomic<bool>&)> kernel
)
{
	static_assert(std::is_base_of<ThreadLocalData, TLD>::value,
		"The template argument to ThreadPool::run must be void or be derived from ThreadLocalData.");

	run_internal(num_jobs, kernel, [](int threadId, std::unique_ptr<ThreadLocalData>& tld) 
		{
			tld.reset(new TLD());
			tld->initialize(threadId);
		}
	);
}

template <>
inline void ThreadPool::run<void>(
	int num_jobs, 
	std::function<void(int, ThreadLocalData* tld, std::atomic<bool>&)> kernel
)
{
	run_internal(num_jobs, kernel, [](int, std::unique_ptr<ThreadLocalData>& tld) 
		{
			tld.reset();
		}
	);
}
