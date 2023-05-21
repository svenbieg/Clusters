//====================
// shared_cluster.hpp
//====================

// Thread-safe implementation of a pyramidal directory
// Shared classes for shared_list and shared_index

// Copyright 2022, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _CLUSTERS_SHARED_CLUSTER_HPP
#define _CLUSTERS_SHARED_CLUSTER_HPP


//=======
// Using
//=======

#include <shared_mutex>
#include "cluster.hpp"


//===========
// Namespace
//===========

namespace Clusters {


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

	// Con-/Destructors
	virtual ~shared_cluster()noexcept {}

	// Access
	inline _item_t get_at(_size_t position)
		{
		std::shared_lock<std::shared_mutex> lock(m_mutex);
		return _cluster_t::get_at(position);
		}
	inline _size_t get_count()
		{
		std::shared_lock<std::shared_mutex> lock(m_mutex);
		return _cluster_t::get_count();
		}

	// Modification
	inline bool clear()
		{
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		return _cluster_t::clear();
		}
	inline void copy_from(_cluster_t&& cluster)
		{
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		_cluster_t::copy_from(std::forward<_cluster_t>(cluster));
		}
	inline void copy_from(_cluster_t const& cluster)
		{
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		_cluster_t::copy_from(cluster);
		}
	inline void copy_from(shared_cluster& cluster)
		{
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		std::shared_lock<std::shared_mutex> shared_lock(cluster.m_mutex);
		_cluster_t::copy_from(cluster);
		}
	inline void copy_from(shared_cluster&& cluster)
		{
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		std::unique_lock<std::shared_mutex> src_lock(cluster.m_mutex);
		_cluster_t::copy_from(std::forward<_cluster_t>(cluster));
		}
	inline bool remove_at(_size_t position, _item_t* item_ptr=nullptr)
		{
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		return _cluster_t::remove_at(position, item_ptr);
		}

protected:
	// Con-/Destructors
	shared_cluster(): _base_t(nullptr) {}

	// Modification
	inline bool remove_internal(_size_t position)
		{
		return _cluster_t::remove_at(position);
		}

	// Common
	std::shared_mutex m_mutex;
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

	// Con-/Destructors
	shared_cluster_iterator_base(_shared_cluster_t* cluster): _base_t((_cluster_ptr)cluster) {}
	shared_cluster_iterator_base(_shared_cluster_t* cluster, _size_t position): _base_t((_cluster_ptr)cluster)
		{
		set_position(position);
		}
	~shared_cluster_iterator_base()
		{
		if(!this->is_outside())
			unlock();
		}

	// Access
	_item_ref operator*()const noexcept { return _base_t::operator*(); }
	_item_ptr operator->()const noexcept { return _base_t::operator->(); }
	_item_ref get_current()const noexcept { return _base_t::get_current(); }
	inline bool has_current()const noexcept { return _base_t::has_current(); }

	// Comparison
	inline bool operator==(shared_cluster_iterator_base const& it) { return _base_t::operator==(it); }
	inline bool operator!=(shared_cluster_iterator_base const& it) { return !operator==(it); }

	// Navigation
	inline shared_cluster_iterator_base& operator++() { move_next(); return *this; }
	inline shared_cluster_iterator_base& operator--() { move_previous(); return *this; }
	inline _size_t get_position()const { return _base_t::get_position(); }
	bool move_next()
		{
		if(this->is_outside())
			{
			this->lock();
			if(_base_t::move_next())
				return true;
			this->unlock();
			return false;
			}
		if(_base_t::move_next())
			return true;
		this->unlock();
		return false;
		}
	bool move_previous()
		{
		if(this->is_outside())
			{
			this->lock();
			if(_base_t::move_previous())
				return true;
			this->unlock();
			return false;
			}
		if(_base_t::move_previous())
			return true;
		this->unlock();
		return false;
		}
	bool set_position(_size_t position)
		{
		if(this->is_outside())
			{
			if(this->is_outside(position))
				{
				this->m_position=position;
				return false;
				}
			this->lock();
			if(_base_t::set_position(position))
				return true;
			this->unlock();
			return false;
			}
		if(_base_t::set_position(position))
			return true;
		this->unlock();
		return false;
		}

protected:
	// Common
	void lock()
		{
		auto cluster=(_shared_cluster_t*)this->m_cluster;
		_is_const? cluster->m_mutex.lock_shared(): cluster->m_mutex.lock();
		}
	bool rbegin()
		{
		if(this->is_outside())
			this->lock();
		_size_t count=this->m_cluster->get_count();
		if(count==0)
			{
			this->unlock();
			return false;
			}
		if(_base_t::set_position(count-1))
			return true;
		this->unlock();
		return false;
		}
	void unlock()
		{
		auto cluster=(_shared_cluster_t*)this->m_cluster;
		_is_const? cluster->m_mutex.unlock_shared(): cluster->m_mutex.unlock();
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
	inline bool remove_current(_item_t* item_ptr=nullptr) { return _iterator_t::remove_current(item_ptr); }
};

template <class _traits_t>
class shared_cluster_iterator<_traits_t, true>: public shared_cluster_iterator_base<_traits_t, true>
{
public:
	// Using
	using _base_t=shared_cluster_iterator_base<_traits_t, true>;
	using _size_t=typename _traits_t::size_t;

	// Navigation
	inline bool cbegin() { return _base_t::set_position(0); }
	inline bool cbegin(_size_t position) { return _base_t::set_position(position); }
	inline bool cend() { return _base_t::set_position(-2); }
	inline bool crbegin() { return _base_t::rbegin(); }
	inline bool crend() { return _base_t::set_position(-1); }

	// Con-/Destructors
	using _base_t::_base_t;
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

} // namespace

#endif // _CLUSTERS_SHARED_CLUSTER_HPP
