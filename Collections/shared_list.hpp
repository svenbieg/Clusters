//=================
// shared_list.hpp
//=================

// Thread-safe implementation of an ordered list.
// Items can be inserted and removed in constant low time.

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Clusters/wiki/List

#pragma once


//=======
// Using
//=======

#include "list.hpp"
#include "shared_cluster.hpp"


//===========
// Namespace
//===========

namespace Collections {


//=============
// Shared List
//=============

template <typename _item_t, typename _size_t=uint32_t, uint16_t _group_size=10>
class shared_list: public iterable_shared_cluster<list_traits<_item_t, _size_t, _group_size>>
{
public:
	// Using
	using _base_t=iterable_shared_cluster<list_traits<_item_t, _size_t, _group_size>>;
	using _traits_t=list_traits<_item_t, _size_t, _group_size>;
	using _cluster_t=typename _traits_t::cluster_t;
	using shared_mutex=std::shared_mutex;
	using read_lock=std::shared_lock<shared_mutex>;
	using write_lock=std::unique_lock<shared_mutex>;

	// Con-/Destructors
	shared_list()noexcept {}
	shared_list(_cluster_t const& copy): _base_t(copy) {}
	shared_list(shared_list& copy): _base_t(copy) {}
	shared_list(shared_list const& copy)=delete;

	// Access
	inline bool contains(_item_t const& item)
		{
		read_lock lock(_base_t::m_mutex);
		return _cluster_t::contains(item);
		}
	inline _size_t get_many(_size_t position, _item_t* items, _size_t count)
		{
		read_lock lock(_base_t::m_mutex);
		return _cluster_t::get_many(position, items, count);
		}
	inline bool index_of(_item_t const& item, _size_t* position)
		{
		read_lock lock(_base_t::m_mutex);
		return _cluster_t::index_of(item, position);
		}

	// Modification
	inline bool add(_item_t const& item)
		{
		write_lock lock(_base_t::m_mutex);
		return _cluster_t::add(item);
		}
	inline void append(_item_t const& item)
		{
		write_lock lock(_base_t::m_mutex);
		_cluster_t::append(item);
		}
	inline void append(_item_t const* items, _size_t count)
		{
		write_lock lock(_base_t::m_mutex);
		_cluster_t::append(items, count);
		}
	inline bool insert_at(_size_t position, _item_t const& item)
		{
		write_lock lock(_base_t::m_mutex);
		return _cluster_t::insert_at(position, item);
		}
	inline bool remove(_item_t const& item)
		{
		write_lock lock(_base_t::m_mutex);
		return _cluster_t::remove(item);
		}
	inline bool set_at(_size_t position, _item_t const& item)
		{
		write_lock lock(_base_t::m_mutex);
		return _cluster_t::set_at(position, item);
		}
	inline _size_t set_many(_size_t position, _item_t const* items, _size_t count)
		{
		write_lock lock(_base_t::m_mutex);
		return _cluster_t::set_many(position, items, count);
		}

private:
	// Modification
	inline bool set_internal(_size_t position, _item_t const& item)
		{
		return _cluster_t::set_at(position, item);
		}
};

}