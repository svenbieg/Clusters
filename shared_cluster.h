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

template <typename _traits_t> class shared_cluster_iterator;


//================
// Shared Cluster
//================

template <class _traits_t>
class shared_cluster: protected _traits_t::cluster_t
{
public:
	// Friends
	friend class shared_cluster_iterator<_traits_t>;

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

template <class _traits_t>
class shared_cluster_iterator
{
public:
	// Using
	using _cluster_t=shared_cluster<_traits_t>;
	using _iterator_t=cluster_iterator<_traits_t>;
	using _item_t=typename _traits_t::item_t;
	using _size_t=typename _traits_t::size_t;

	// Con-/Destructors
	shared_cluster_iterator(_cluster_t* cluster, _size_t position):
		m_cluster(cluster), m_iterator(cluster)
		{
		m_cluster->m_mutex.lock_shared();
		set_position(position);
		}
	shared_cluster_iterator(_cluster_t* cluster, _iterator_t&& it):
		m_cluster(cluster), m_iterator(std::forward<_iterator_t>(it))
		{
		m_cluster->m_mutex.lock_shared();
		}
	~shared_cluster_iterator()
		{
		m_cluster->m_mutex.unlock_shared();
		}

	// Access
	_item_t& operator*()const { return m_iterator.get_current(); }
	_item_t* operator->()const { return m_iterator.operator->(); }
	_item_t& get_current()const { return m_iterator.get_current(); }
	inline bool has_current()const { return m_iterator.has_current(); }

	// Comparison
	inline bool operator==(shared_cluster_iterator const& it) { return m_iterator==it.m_iterator; }
	inline bool operator!=(shared_cluster_iterator const& it) { return !operator==(it); }

	// Navigation
	inline shared_cluster_iterator& operator++() { m_iterator.move_next(); return *this; }
	inline shared_cluster_iterator& operator--() { m_iterator.move_previous(); return *this; }
	inline _size_t get_position()const { return m_iterator.get_position(); }
	inline bool move_next() { return m_iterator.move_next(); }
	inline bool move_previous() { return m_iterator.move_previous(); }
	inline bool set_position(_size_t position) { return m_iterator.set_position(position); }

private:
	// Common
	_cluster_t* m_cluster;
	_iterator_t m_iterator;
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
	using iterator=shared_cluster_iterator<_traits_t>;

	// Access
	inline iterator begin() { return iterator(this, 0); }
	inline iterator begin(_size_t position) { return iterator(this, position); }
	inline iterator end() { return iterator(this, -2); }
	inline iterator rend() { return iterator(this, -1); }
};

} // namespace

#endif // _CLUSTERS_SHARED_CLUSTER_H
