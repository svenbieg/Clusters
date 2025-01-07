//=========
// map.hpp
//=========

// Implementation of a sorted map
// Items can be inserted, removed and looked-up in constant low time

// Copyright 2025, Sven Bieg (svenbieg@web.de)
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

template <class _key_t, class _value_t, typename _size_t, uint16_t _group_size> class map;
template <class _key_t, class _value_t> class map_item;
template <class _traits_t, bool _is_const> class map_iterator;
template <class _traits_t, bool _is_const> class shared_map_iterator;

template <class _key_t, class _value_t, typename _size_t, uint16_t _group_size>
struct map_traits
{
using key_t=_key_t;
using item_t=map_item<_key_t, _value_t>;
using group_t=index_group<item_t, _size_t, _group_size>;
using item_group_t=index_item_group<item_t, _size_t, _group_size>;
using parent_group_t=index_parent_group<item_t, _size_t, _group_size>;
using cluster_t=map<_key_t, _value_t, _size_t, _group_size>;
using iterator_t=map_iterator<map_traits, false>;
using const_iterator_t=map_iterator<map_traits, true>;
using shared_iterator_t=shared_map_iterator<map_traits, false>;
using shared_const_iterator_t=shared_map_iterator<map_traits, true>;
using size_t=_size_t;
using value_t=_value_t;
static constexpr uint16_t group_size=_group_size;
};


//======
// Item
//======

template <class _key_t, class _value_t>
class map_item
{
public:
	// Con-/Destructors
	map_item() {}
	map_item(map_item const& item): m_key(item.m_key), m_value(item.m_value) {}
	map_item(map_item&& item)noexcept: m_key(std::move(item.m_key)), m_value(std::move(item.m_value)) {}
	map_item(_key_t const& key, _value_t const& value): m_key(key), m_value(value) {}

	// Assignment
	inline map_item& operator=(map_item const& item)
		{
		m_key=item.m_key;
		m_value=item.m_value;
		return *this;
		}

	// Comparison
	inline bool operator==(map_item const& item)const { return m_key==item.m_key; }
	inline bool operator!=(map_item const& item)const { return m_key!=item.m_key; }
	inline bool operator>(map_item const& item)const { return m_key>item.m_key; }
	inline bool operator>=(map_item const& item)const { return m_key>=item.m_key; }
	inline bool operator<(map_item const& item)const { return m_key<item.m_key; }
	inline bool operator<=(map_item const& item)const { return m_key<=item.m_key; }

	// Access
	inline _key_t const& get_key()const { return m_key; }
	inline _value_t& get_value() { return m_value; }
	inline _value_t const& get_value()const { return m_value; }

	// Modification
	inline void set_value(_value_t const& value) { m_value=value; }
	inline void set_value(_value_t&& value)
		{
		m_value.~_value_t();
		new (&m_value) _value_t(std::move(value));
		}

private:
	// Common
	_key_t m_key;
	_value_t m_value;
};


//=====
// Map
//=====

template <class _key_t, class _value_t, typename _size_t=uint32_t, uint16_t _group_size=10>
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
	map(map const& map): _base_t(nullptr)
		{
		this->copy_from(map);
		}

	// Access
	template <class _key_param_t> inline _value_t& operator[](_key_param_t const& key) { return get(key); }
	template <class _key_param_t> inline _value_t const& operator[](_key_param_t const& key)const { return get(key); }
	inline const_iterator cfind(_key_t const& key, find_func func=find_func::equal)const
		{
		const_iterator it(this);
		it.find(key, func);
		return it;
		}
	bool contains(_key_t const& key)const
		{
		_item_t item(key, _value_t());
		return get_internal(item)!=nullptr;
		}
	inline iterator find(_key_t const& key, find_func func=find_func::equal)
		{
		iterator it(this);
		it.find(key, func);
		return it;
		}
	template <class _key_param_t> _value_t& get(_key_param_t const& key)
		{
		_item_t item(key, _value_t());
		bool created=false;
		auto got=get_internal(std::forward<_item_t>(item), &created);
		return got->get_value();
		}
	template <class _key_param_t, class _value_param_t> _value_t& get(_key_param_t const& key, _value_param_t const& value)
		{
		_item_t item(key, value);
		bool created=false;
		auto got=get_internal(std::forward<_item_t>(item), &created);
		return got->get_value();
		}
	template <class _key_param_t> _value_t const& get(_key_param_t const& key)const
		{
		_item_t item(key, _value_t());
		auto got=get_internal(item);
		if(!got)
			throw std::out_of_range(nullptr);
		return got->get_value();
		}
	template <class _key_param_t> bool try_get(_key_param_t const& key, _value_t* value_ptr)const
		{
		_item_t item(key, _value_t());
		auto got=get_internal(item);
		if(!got)
			return false;
		if(value_ptr)
			*value_ptr=got->get_value();
		return true;
		}

	// Modification
	inline map& operator=(map const& map)
		{
		this->copy_from(map);
		return *this;
		}
	template <class _key_param_t, class _value_param_t> bool add(_key_param_t const& key, _value_param_t const& value)
		{
		_item_t item(key, value);
		bool created=false;
		get_internal(std::forward<_item_t>(item), &created);
		return created;
		}
	bool remove(_key_t const& key, _item_t* item_ptr=nullptr)
		{
		auto root=this->m_root;
		if(!root)
			return false;
		_item_t item(key, _value_t());
		return root->remove(item, item_ptr);
		}
	template <class _key_param_t, class _value_param_t> bool set(_key_param_t const& key, _value_param_t const& value)
		{
		_item_t item(key, value);
		bool created=false;
		auto got=get_internal(std::forward<_item_t>(item), &created);
		if(!created)
			got->set_value(std::forward<_value_t>(item.get_value()));
		return true;
		}

protected:
	// Con-/Destructors
	map(_group_t* root): _base_t(root) {}

private:
	// Common
	_item_t const* get_internal(_item_t const& item)const
		{
		auto root=this->m_root;
		if(!root)
			return nullptr;
		return root->get(item);
		}
	_item_t* get_internal(_item_t&& item, bool* created)
		{
		auto root=this->create_root();
		auto got=root->get(std::forward<_item_t>(item), created, false);
		if(got)
			return got;
		root=this->lift_root();
		return root->get(std::forward<_item_t>(item), created, true);
		}
};


//==========
// Iterator
//==========

template <class _traits_t, bool _is_const>
class map_iterator: public index_iterator<_traits_t, _is_const>
{
public:
	// Using
	using _base_t=index_iterator<_traits_t, _is_const>;
	using _item_t=typename _traits_t::item_t;
	using _key_t=typename _traits_t::key_t;
	using _value_t=typename _traits_t::value_t;
	using _value_ref=typename std::conditional<_is_const, _value_t const&, _value_t&>::type;

	// Con-/Destructors
	using _base_t::_base_t;

	// Access
	inline _key_t const& get_key() { return _base_t::get_current().get_key(); }
	inline _value_ref get_value() { return _base_t::get_current().get_value(); }

	// Navigation
	template <class _key_param_t> inline bool find(_key_param_t const& key, find_func func=find_func::equal)
		{
		_item_t item(key, _value_t());
		return _base_t::find(item, func);
		}
};

}