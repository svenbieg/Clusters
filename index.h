//=========
// index.h
//=========

// Implementation of a sorted list
// Items can be inserted, removed and looked-up in constant low time

// Copyright 2022, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _CLUSTERS_INDEX_H
#define _CLUSTERS_INDEX_H


//=======
// Using
//=======

#include "cluster.h"


//===========
// Namespace
//===========

namespace Clusters {


//======================
// Forward-Declarations
//======================

template <typename _item_t, typename _size_t, uint16_t _group_size> class index;
template <typename _key_t, typename _item_t, typename _size_t, uint16_t _group_size> class index_group;
template <typename _key_t, typename _item_t, typename _size_t, uint16_t _group_size> class index_item_group;
template <typename _key_t, typename _item_t, typename _size_t, uint16_t _group_size> class index_parent_group;

template <typename _key_t, typename _item_t, typename _size_t, uint16_t _group_size>
struct index_traits
{
using item_t=_item_t;
using group_t=index_group<_key_t, _item_t, _size_t, _group_size>;
using item_group_t=index_item_group<_key_t, _item_t, _size_t, _group_size>;
using parent_group_t=index_parent_group<_key_t, _item_t, _size_t, _group_size>;
using cluster_t=index<_item_t, _size_t, _group_size>;
using size_t=_size_t;
static constexpr uint16_t group_size=_group_size;
};


//=============
// Find-Method
//=============

enum class index_find_method
{
below_or_equal,
above_or_equal
};


//=======
// Group
//=======

template <typename _key_t, typename _item_t, typename _size_t, uint16_t _group_size>
class index_group: public cluster_group<index_traits<_key_t, _item_t, _size_t, _group_size>>
{
public:
	// Access
	virtual uint16_t find(_key_t const& key, _size_t* position, bool* exists, index_find_method method)const noexcept=0;
	virtual _item_t* get(_key_t const& key, _item_t* create=nullptr, bool* created=nullptr, bool again=false)noexcept=0;
	virtual _item_t* get_first()noexcept=0;
	virtual _item_t* get_last()noexcept=0;

	// Modification
	virtual bool remove(_key_t const& key)noexcept=0;
};


//============
// Item-Group
//============

template <typename _key_t, typename _item_t, typename _size_t, uint16_t _group_size>
class index_item_group: public cluster_item_group<index_traits<_key_t, _item_t, _size_t, _group_size>>
{
public:
	// Using
	using _traits_t=index_traits<_key_t, _item_t, _size_t, _group_size>;
	using _base_t=cluster_item_group<_traits_t>;
	using _group_t=typename _traits_t::group_t;
	using _item_group_t=typename _traits_t::item_group_t;

	// Con-/Destructors
	using _base_t::_base_t;

	// Access
	inline uint16_t find(_key_t const& key, _size_t* position, bool* exists, index_find_method method)const noexcept override
		{
		uint16_t pos=get_item_pos(key, exists);
		if(!(*exists))
			{
			switch(method)
				{
				case index_find_method::below_or_equal:
					{
					if(pos==0)
						return _group_size;
					pos--;
					break;
					}
				case index_find_method::above_or_equal:
					{
					uint16_t item_count=this->m_item_count;
					if(pos==item_count)
						return _group_size;
					break;
					}
				}
			}
		*position+=pos;
		return pos;
		}
	_item_t* get(_key_t const& key, _item_t* create, bool* created, bool again)noexcept override
		{
		bool exists=false;
		uint16_t pos=get_item_pos(key, &exists);
		if(exists)
			return this->get_at(pos);
		if(!create)
			return nullptr;
		if(!this->insert_items(pos, create, 1))
			return create;
		*created=true;
		return this->get_at(pos);
		}
	inline _item_t* get_first()noexcept override { return this->get_first_item(); }
	inline _item_t* get_last()noexcept override { return this->get_last_item(); }

	// Modification
	bool remove(_key_t const& key)noexcept override
		{
		bool exists=false;
		uint16_t pos=get_item_pos(key, &exists);
		if(!exists)
			return false;
		return this->remove_at(pos);
		}

private:
	// Access
	uint16_t get_item_pos(_key_t const& key, bool* exists)const noexcept
		{
		uint16_t item_count=this->m_item_count;
		if(item_count==0)
			return 0;
		uint16_t start=0;
		uint16_t end=item_count;
		while(start<end)
			{
			uint16_t pos=(uint16_t)(start+(end-start)/2);
			_item_t const* item=this->get_at(pos);
			if(*item>key)
				{
				end=pos;
				continue;
				}
			if(*item<key)
				{
				start=(uint16_t)(pos+1);
				continue;
				}
			*exists=true;
			return pos;
			}
		return start;
		}
};


//==============
// Parent-Group
//==============

template <typename _key_t, typename _item_t, typename _size_t, uint16_t _group_size>
class index_parent_group: public cluster_parent_group<index_traits<_key_t, _item_t, _size_t, _group_size>>
{
public:
	// Using
	using _traits_t=index_traits<_key_t, _item_t, _size_t, _group_size>;
	using _base_t=cluster_parent_group<_traits_t>;
	using _group_t=typename _traits_t::group_t;
	using _item_group_t=typename _traits_t::item_group_t;
	using _parent_group_t=typename _traits_t::parent_group_t;

