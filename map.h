//=======
// map.h
//=======

// Implementation of a sorted map
// Items can be inserted, removed and looked-up in constant low time

// Copyright 2022, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _CLUSTERS_MAP_H
#define _CLUSTERS_MAP_H


//=======
// Using
//=======

#include "index.h"


//===========
// Namespace
//===========

namespace Clusters {


//======================
// Forward-Declarations
//======================

template <typename _key_t, typename _value_t, typename _size_t, uint16_t _group_size> class map;
template <typename _key_t, typename _value_t> class map_item;

template <typename _key_t, typename _value_t, typename _size_t, uint16_t _group_size>
struct map_traits
{
using key_t=_key_t;
using item_t=map_item<_key_t, _value_t>;
using group_t=index_group<_key_t, item_t, _size_t, _group_size>;
using item_group_t=index_item_group<_key_t, item_t, _size_t, _group_size>;
using parent_group_t=index_parent_group<_key_t, item_t, _size_t, _group_size>;
using cluster_t=map<_key_t, _value_t, _size_t, _group_size>;
using iterator_t=index_iterator<map_traits, false>;
using const_iterator_t=index_iterator<map_traits, true>;
using size_t=_size_t;
static constexpr uint16_t group_size=_group_size;
};


//======
// Item
//======

template <typename _key_t, typename _value_t>
class map_item
{
public:
	// Con-/Destructors
	map_item(map_item const& item)noexcept: m_key(item.m_key), m_value(item.m_value) {}
	map_item(map_item&& item)noexcept: m_key(std::move(item.m_key)), m_value(std::move(item.m_value)) {}
	map_item(_key_t const& key, _value_t const& value)noexcept: m_key(key), m_value(value) {}
	map_item(_key_t const& key, _value_t&& value)noexcept: m_key(key), m_value(std::move(value)) {}
	map_item(_key_t&& key, _value_t const& value)noexcept: m_key(std::move(key)), m_value(value) {}
	map_item(_key_t&& key, _value_t&& value)noexcept: m_key(std::move(key)), m_value(std::move(value)) {}

	// Assignment
	inline map_item& operator=(map_item const& item)noexcept
		{
		m_key=item.m_key;
		m_value=item.m_value;
		return *this;
		}
	inline map_item& operator=(map_item&& item)noexcept
		{
		m_key=std::move(item.m_key);
		m_value=std::move(item.m_value);
		return *this;
		}
	inline map_item& operator=(_value_t const& value)noexcept { m_value=value; return *this; }
	inline map_item& operator=(_value_t&& value)noexcept { m_value=std::move(value); return *this; }

	// Comparison
	inline bool operator==(_key_t const& key)const noexcept { return m_key==key; }
	inline bool operator!=(_key_t const& key)const noexcept { return m_key!=key; }
	inline bool operator>(_key_t const& key)const noexcept { return m_key>key; }
	inline bool operator>=(_key_t const& key)const noexcept { return m_key>=key; }
	inline bool operator<(_key_t const& key)const noexcept { return m_key<key; }
	inline bool operator<=(_key_t const& key)const noexcept { return m_key<=key; }

	// Access
	inline _key_t const& get_key()const noexcept { return m_key; }
	inline _value_t& get_value()noexcept { return m_value; }
	inline _value_t const& get_value()const noexcept { return m_value; }

	// Modification
	template <typename _value_param_t> inline void set_value(_value_param_t&& value)noexcept
		{
		_value_t set(std::forward<_value_param_t>(value));
		m_value=std::move(set);
		}

private:
	// Common
	_key_t m_key;
	_value_t m_value;
};


//=====
// Map
//=====

template <typename _key_t, typename _value_t, typename _size_t=uint32_t, uint16_t _group_size=10>
class map: public cluster<map_traits<_key_t, _value_t, _size_t, _group_size>>
{
public:
	// Using
	using _traits_t=map_traits<_key_t, _value_t, _size_t, _group_size>;
	using _base_t=cluster<_traits_t>;
	using _item_t=typename _traits_t::item_t;
	using _group_t=typename _traits_t::group_t;
	using _item_group_t=typename _traits_t::item_group_t;
	using _parent_group_t=typename _traits_t::parent_group_t;
	using iterator=typename _traits_t::iterator_t;
	using const_iterator=typename _traits_t::const_iterator_t;

	// Con-/Destructors
	using _base_t::_base_t;

	// Access
	template <class _key_param_t> inline _value_t& operator[](_key_param_t&& key)noexcept { return get(std::forward<_key_param_t>(key)); }
	bool contains(_key_t const& key)const noexcept
		{
		auto root=this->m_root;
		if(!root)
			return false;
		return root->get(key)!=nullptr;
		}
	inline iterator find(_key_t const& key, bool above_or_equal=true)noexcept
		{
		iterator it(this);
		it.find(key);
		return it;
		}
	inline const_iterator find(_key_t const& key, bool above_or_equal)const noexcept
		{
		const_iterator it(this);
		it.find(key);
		return it;
		}
	template <class _key_param_t> _value_t& get(_key_param_t&& key)noexcept
		{
		_item_t create(std::forward<_key_param_t>(key), _value_t());
		bool created=false;
		_item_t* got=get_internal(&create, &created);
		return got->get_value();
		}
	_value_t get(_key_t const& key)const noexcept
		{
		auto root=this->m_root;
		if(!root)
			return _value_t();
		_item_t* item=root->get(key);
		if(!item)
			return _value_t();
		return item->get_value();
		}
	bool try_get(_key_t const& key, _value_t* value)const noexcept
		{
		auto root=this->m_root;
		if(!root)
			return false;
		_item_t* item=root->get(key);
		if(!item)
			return false;
		*value=item->get_value();
		return true;
		}

	// Modification
	template <typename _key_param_t, typename _value_param_t> bool add(_key_param_t&& key, _value_param_t&& value)noexcept
		{
		_item_t create(std::forward<_key_param_t>(key), std::forward<_value_param_t>(value));
		bool created=false;
		get_internal(&create, &created);
		return created;
		}
	bool remove(_key_t const& key)noexcept
		{
		auto root=this->m_root;
		if(!root)
			return false;
		return root->remove(key);
		}
	template <typename _key_param_t, typename _value_param_t> void set(_key_param_t&& key, _value_param_t&& value)noexcept
		{
		_item_t create(std::forward<_key_param_t>(key), std::forward<_value_param_t>(value));
		bool created=false;
		auto item=get_internal(&create, &created);
		if(!created)
			*item=std::move(create);
		}

private:
	// Common
	_item_t* get_internal(_item_t* create, bool* created)noexcept
		{
		auto root=this->create_root();
		_item_t* got=root->get(create->get_key(), create, created, false);
		if(got!=create)
			return got;
		root=this->lift_root();
		return root->get(create->get_key(), create, created, true);
		}
};


} // namespace

#endif // _CLUSTERS_MAP_H
