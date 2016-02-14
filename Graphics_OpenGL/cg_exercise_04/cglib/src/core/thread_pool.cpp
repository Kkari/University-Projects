#include <cglib/core/thread_pool.h>
#include <cglib/core/timer.h>

#include <cglib/core/assert.h>
#include <iostream>
#include <sstream>

ThreadPool::ThreadPool(unsigned max_threads) :
	m_numJobs(0), m_hasException(false)
{
	using std::cout;
	using std::endl;

	if (max_threads == unsigned(-1))
	{
		max_threads = std::thread::hardware_concurrency();
	}

	cout << "[ThreadPool] " << "Using " << max_threads << " worker threads" << endl;
	m_threads.resize(max_threads);
	m_tld.resize(max_threads);
	m_terminate.store(true);
}

// -----------------------------------------------------------------------------

ThreadPool::~ThreadPool()
{
	terminate();
}

// -----------------------------------------------------------------------------

void ThreadPool::run_internal(
	int num_jobs, 
	std::function<void(int, ThreadLocalData* tld, std::atomic<bool>&)> kernel,
	std::function<void(int, std::unique_ptr<ThreadLocalData>& tld)> tldAlloc
)
{
	cg_assert(num_jobs >= 0);
	terminate();

	// Set up data for jobs.
	m_currentJob.store(0);
	m_kernel = kernel;
	m_numJobs.store(num_jobs);
	m_jobsDone.store(0);
	m_terminate.store(false);
	m_hasException.store(false);
	m_exceptionMsg.clear();

	for (int i = 0; i < static_cast<int>(m_threads.size()); ++i)
	{
		tldAlloc(i, m_tld[i]);
	}

	auto thread_func = [&](int threadId) 
	{
		while (true)
		{
			int const jobId = m_currentJob++;
			if (jobId >= m_numJobs.load())
			{
				return;
			}

			try 
			{
				m_kernel(jobId, m_tld[threadId].get(), m_terminate);
			} catch (std::exception const& e)
			{
				std::lock_guard<std::mutex> guard(m_exceptionMutex);
				m_hasException.store(true);
				std::ostringstream os;
				os << "Thread " << std::this_thread::get_id() << ": " << e.what();
				m_exceptionMsg.push_back(os.str());
				m_numJobs.store(0);
				m_terminate.store(true);
			} catch(...)
			{
				std::lock_guard<std::mutex> guard(m_exceptionMutex);
				m_hasException.store(true);
				std::ostringstream os;
				os << "Thread " << std::this_thread::get_id() << ": " << "unknown exception caught";
				m_exceptionMsg.push_back(os.str());
				m_numJobs.store(0);
				m_terminate.store(true);
			}
			m_jobsDone++;
			std::this_thread::yield();
		}

		m_tld[threadId].reset();
	};

	// Launch threads.
	for (int i = 0; i < static_cast<int>(m_threads.size()); ++i)
	{
		m_threads[i].reset(new std::thread(thread_func, i));
	}
}

// -----------------------------------------------------------------------------

void ThreadPool::terminate() 
{
	m_numJobs.store(0);
	m_terminate.store(true);
	for (int i = 0; i < static_cast<int>(m_threads.size()); ++i)
	{
		auto& t = m_threads[i];
		if (t && t->joinable())
		{
			t->join();
		}
		t.reset();
		m_tld[i].reset();
	}
}

// -----------------------------------------------------------------------------

#if (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 199506L) \
 || (defined(_XOPEN_SOURCE))
#include <signal.h>
void ThreadPool::force_kill()
{
	// Give some chance to threads to terminate gracefully.
	m_numJobs.store(0);
	m_terminate.store(true);

	for (int i = 0; i < static_cast<int>(m_threads.size()); ++i)
	{
		auto& t = m_threads[i];
		if (t && t->joinable())
		{
			int const result = pthread_kill(t->native_handle(), SIGTERM);
			cg_assert((result == 0) && bool("Cannot kill thread."));
		}
		t.reset();
		m_tld[i].reset();
	}
}
#else
void ThreadPool::force_kill()
{
	cg_assert(!bool("Trying to terminate the thread pool, but that is not "
				    "supported on non-POSIX system."));
}
#endif

// -----------------------------------------------------------------------------

bool ThreadPool::kill_at_timeout(int timeout)
{
    Timer timer;
    timer.start();
	bool terminated = false;

	while (timer.getElapsedTimeInSec() < timeout)
	{
		if (jobs_done() >= num_jobs())
		{
			terminated = true;
			break;
		}
		std::this_thread::yield();
	}

	if (!terminated)
	{
		force_kill();
		return true;
	}

	return false;
}
