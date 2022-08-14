//================
// shared_index.h
//================

// Thread-safe implementation of a sorted index
// Items can be inserted, removed and looked-up in constant low time

// Copyright 2022, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _CLUSTERS_SHARED_INDEX_H
#define _CLUSTERS_SHARED_INDEX_H


//=======
// Using
//=======

#include "index.h"
#include "shared_cluster.h"


//===========
// Namespace
//===========

namespace Clusters {


//==============
// Shared Index
//==============

template <typename _item_t, typename _size_t=uint32_t, uint16_t _group_size=10>
class shared_index: public iterable_shared_cluster<index_traits<_item_t, _item_t, _size_t, _group_size>>
{
public:
	// Using
	using _traits_t=index_traits<_item_t, _item_t, _size_t, _group_size>;
	using _cluster_t=typename _traits_t::cluster_t;
	using _iterator_t=cluster_iterator<_traits_t, false>;
	using iterator=shared_cluster_iterator<_traits_t, false>;

	// Con-/Destructors
	shared_index() {}

	// Access
	bool contains(_item_t const& item)
		{
		this->m_mutex.lock_shared();
		bool contains=_cluster_t::contains(item);
		this->m_mutex.unlock_shared();
		return contains;
		}
	iterator find(_item_t const& item, bool above_or_equal=true)
		{
		this->m_mutex.lock_shared();
		_iterator_t found=_cluster_t::find(item, above_or_equal);
		iterator it(std::forward<_iterator_t>(found));
		this->m_mutex.unlock_shared();
		return it;
		}

	// Modification
	template <typename _item_param_t> bool add(_item_param_t&& item)noexcept
		{
		this->m_mutex.lock();
		bool added=_cluster_t::add(std::forward<_item_param_t>(item));
		this->m_mutex.unlock();
		return added;
		}
	bool remove(_item_t const& item)
		{
		this->m_mutex.lock();
		bool removed=_cluster_t::remove(item);
		this->m_mutex.unlock();
		return removed;
		}
	template <typename _item_param_t> void set(_item_param_t&& item)noexcept
		{
		this->m_mutex.lock();
		_cluster_t::set(std::forward<_item_param_t>(item));
		this->m_mutex.unlock();
		}
};

} // namespace

#endif // _CLUSTERS_SHARED_INDEX_H
