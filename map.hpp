//=========
// map.hpp
//=========

// Implementation of a sorted map
// Items can be inserted, removed and looked-up in constant low time

// Copyright 2024, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/Clusters


#pragma once


//=======
// Using
//=======

#include "index.hpp"


//===========
// Namespace
//===========

namespace Collections {


//======================
// Forward-Declarations
//======================

template <typename _key_t, typename _value_t, typename _size_t, uint16_t _group_size> class map;
template <typename _key_t, typename _value_t> class map_item;
template <typename _traits_t, bool _is_const> class shared_map_iterator;

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
using shared_iterator_t=shared_map_iterator<map_traits, false>;
using shared_const_iterator_t=shared_map_iterator<map_traits, true>;
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
		m_key.~_key_t();
		m_value.~_value_t();
		new (&m_key) _key_t(std::move(item.m_key));
		new (&m_value) _value_t(std::move(item.m_value));
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
	map(): _base_t(nullptr) {}
	map(map&& map)noexcept: _base_t(map.m_root)
		{
		map.m_root=nullptr;
		}
	map(map const& map): _base_t(nullptr)
		{
		copy_from(map);
		}

	// Access
	template <class _key_param_t> inline _value_t& operator[](_key_param_t&& key)noexcept { return get(std::forward<_key_param_t>(key)); }
	inline const_iterator cfind(_key_t const& key, find_func func=find_func::equal)const noexcept
		{
		const_iterator it(this);
		it.find(key, func);
		return it;
		}
	bool contains(_key_t const& key)const noexcept
		{
		auto root=this->m_root;
		if(!root)
			return false;
		return root->get(key)!=nullptr;
		}
	inline iterator find(_key_t const& key, find_func func=find_func::equal)noexcept
		{
		iterator it(this);
		it.find(key, func);
		return it;
		}
	template <class _key_param_t> _value_t& get(_key_param_t&& key)noexcept
		{
		_item_t create(std::forward<_key_param_t>(key), _value_t());
		bool created=false;
		_item_t* got=get_internal(std::forward<_item_t>(create), &created);
		return got->get_value();
		}
	template <class _key_param_t, class _value_param_t> _value_t& get(_key_param_t&& key, _value_param_t&& init)noexcept
		{
		_item_t create(std::forward<_key_param_t>(key), std::forward<_value_param_t>(init));
		bool created=false;
		_item_t* got=get_internal(std::forward<_item_t>(create), &created);
		return got->get_value();
		}
	_value_t& get(_key_t const& key)const
		{
		auto root=this->m_root;
		if(!root)
			throw std::out_of_range(nullptr);
		_item_t* item=root->get(key);
		if(!item)
			throw std::out_of_range(nullptr);
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
	map& operator=(map&& map)noexcept
		{
		this->clear();
		this->m_root=map.m_root;
		map.m_root=nullptr;
		return *this;
		}
	inline map& operator=(map const& map)noexcept
		{
		this->copy_from(map);
		return *this;
		}
	template <typename _key_param_t, typename _value_param_t> bool add(_key_param_t&& key, _value_param_t&& value)noexcept
		{
		_item_t create(std::forward<_key_param_t>(key), std::forward<_value_param_t>(value));
		bool created=false;
		get_internal(std::forward<_item_t>(create), &created);
		return created;
		}
	bool remove(_key_t const& key, _item_t* item_ptr=nullptr)noexcept
		{
		auto root=this->m_root;
		if(!root)
			return false;
		return root->remove(key, item_ptr);
		}
	template <typename _key_param_t, typename _value_param_t> bool set(_key_param_t&& key, _value_param_t&& value)noexcept
		{
		_item_t create(std::forward<_key_param_t>(key), std::forward<_value_param_t>(value));
		bool created=false;
		auto got=get_internal(std::forward<_item_t>(create), &created);
		if(created)
			return true;
		if(got->get_value()==create.get_value())
			return false;
		*got=std::move(create);
		return true;
		}

protected:
	// Con-/Destructors
	map(_group_t* root): _base_t(root) {}

private:
	// Common
	_item_t* get_internal(_item_t&& create, bool* created)noexcept
		{
		auto root=this->create_root();
		_item_t* got=root->get(create.get_key(), std::forward<_item_t>(create), created, false);
		if(got)
			return got;
		root=this->lift_root();
		return root->get(create.get_key(), std::forward<_item_t>(create), created, true);
		}
};

}