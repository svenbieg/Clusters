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


//==========
// Map-Item
//==========

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
	_key_t const& get_key()const noexcept { return m_key; }
	_value_t& get_value()noexcept { return m_value; }
	_value_t const& get_value()const noexcept { return m_value; }

	// Modification
	template <typename _value_param_t> void set_value(_value_param_t&& value)noexcept
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
class map: public iterable_cluster<map_item<_key_t, _value_t>, index_group<_key_t, map_item<_key_t, _value_t>, _size_t>, index_item_group<_key_t, map_item<_key_t, _value_t>, _size_t, _group_size>, index_parent_group<_key_t, map_item<_key_t, _value_t>, _size_t, _group_size>, _size_t, _group_size>
{
public:
	// Using
	using _item_t=map_item<_key_t, _value_t>;
	using _group_t=index_group<_key_t, _item_t, _size_t>;
	using _item_group_t=index_item_group<_key_t, _item_t, _size_t, _group_size>;
	using _parent_group_t=index_parent_group<_key_t, _item_t, _size_t, _group_size>;
	using _base_t=iterable_cluster<_item_t, _group_t, _item_group_t, _parent_group_t, _size_t, _group_size>;
	using iterator=typename _base_t::iterator;
	using const_iterator=typename _base_t::const_iterator;

	// Con-/Destructors
	using _base_t::_base_t;

	// Access
	template <class _key_param_t> inline _value_t& operator[](_key_param_t&& key)noexcept
		{
		return get(std::forward<_key_param_t>(key));
		}
	inline _value_t operator[](_key_t const& key)const noexcept { return get(key); }
	bool contains(_key_t const& key)const noexcept
		{
		auto root=this->m_root;
		if(!root)
			return false;
		return root->get(key)!=nullptr;
		}
	inline iterator find(_key_t const& key)noexcept { return search_internal(key, true); }
	template <class _key_param_t> _value_t& get(_key_param_t&& key)noexcept
		{
		_item_t create(std::forward<_key_param_t>(key), _value_t());
		bool created=false;
		_item_t* got=get_internal(&create, &created);
		return got->get_value();
		}
	_value_t get(_key_t const& key, bool read_only=true)const noexcept
		{
		auto root=this->m_root;
		if(!root)
			return _value_t();
		_item_t* item=root->get(key);
		if(!item)
			return _value_t();
		return item->get_value();
		}
	inline iterator search(_key_t const& key)noexcept { return search_internal(key, false); }

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
		get_internal(&create, &created);
		}

private:
	// Common
	_item_t* get_internal(_item_t* item, bool* created)noexcept
		{
		auto root=this->create_root();
		_item_t* got=root->get(item->get_key(), item, created, false);
		if(got!=item)
			return got;
		root=this->lift_root();
		return root->get(item->get_key(), item, created, true);
		}
	iterator search_internal(_key_t const& key, bool find)noexcept
		{
		_size_t position=0;
		bool exists=false;
		auto group=this->m_root;
		while(group)
			{
			uint16_t group_pos=group->search(key, &position, &exists);
			if(group->get_level()>0)
				{
				auto parent_group=(_parent_group_t*)group;
				group=parent_group->get_child(group_pos);
				continue;
				}
			if(find&&!exists)
				break;
			return iterator(this, position, (_item_group_t*)group, group_pos);
			}
		return this->end();
		}
};


} // namespace

#endif // _CLUSTERS_MAP_H
