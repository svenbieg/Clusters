//================
// shared_map.hpp
//================

// Thread-safe implementation of a sorted map
// Items can be inserted, removed and looked-up in constant low time

// Copyright 2025, Sven Bieg (svenbieg@outlook.de)
// http://github.com/svenbieg/Clusters


#pragma once


//=======
// Using
//=======

#include "map.hpp"
#include "shared_cluster.hpp"


//===========
// Namespace
//===========

namespace Collections {


//==========
// Iterator
//==========

template <typename _traits_t, bool _is_const>
class shared_map_iterator: public shared_cluster_iterator<_traits_t, _is_const>
{
public:
	// Using
	using _base_t=shared_cluster_iterator<_traits_t, _is_const>;
	using _item_t=typename _traits_t::item_t;
	using _iterator_t=typename _traits_t::iterator_t;
	using _key_t=typename _traits_t::key_t;
	using _value_t=typename _traits_t::value_t;

	// Con-/Destructors
	using _base_t::_base_t;

	// Navigation
	template <class _key_param_t> bool find(_key_param_t const& key, find_func func=find_func::equal)
		{
		if(this->is_outside())
			this->lock();
		if(!_iterator_t::find(key, func))
			{
			this->unlock();
			return false;
			}
		return true;
		}
	_value_t const& get_value()const { return _base_t::get_current().get_value(); }
	_key_t const& get_key()const { return _base_t::get_current().get_key(); }
};


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
	using _iterator_base_t=typename shared_cluster_iterator_base<_traits_t, false>::_base_t;
	using iterator=shared_map_iterator<_traits_t, false>;
	using const_iterator=shared_map_iterator<_traits_t, true>;

	// Con-/Destructors
	shared_map() {}

	// Access
	template <class _key_param_t> inline _value_t operator[](_key_param_t const& key) { return get(key); }
	inline const_iterator cfind(_key_t const& key, find_func func=find_func::equal)
		{
		const_iterator it(this);
		it.find(key, func);
		return it;
		}
	inline bool contains(_key_t const& key)
		{
		std::shared_lock<std::shared_mutex> lock(this->m_mutex);
		return _cluster_t::contains(key);
		}
	inline iterator find(_key_t const& key, find_func func=find_func::equal)
		{
		iterator it(this);
		it.find(key, func);
		return it;
		}
	template <class _key_param_t> inline _value_t get(_key_param_t const& key)
		{
		std::shared_lock<std::shared_mutex> lock(this->m_mutex);
		return _cluster_t::get(key);
		}
	template <class _key_param_t> inline bool try_get(_key_param_t const& key, _value_t* value)
		{
		std::shared_lock<std::shared_mutex> lock(this->m_mutex);
		return _cluster_t::try_get(key, value);
		}

	// Modification
	template <class _key_param_t, class _value_param_t> inline bool add(_key_param_t const& key, _value_param_t const& value)
		{
		std::unique_lock<std::shared_mutex> lock(this->m_mutex);
		return _cluster_t::add(key, value);
		}
	inline bool remove(_key_t const& key)
		{
		std::unique_lock<std::shared_mutex> lock(this->m_mutex);
		return _cluster_t::remove(key);
		}
	template <class _key_param_t, class _value_param_t> inline bool set(_key_param_t const& key, _value_param_t const& value)
		{
		std::unique_lock<std::shared_mutex> lock(this->m_mutex);
		return _cluster_t::set(key, value);
		}
};

}