	// Con-Destructors
	index_parent_group(uint16_t level=0)noexcept: _base_t(level), m_first(nullptr), m_last(nullptr) {}
	index_parent_group(_parent_group_t const& group)noexcept: _base_t(group)
		{
		uint16_t last=(uint16_t)(this->m_child_count-1);
		m_first=this->m_children[0]->get_first();
		m_last=this->m_children[last]->get_last();
		}

	// Access
	inline uint16_t find(_key_t const& key, _size_t* position, bool* exists, index_find_method method)const noexcept override
		{
		uint16_t pos=0;
		uint16_t count=get_item_pos(key, &pos, false);
		if(method==index_find_method::above_or_equal)
			{
			if(count>1)
				pos++;
			}
		for(uint16_t u=0; u<pos; u++)
			*position+=this->m_children[u]->get_item_count();
		return pos;
		}
	_item_t* get(_key_t const& key, _item_t* create, bool* created, bool again)noexcept override
		{
		bool created_internal=false;
		_item_t* item=get_internal(key, create, &created_internal, again);
		if(created_internal)
			{
			this->m_item_count++;
			update_bounds();
			}
		if(created)
			*created=created_internal;
		return item;
		}
	inline _item_t* get_first()noexcept override { return m_first; }
	inline _item_t* get_last()noexcept override { return m_last; }

	// Modification
	_size_t insert_groups(uint16_t position, _group_t* const* groups, uint16_t count)noexcept override
		{
		_size_t item_count=_base_t::insert_groups(position, groups, count);
		update_bounds();
		return item_count;
		}
	bool remove(_key_t const& key)noexcept override
		{
		uint16_t pos=0;
		uint16_t count=get_item_pos(key, &pos, true);
		if(count==0)
			return false;
		if(!this->m_children[pos]->remove(key))
			return false;
		this->m_item_count--;
		this->combine_children(pos);
		update_bounds();
		return true;
		}
	bool remove_at(_size_t position)noexcept override
		{
		if(!_base_t::remove_at(position))
			return false;
		update_bounds();
		return true;
		}
	void remove_groups(uint16_t position, uint16_t count, _size_t item_count)noexcept override
		{
		_base_t::remove_groups(position, count, item_count);
		update_bounds();
		}
	void set_child(_group_t* child)override
		{
		_base_t::set_child(child);
		m_first=this->m_children[0]->get_first();
		m_last=this->m_children[0]->get_last();
		}

private:
	// Access
	uint16_t get_item_pos(_key_t const& key, uint16_t* group, bool must_exist)const noexcept
		{
		uint16_t child_count=this->m_child_count;
		uint16_t start=0;
		uint16_t end=child_count;
		while(start<end)
			{
			uint16_t pos=(uint16_t)(start+(end-start)/2);
			auto child=this->m_children[pos];
			_item_t* first=child->get_first();
			_item_t* last=child->get_last();
			if(*first>key)
				{
				end=pos;
				continue;
				}
			if(*last<key)
				{
				start=(uint16_t)(pos+1);
				continue;
				}
			*group=pos;
			return 1;
			}
		if(must_exist)
			return 0;
		if(start>=child_count)
			start=(uint16_t)(child_count-1);
		*group=start;
		if(start>0)
			{
			auto child=this->m_children[start];
			_item_t* first=child->get_first();
			if(*first>key)
				{
				*group=(uint16_t)(start-1);
				return 2;
				}
			}
		if(start+1<child_count)
			{
			auto child=this->m_children[start];
			_item_t* last=child->get_last();
			if(*last<key)
				return 2;
			}
		return 1;
		}

