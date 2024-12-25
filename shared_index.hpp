//==================
// shared_index.hpp
//==================

// Thread-safe implementation of a sorted index
// Items can be inserted, removed and looked-up in constant low time

// Copyright 2024, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/Clusters


#pragma once


//=======
// Using
//=======

#include "index.hpp"
#include "shared_cluster.hpp"


//===========
// Namespace
//===========

namespace Collections {


//==========
// Iterator
//==========

template <typename _traits_t, bool _is_const>
class shared_index_iterator: public shared_cluster_iterator<_traits_t, _is_const>
{
public:
	// Using
	using _base_t=shared_cluster_iterator<_traits_t, _is_const>;
	using _item_t=typename _traits_t::item_t;
	using _iterator_t=typename _traits_t::iterator_t;

	// Con-/Destructors
	using _base_t::_base_t;

	// Navigation
	bool find(_item_t const& item, find_func func=find_func::equal)
		{
		if(this->is_outside())
			this->lock();
		if(!_iterator_t::find(item, func))
			{
			this->unlock();
			return false;
			}
		return true;
		}
};


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
	using iterator=shared_index_iterator<_traits_t, false>;
	using const_iterator=shared_index_iterator<_traits_t, true>;

	// Con-/Destructors
	shared_index() {}

	// Access
	inline const_iterator cfind(_item_t const& item, find_func func=find_func::equal)
		{
		const_iterator it(this);
		it.find(item, func);
		return it;
		}
	inline bool contains(_item_t const& item)
		{
		std::shared_lock<std::shared_mutex> lock(this->m_mutex);
		return _cluster_t::contains(item);
		}
	inline iterator find(_item_t const& item, find_func func=find_func::equal)
		{
		iterator it(this);
		it.find(item, func);
		return it;
		}

	// Modification
	template <typename _item_param_t> inline bool add(_item_param_t&& item)
		{
		std::unique_lock<std::shared_mutex> lock(this->m_mutex);
		return _cluster_t::add(std::forward<_item_param_t>(item));
		}
	inline bool remove(_item_t const& item)
		{
		std::unique_lock<std::shared_mutex> lock(this->m_mutex);
		return _cluster_t::remove(item);
		}
	template <typename _item_param_t> inline bool set(_item_param_t&& item)
		{
		std::unique_lock<std::shared_mutex> lock(this->m_mutex);
		return _cluster_t::set(std::forward<_item_param_t>(item));
		}
};

}