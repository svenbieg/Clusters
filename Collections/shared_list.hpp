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

#include "Collections/list.hpp"
#include "Collections/shared_cluster.hpp"


//===========
// Namespace
//===========

namespace Collections {


//======================
// Forward-Declarations
//======================

template <typename _item_t, typename _size_t, uint16_t _group_size> class shared_list;


//=============
// Shared Item
//=============

template <typename _item_t, typename _size_t, uint16_t _group_size>
class shared_list_item
{
public:
	// Using
	using _shared_list_t=shared_list<_item_t, _size_t, _group_size>;

	// Friends
	friend _shared_list_t;

	// Con-/Destructors
	~shared_list_item();

	// Access
	operator _item_t();

	// Modification
	shared_list_item& operator=(_item_t const& item);

private:
	// Con-/Destructors
	shared_list_item(_shared_list_t* list, _size_t position);

	// Common
	_shared_list_t* m_list;
	_size_t m_position;
};


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
	using _shared_item_t=shared_list_item<_item_t, _size_t, _group_size>;
	using ReadLock=Concurrency::ReadLock;
	using WriteLock=Concurrency::WriteLock;

	// Friends
	friend _shared_item_t;

	// Con-/Destructors
	shared_list()noexcept {}
	shared_list(_cluster_t const& copy): _base_t(copy) {}
	shared_list(shared_list& copy): _base_t(copy) {}
	shared_list(shared_list const& copy)=delete;

	// Access
	_shared_item_t operator[](_size_t position)
		{
		return _shared_item_t(this, position);
		}
	inline bool contains(_item_t const& item)
		{
		ReadLock lock(_base_t::m_mutex);
		return _cluster_t::contains(item);
		}
	inline _size_t get_many(_size_t position, _item_t* items, _size_t count)
		{
		ReadLock lock(_base_t::m_mutex);
		return _cluster_t::get_many(position, items, count);
		}
	inline bool index_of(_item_t const& item, _size_t* position)
		{
		ReadLock lock(_base_t::m_mutex);
		return _cluster_t::index_of(item, position);
		}

	// Modification
	inline bool add(_item_t const& item)
		{
		WriteLock lock(_base_t::m_mutex);
		return _cluster_t::add(item);
		}
	inline void append(_item_t const& item)
		{
		WriteLock lock(_base_t::m_mutex);
		_cluster_t::append(item);
		}
	inline void append(_item_t const* items, _size_t count)
		{
		WriteLock lock(_base_t::m_mutex);
		_cluster_t::append(items, count);
		}
	inline bool insert_at(_size_t position, _item_t const& item)
		{
		WriteLock lock(_base_t::m_mutex);
		return _cluster_t::insert_at(position, item);
		}
	inline bool remove(_item_t const& item)
		{
		WriteLock lock(_base_t::m_mutex);
		return _cluster_t::remove(item);
		}
	inline bool set_at(_size_t position, _item_t const& item)
		{
		WriteLock lock(_base_t::m_mutex);
		return _cluster_t::set_at(position, item);
		}
	inline _size_t set_many(_size_t position, _item_t const* items, _size_t count)
		{
		WriteLock lock(_base_t::m_mutex);
		return _cluster_t::set_many(position, items, count);
		}

private:
	// Modification
	inline bool set_internal(_size_t position, _item_t const& item)
		{
		return _cluster_t::set_at(position, item);
		}
};


//=====================
// Item Implementation
//=====================

template <typename _item_t, typename _size_t, uint16_t _group_size>
shared_list_item<_item_t, _size_t, _group_size>::shared_list_item(shared_list<_item_t, _size_t, _group_size>* list, _size_t position):
m_list(list),
m_position(position)
{
m_list->m_mutex.Lock();
}

template <typename _item_t, typename _size_t, uint16_t _group_size>
shared_list_item<_item_t, _size_t, _group_size>::~shared_list_item()
{
m_list->m_mutex.Unlock();
}

template <typename _item_t, typename _size_t, uint16_t _group_size>
shared_list_item<_item_t, _size_t, _group_size>::operator _item_t()
{
return m_list->get_internal(m_position);
}

template <typename _item_t, typename _size_t, uint16_t _group_size>
shared_list_item<_item_t, _size_t, _group_size>& shared_list_item<_item_t, _size_t, _group_size>::operator=(_item_t const& item)
{
m_list->set_internal(m_position, item);
return *this;
}

}