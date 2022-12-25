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
	using _iterator_t=cluster_iterator<_traits_t, false>;
	using iterator=shared_cluster_iterator<_traits_t, false>;

	// Con-/Destructors
	shared_map() {}

	// Access
	inline _value_t operator[](_key_t const& key) { return get(key); }
	inline bool contains(_key_t const& key)
		{
		std::shared_lock lock(this->m_mutex);
		return _cluster_t::contains(key);
		}
	iterator find(_key_t const& key, bool above_or_equal=true)
		{
		std::shared_lock lock(this->m_mutex);
		_iterator_t found=_cluster_t::find(key, above_or_equal);
		return iterator(std::forward<_iterator_t>(found));
		}
	_value_t get(_key_t const& key)
		{
		std::shared_lock lock(this->m_mutex);
		return _cluster_t::get(key);
		}

	// Modification
	template <typename _key_param_t, typename _value_param_t> inline bool add(_key_param_t&& key, _value_param_t&& value)
		{
		std::unique_lock lock(this->m_mutex);
		return _cluster_t::add(std::forward<_key_param_t>(key), std::forward<_value_param_t>(value));
		}
	inline bool remove(_key_t const& key)
		{
		std::unique_lock lock(this->m_mutex);
		return _cluster_t::remove(key);
		}
	template <typename _key_param_t, typename _value_param_t> inline void set(_key_param_t&& key, _value_param_t&& value)
		{
		std::unique_lock lock(this->m_mutex);
		_cluster_t::set(std::forward<_key_param_t>(key), std::forward<_value_param_t>(value));
		}
};

} // namespace

#endif // _CLUSTERS_SHARED_MAP_H