	// Modification
	_item_t* get_internal(_key_t const& key, _item_t* create, bool* created, bool again)noexcept
		{
		BOOL must_exist=(create==nullptr);
		uint16_t pos=0;
		uint16_t count=get_item_pos(key, &pos, must_exist);
		if(count==0)
			return nullptr;
		if(!again)
			{
			for(uint16_t u=0; u<count; u++)
				{
				_item_t* item=this->m_children[pos+u]->get(key, create, created, false);
				if(item!=create)
					return item;
				}
			if(this->shift_children(pos, count))
				{
				count=get_item_pos(key, &pos, false);
				for(uint16_t u=0; u<count; u++)
					{
					_item_t* item=this->m_children[pos+u]->get(key, create, created, false);
					if(item!=create)
						return item;
					}
				}
			}
		if(!this->split_child(pos))
			return create;
		count=get_item_pos(key, &pos, false);
		for(uint16_t u=0; u<count; u++)
			{
			_item_t* item=this->m_children[pos+u]->get(key, create, created, false);
			if(item!=create)
				return item;
			}
		return create;
		}
	void update_bounds()noexcept
		{
		if(this->m_child_count==0)
			{
			m_first=nullptr;
			m_last=nullptr;
			return;
			}
		m_first=this->m_children[0]->get_first();
		m_last=this->m_children[(uint16_t)(this->m_child_count-1)]->get_last();
		}
	
	// Common
	_item_t* m_first;
	_item_t* m_last;
};


//=======
// Index
//=======

template <typename _item_t, typename _size_t=uint32_t, uint16_t _group_size=10>
class index: public iterable_cluster<index_traits<_item_t, _item_t, _size_t, _group_size>>
{
public:
	// Using
	using _traits_t=index_traits<_item_t, _item_t, _size_t, _group_size>;
	using _base_t=iterable_cluster<_traits_t>;
	using _group_t=typename _traits_t::group_t;
	using _item_group_t=typename _traits_t::item_group_t;
	using _parent_group_t=typename _traits_t::parent_group_t;
	using _cluster_pos_t=cluster_position<_group_t>;
	using iterator=typename _base_t::iterator;
	using const_iterator=typename _base_t::const_iterator;

	// Con-/Destructors
	using _base_t::_base_t;

	// Access
	inline _item_t& operator[](_size_t position) { return this->get_at(position); }
	inline _item_t const& operator[](_size_t position)const { return this->get_at(position); }
	bool contains(_item_t const& item)const noexcept
		{
		auto root=this->m_root;
		if(!root)
			return false;
		return root->get(item)!=nullptr;
		}
	inline iterator find(_item_t const& item, index_find_method method=index_find_method::above_or_equal)noexcept { return find_internal<iterator>(item, method); }
	inline const_iterator find(_item_t const& item, index_find_method method=index_find_method::above_or_equal)const noexcept { return find_internal<const_iterator>(item, method); }

	// Modification
	template <typename _item_param_t> bool add(_item_param_t&& item)noexcept
		{
		_item_t create(std::forward<_item_param_t>(item));
		bool created=false;
		get_internal(&create, &created);
		return created;
		}
	bool remove(_item_t const& item)
		{
		auto root=this->m_root;
		if(!root)
			return false;
		return root->remove(item);
		}
	template <typename _item_param_t> void set(_item_param_t&& item)noexcept
		{
		_item_t create(std::forward<_item_param_t>(item));
		bool created=false;
		auto item=get_internal(&create, &created);
		if(!created)
			*item=std::move(create);
		}

private:
	// Common
	template <class _it_t> _it_t find_internal(_item_t const& item, index_find_method method)const noexcept
		{
		auto group=this->m_root;
		if(!group)
			return _it_t((_base_t*)this, -2);
		uint16_t level_count=group->get_level()+1;
		auto pointers=(_cluster_pos_t*)operator new(level_count*sizeof(_cluster_pos_t));
		auto pos_ptr=&pointers[0];
		_size_t position=0;
		bool exists=false;
		while(group)
			{
			uint16_t group_pos=group->find(item, &position, &exists, method);
			if(group_pos==_group_size)
				break;
			pos_ptr->group=group;
			pos_ptr->position=group_pos;
			pos_ptr++;
			if(group->get_level()>0)
				{
				auto parent_group=(_parent_group_t*)group;
				group=parent_group->get_child(group_pos);
				continue;
				}
			return _it_t((_base_t*)this, position, pointers, level_count);
			}
		operator delete(pointers);
		return _it_t((_base_t*)this, -2);
		}
	_item_t* get_internal(_item_t* create, bool* created)
		{
		auto root=this->create_root();
		_item_t* got=root->get(*create, create, created, false);
		if(got!=create)
			return got;
		root=this->lift_root();
		return root->get(*create, create, created, true);
		}
};


} // namespace

#endif // _CLUSTERS_INDEX_H
