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
	inline bool contains(_item_t const& item)
		{
		std::shared_lock lock(this->m_mutex);
		return _cluster_t::contains(item);
		}
	iterator find(_item_t const& item, bool above_or_equal=true)
		{
		std::shared_lock lock(this->m_mutex);
		_iterator_t found=_cluster_t::find(item, above_or_equal);
		return iterator(std::forward<_iterator_t>(found));
		}

	// Modification
	template <typename _item_param_t> inline bool add(_item_param_t&& item)noexcept
		{
		std::unique_lock lock(this->m_mutex);
		return _cluster_t::add(std::forward<_item_param_t>(item));
		}
	inline bool remove(_item_t const& item)
		{
		std::unique_lock lock(this->m_mutex);
		return _cluster_t::remove(item);
		}
	template <typename _item_param_t> inline void set(_item_param_t&& item)noexcept
		{
		std::unique_lock lock(this->m_mutex);
		_cluster_t::set(std::forward<_item_param_t>(item));
		}
};

} // namespace

#endif // _CLUSTERS_SHARED_INDEX_H
