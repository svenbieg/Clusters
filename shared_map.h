//==============
// shared_map.h
//==============

// Thread-safe implementation of a sorted map
// Items can be inserted, removed and looked-up in constant low time

// Copyright 2022, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _CLUSTERS_SHARED_MAP_H
#define _CLUSTERS_SHARED_MAP_H


//=======
// Using
//=======

#include "map.h"
#include "shared_cluster.h"


//===========
// Namespace
//===========

namespace Clusters {


//============
// Shared Map
//============

template <typename _key_t, typename _value_t, typename _size_t=uint32_t, uint16_t _group_size=10>
class shared_map: public iterable_shared_cluster<map_traits<_key_t, _value_t, _size_t, _group_size>>
{
public:
	// Using
	using _traits_t=map_traits<_key_t, _value_t, _size_t, _group_size>;
	using _item_t=typename _traits_t::item_t;
	using _cluster_t=typename _traits_t::cluster_t;
	using _iterator_t=cluster_iterator<_traits_t>;
	using iterator=shared_cluster_iterator<_traits_t>;

	// Con-/Destructors
	shared_map() {}

	// Access
	inline _value_t operator[](_key_t const& key) { return get(key); }
	bool contains(_key_t const& key)
		{
		this->m_mutex.lock_shared();
		bool contains=_cluster_t::contains(key);
		this->m_mutex.unlock_shared();
		return contains;
		}
	iterator find(_key_t const& key)
		{
		this->m_mutex.lock_shared();
		_iterator_t found=_cluster_t::find(key);
		iterator it(this, std::forward<_iterator_t>(found));
		this->m_mutex.unlock_shared();
		return it;
		}
	_value_t get(_key_t const& key)
		{
		this->m_mutex.lock_shared();
		_value_t value=_cluster_t::get(key);
		this->m_mutex.unlock_shared();
		return value;
		}

	// Modification
	template <typename _key_param_t, typename _value_param_t> bool add(_key_param_t&& key, _value_param_t&& value)
		{
		this->m_mutex.lock();
		bool added=_cluster_t::add(std::forward<_key_param_t>(key), std::forward<_value_param_t>(value));
		this->m_mutex.unlock();
		return added;
		}
	bool remove(_key_t const& key)
		{
		this->m_mutex.lock();
		bool removed=_cluster_t::remove(key);
		this->m_mutex.unlock();
		return removed;
		}
	template <typename _key_param_t, typename _value_param_t> void set(_key_param_t&& key, _value_param_t&& value)
		{
		this->m_mutex.lock();
		_cluster_t::set(std::forward<_key_param_t>(key), std::forward<_value_param_t>(value));
		this->m_mutex.unlock();
		}
};

} // namespace

#endif // _CLUSTERS_SHARED_MAP_H
