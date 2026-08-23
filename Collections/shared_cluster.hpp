//====================
// shared_cluster.hpp
//====================

// Thread-safe implementation of a pyramidal directory.
// Shared classes for index and list.

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// http://github.com/svenbieg/Clusters

#pragma once


//=======
// Using
//=======

#include "Collections/cluster.hpp"
#include "Concurrency/ReadLock.h"
#include "Concurrency/WriteLock.h"


//===========
// Namespace
//===========

namespace Collections {


//======================
// Forward-Declarations
//======================

template <typename _traits_t, bool _is_const> class shared_cluster_iterator_base;


//================
// Shared Cluster
//================

template <typename _traits_t>
class shared_cluster: protected _traits_t::cluster_t
{
public:
	// Friends
	friend class shared_cluster_iterator_base<_traits_t, true>;
	friend class shared_cluster_iterator_base<_traits_t, false>;

	// Using
	using _base_t=typename _traits_t::cluster_t;
	using _cluster_t=typename _traits_t::cluster_t;
	using _item_t=typename _traits_t::item_t;
	using _size_t=typename _traits_t::size_t;
	using Mutex=Concurrency::Mutex;
	using ReadLock=Concurrency::ReadLock;
	using WriteLock=Concurrency::WriteLock;

	// Con-/Destructors
	virtual ~shared_cluster()noexcept {}

	// Access
	inline operator bool()
		{
		ReadLock lock(m_mutex);
		return _base_t::m_root!=nullptr;
		}
	inline _item_t get_at(_size_t position)
		{
		ReadLock lock(m_mutex);
		return _cluster_t::get_at(position);
		}
	inline _size_t get_count()
		{
		ReadLock lock(m_mutex);
		return _cluster_t::get_count();
		}

	// Modification
	inline shared_cluster& operator=(_cluster_t const& cluster)
		{
		copy_from(cluster);
		return *this;
		}
	inline shared_cluster& operator=(shared_cluster& cluster)
		{
		copy_from(cluster);
		return *this;
		}
	inline bool clear()
		{
		WriteLock lock(m_mutex);
		return _cluster_t::clear();
		}
	inline void copy_from(_cluster_t const& cluster)
		{
		WriteLock lock(m_mutex);
		_cluster_t::copy_from(cluster);
		}
	inline void copy_from(shared_cluster& cluster)
		{
		WriteLock lock(m_mutex);
		ReadLock read_lock(cluster.m_mutex);
		_cluster_t::copy_from(cluster);
		}
	inline void remove_at(_size_t position, _item_t* item_ptr=nullptr)
		{
		WriteLock lock(m_mutex);
		_cluster_t::remove_at(position, item_ptr);
		}

protected:
	// Con-/Destructors
	shared_cluster()noexcept: _base_t(nullptr) {}
	shared_cluster(_cluster_t const& copy): _base_t(nullptr)
		{
		copy_from(copy);
		}
	shared_cluster(shared_cluster& copy): _base_t(nullptr)
		{
		copy_from(copy);
		}

	// Access
	inline _item_t get_internal(_size_t position)
		{
		return _cluster_t::get_at(position);
		}

	// Modification
	inline bool remove_internal(_size_t position)
		{
		return _cluster_t::remove_at(position);
		}

	// Common
	Mutex m_mutex;
};


//==========
// Iterator
//==========

template <typename _traits_t, bool _is_const>
class shared_cluster_iterator_base: protected std::conditional<_is_const, typename _traits_t::const_iterator_t, typename _traits_t::iterator_t>::type
{
public:
	// Using
	using _base_t=typename std::conditional<_is_const, typename _traits_t::const_iterator_t, typename _traits_t::iterator_t>::type;
	using _cluster_t=cluster<_traits_t>;
	using _cluster_ptr=typename std::conditional<_is_const, _cluster_t const*, _cluster_t*>::type;
	using _shared_cluster_t=shared_cluster<_traits_t>;
	using _item_t=typename _traits_t::item_t;
	using _item_ptr=typename std::conditional<_is_const, _item_t const*, _item_t*>::type;
	using _item_ref=typename std::conditional<_is_const, _item_t const&, _item_t&>::type;
	using _size_t=typename _traits_t::size_t;
	using AccessMode=Concurrency::AccessMode;

	// Con-/Destructors
	shared_cluster_iterator_base(_shared_cluster_t* cluster)noexcept: _base_t((_cluster_ptr)cluster) {}
	shared_cluster_iterator_base(_shared_cluster_t* cluster, _size_t position): _base_t((_cluster_ptr)cluster)
		{
		set_position(position);
		}
	~shared_cluster_iterator_base()noexcept
		{
		if(!_base_t::is_outside())
			unlock();
		}

	// Access
	inline _item_ref operator*()const { return _base_t::operator*(); }
	inline _item_ptr operator->()const { return _base_t::operator->(); }
	inline _item_ref get_current()const { return _base_t::get_current(); }
	inline bool has_current()const noexcept { return _base_t::has_current(); }

