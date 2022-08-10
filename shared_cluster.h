//==================
// shared_cluster.h
//==================

// Tread-safe implementation of a pyramidal directory
// Shared classes for shared_list and shared_index

// Copyright 2022, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _CLUSTERS_SHARED_CLUSTER_H
#define _CLUSTERS_SHARED_CLUSTER_H


//=======
// Using
//=======

#include <shared_mutex>
#include "cluster.h"


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
	using _cluster_t=typename _traits_t::cluster_t;
	using _item_t=typename _traits_t::item_t;
	using _size_t=typename _traits_t::size_t;

	// Con-/Destructors
	shared_cluster() {}
	virtual ~shared_cluster() {}

	// Access
	_item_t get_at(_size_t position)
		{
		m_mutex.lock_shared();
		_item_t const& item=_cluster_t::get_at(position);
		m_mutex.unlock_shared();
		return item;
		}

	// Modification
	void clear()
		{
		m_mutex.lock();
		_cluster_t::clear();
		m_mutex.unlock();
		}
	bool remove_at(_size_t position)
		{
		m_mutex.lock();
		bool removed=_cluster_t::remove_at(position);
		m_mutex.unlock();
		return removed;
		}

protected:
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
class shared_cluster_iterator_base: protected cluster_iterator_typed<_traits_t, _is_const>
{
public:
	// Using
	using _base_t=cluster_iterator_typed<_traits_t, _is_const>;
	using _cluster_t=cluster<_traits_t>;
	using _cluster_ptr=typename std::conditional<_is_const, _cluster_t const*, _cluster_t*>::type;
	using _shared_cluster_t=shared_cluster<_traits_t>;
	using _item_t=typename _traits_t::item_t;
	using _item_ptr=typename std::conditional<_is_const, _item_t const*, _item_t*>::type;
	using _item_ref=typename std::conditional<_is_const, _item_t const&, _item_t&>::type;
	using _size_t=typename _traits_t::size_t;

	// Con-/Destructors
	shared_cluster_iterator_base(_shared_cluster_t* cluster, _size_t position): _base_t((_cluster_ptr)cluster)
		{
		set_position(position);
		}
	shared_cluster_iterator_base(_base_t&& it): _base_t(std::forward<_base_t>(it)) {}
	~shared_cluster_iterator_base()
		{
		if(!this->is_outside())
			unlock();
		}

	// Access
	_item_ref operator*()const { return _base_t::get_current(); }
	_item_ptr operator->()const { return _base_t::operator->(); }
	_item_ref get_current()const { return _base_t::get_current(); }
	inline bool has_current()const { return _base_t::has_current(); }

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
	void unlock()
		{
		auto cluster=(_shared_cluster_t*)this->m_cluster;
		_is_const? cluster->m_mutex.unlock_shared(): cluster->m_mutex.unlock();
		}
};

template <class _traits_t, bool _is_const>
class shared_cluster_iterator_typed: public shared_cluster_iterator_base<_traits_t, false>
{
public:
	// Using
	using _base_t=shared_cluster_iterator_base<_traits_t, false>;
	using iterator_base_t=cluster_iterator_typed<_traits_t, false>;

	// Con-/Destructors
	using _base_t::_base_t;

	// Modification
	inline bool remove_current() { return iterator_base_t::remove_current(); }
};

template <class _traits_t>
class shared_cluster_iterator_typed<_traits_t, true>: public shared_cluster_iterator_base<_traits_t, true>
{
public:
	// Using
	using _base_t=cluster_iterator_base<_traits_t, true>;

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
	using iterator=shared_cluster_iterator_typed<_traits_t, false>;
	using const_iterator=shared_cluster_iterator_typed<_traits_t, true>;

	// Access
	inline iterator begin() { return iterator(this, 0); }
	inline iterator begin(_size_t position) { return iterator(this, position); }
	inline const_iterator cbegin() { return const_iterator(this, 0); }
	inline const_iterator cbegin(_size_t position) { return const_iterator(this, position); }
	inline const_iterator cend() { return const_iterator(this, -2); }
	inline iterator end() { return iterator(this, -2); }
	inline iterator rend() { return iterator(this, -1); }
};

} // namespace

#endif // _CLUSTERS_SHARED_CLUSTER_H
