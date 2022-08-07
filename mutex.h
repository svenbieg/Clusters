//=========
// mutex.h
//=========

// Implementation of an upgradable shared lock

// Copyright 2022, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _CLUSTERS_MUTEX_H
#define _CLUSTERS_MUTEX_H


//=======
// Using
//=======

#include <condition_variable>
#include <shared_mutex>


//===========
// Namespace
//===========

namespace Clusters {


//=======
// Mutex
//=======

class mutex
{
public:
	// Con-/Destructors
	mutex(): m_access_count(0) {}

	// Access
	void lock_shared()
		{
		std::unique_lock<std::mutex> lock(m_access_mutex);
		m_mutex.lock_shared();
		m_access_count++;
		}
	void unlock_shared()
		{
		m_mutex.unlock_shared();
		m_access_count--;
		m_signal.notify_one();
		}

	// Modification
	void downgrade_lock()
		{
		m_mutex.unlock();
		m_mutex.lock_shared();
		m_access_mutex.unlock();
		}
	void lock_exclusive()
		{
		m_mutex.lock();
		}
	void unlock_exclusive()
		{
		m_mutex.unlock();
		}
	void upgrade_lock()
		{
		std::unique_lock<std::mutex> lock(m_access_mutex);
		while(m_access_count>1)
			m_signal.wait(lock);
		lock.release();
		m_mutex.unlock_shared();
		m_mutex.lock();
		}

private:
	// Common
	std::shared_mutex m_mutex;
	volatile uint16_t m_access_count;
	std::mutex m_access_mutex;
	std::condition_variable m_signal;
};

} // namespace

#endif // _CLUSTERS_MUTEX_H