	// Comparison
	inline bool operator==(shared_cluster_iterator_base const& it)noexcept { return _base_t::operator==(it); }
	inline bool operator!=(shared_cluster_iterator_base const& it)noexcept { return !operator==(it); }

	// Navigation
	inline shared_cluster_iterator_base& operator++() { move_next(); return *this; }
	inline shared_cluster_iterator_base& operator--() { move_previous(); return *this; }
	inline _size_t get_position()const noexcept { return _base_t::get_position(); }
	bool move_next()override
		{
		if(_base_t::is_outside())
			{
			lock();
			if(_base_t::move_next())
				return true;
			unlock();
			return false;
			}
		if(_base_t::move_next())
			return true;
		unlock();
		return false;
		}
	bool move_previous()override
		{
		if(_base_t::is_outside())
			{
			lock();
			if(_base_t::move_previous())
				return true;
			unlock();
			return false;
			}
		if(_base_t::move_previous())
			return true;
		unlock();
		return false;
		}
	bool set_position(_size_t position)override
		{
		if(_base_t::is_outside())
			{
			if(_base_t::is_outside(position))
				{
				_base_t::m_position=position;
				return false;
				}
			lock();
			if(_base_t::set_position(position))
				return true;
			unlock();
			return false;
			}
		if(_base_t::set_position(position))
			return true;
		unlock();
		return false;
		}

protected:
	// Common
	void lock()
		{
		auto cluster=(_shared_cluster_t*)_base_t::m_cluster;
		_is_const? cluster->m_mutex.Lock(AccessMode::ReadOnly): cluster->m_mutex.Lock();
		}
	bool rbegin()
		{
		if(_base_t::is_outside())
			lock();
		_size_t count=_base_t::m_cluster->get_count();
		if(count==0)
			{
			unlock();
			return false;
			}
		if(_base_t::set_position(count-1))
			return true;
		unlock();
		return false;
		}
	void unlock()noexcept
		{
		auto cluster=(_shared_cluster_t*)_base_t::m_cluster;
		_is_const? cluster->m_mutex.Unlock(AccessMode::ReadOnly): cluster->m_mutex.Unlock();
		}
};

template <class _traits_t, bool _is_const>
class shared_cluster_iterator: public shared_cluster_iterator_base<_traits_t, false>
{
public:
	// Using
	using _base_t=shared_cluster_iterator_base<_traits_t, false>;
	using _item_t=typename _traits_t::item_t;
	using _iterator_t=typename _traits_t::iterator_t;
	using _size_t=typename _traits_t::size_t;

	// Con-/Destructors
	using _base_t::_base_t;

	// Navigation
	inline bool begin() { return _base_t::set_position(0); }
	inline bool begin(_size_t position) { return _base_t::set_position(position); }
	inline bool end() { return _base_t::set_position(-2); }
	inline bool rbegin() { return _base_t::rbegin(); }
	inline bool rend() { return _base_t::set_position(-1); }

	// Modification
	bool remove_current(_item_t* item_ptr=nullptr)
		{
		if(!_iterator_t::remove_current(item_ptr))
			return false;
		if(_base_t::is_outside())
			_base_t::unlock();
		return true;
		}
};

template <class _traits_t>
class shared_cluster_iterator<_traits_t, true>: public shared_cluster_iterator_base<_traits_t, true>
{
public:
	// Using
	using _base_t=shared_cluster_iterator_base<_traits_t, true>;
	using _size_t=typename _traits_t::size_t;

	// Con-/Destructors
	using _base_t::_base_t;

	// Navigation
	inline bool cbegin() { return _base_t::set_position(0); }
	inline bool cbegin(_size_t position) { return _base_t::set_position(position); }
	inline bool cend() { return _base_t::set_position(-2); }
	inline bool crbegin() { return _base_t::rbegin(); }
	inline bool crend() { return _base_t::set_position(-1); }
};


//=========================
// Iterable Shared Cluster
//=========================

template <typename _traits_t>
class iterable_shared_cluster: public shared_cluster<_traits_t>
{
public:
	// Using
	using _size_t=typename _traits_t::size_t;
	using iterator=typename _traits_t::shared_iterator_t;
	using const_iterator=typename _traits_t::shared_const_iterator_t;

	// Access
	inline iterator begin() { return iterator(this, 0); }
	inline iterator begin(_size_t position) { return iterator(this, position); }
	inline const_iterator cbegin() { return const_iterator(this, 0); }
	inline const_iterator cbegin(_size_t position) { return const_iterator(this, position); }
	inline const_iterator cend() { return const_iterator(this, -2); }
	inline const_iterator crend() { return const_iterator(this, -1); }
	inline iterator end() { return iterator(this, -2); }
	inline iterator rend() { return iterator(this, -1); }

protected:
	// Con-/Destructor
	iterable_shared_cluster() {}
};

}