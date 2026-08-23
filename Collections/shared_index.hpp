//==================
// shared_index.hpp
//==================

// Thread-safe implementation of a sorted index.
// Items can be inserted, removed and looked-up in constant low time.

// Copyright 2025, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Clusters/wiki/Index

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
		if(_base_t::is_outside())
			_base_t::lock();
		if(!_iterator_t::find(item, func))
			{
			_base_t::unlock();
			return false;
			}
		return true;
		}
};


//==============
// Shared Index
//==============

template <typename _item_t, typename _size_t=uint32_t, uint16_t _group_size=10>
class shared_index: public iterable_shared_cluster<index_traits<_item_t, _size_t, _group_size>>
{
public:
	// Using
	using _traits_t=index_traits<_item_t, _size_t, _group_size>;
	using _base_t=iterable_shared_cluster<_traits_t>;
	using _cluster_t=typename _traits_t::cluster_t;
	using iterator=shared_index_iterator<_traits_t, false>;
	using const_iterator=shared_index_iterator<_traits_t, true>;
	using shared_mutex=std::shared_mutex;
	using read_lock=std::shared_lock<shared_mutex>;
	using write_lock=std::unique_lock<shared_mutex>;

	// Con-/Destructors
	shared_index()noexcept {}
	shared_index(_cluster_t const& copy): _base_t(copy) {}
	shared_index(shared_index& copy): _base_t(copy) {}
	shared_index(shared_index const& copy)=delete;

	// Access
	inline const_iterator cfind(_item_t const& item, find_func func=find_func::equal)
		{
		const_iterator it(this);
		it.find(item, func);
		return it;
		}
	inline bool contains(_item_t const& item)
		{
		read_lock lock(_base_t::m_mutex);
		return _cluster_t::contains(item);
		}
	inline iterator find(_item_t const& item, find_func func=find_func::equal)
		{
		iterator it(this);
		it.find(item, func);
		return it;
		}
	inline bool index_of(_item_t const& item, _size_t* pos_ptr)
		{
		read_lock lock(_base_t::m_mutex);
		return _cluster_t::index_of(item, pos_ptr);
		}

	// Modification
	template <class _item_param_t> inline bool add(_item_param_t const& item)
		{
		write_lock lock(_base_t::m_mutex);
		return _cluster_t::add(item);
		}
	inline bool remove(_item_t const& item)
		{
		write_lock lock(_base_t::m_mutex);
		return _cluster_t::remove(item);
		}
	template <class _item_param_t> inline bool set(_item_param_t const& item)
		{
		write_lock lock(_base_t::m_mutex);
		return _cluster_t::set(item);
		}
};

}