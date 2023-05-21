//===========
// index.hpp
//===========

// Implementation of a sorted list
// Items can be inserted, removed and looked-up in constant low time

// Copyright 2022, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _CLUSTERS_INDEX_HPP
#define _CLUSTERS_INDEX_HPP


//=======
// Using
//=======

#include "cluster.hpp"


//===========
// Namespace
//===========

namespace Clusters {


//===============
// Find-Function
//===============

enum class find_func
{
above,
above_or_equal,
any,
below,
below_or_equal,
equal
};


//======================
// Forward-Declarations
//======================

template <typename _item_t, typename _size_t, uint16_t _group_size> class index;
template <typename _key_t, typename _item_t, typename _size_t, uint16_t _group_size> class index_group;
template <typename _key_t, typename _item_t, typename _size_t, uint16_t _group_size> class index_item_group;
template <typename _key_t, typename _item_t, typename _size_t, uint16_t _group_size> class index_parent_group;
template <typename _traits_t, bool _is_const> class index_iterator;
template <typename _traits_t, bool _is_const> class shared_index_iterator;

template <typename _key_t, typename _item_t, typename _size_t, uint16_t _group_size>
struct index_traits
{
using key_t=_key_t;
using item_t=_item_t;
using group_t=index_group<_key_t, _item_t, _size_t, _group_size>;
using item_group_t=index_item_group<_key_t, _item_t, _size_t, _group_size>;
using parent_group_t=index_parent_group<_key_t, _item_t, _size_t, _group_size>;
using cluster_t=index<_item_t, _size_t, _group_size>;
using iterator_t=index_iterator<index_traits, false>;
using const_iterator_t=index_iterator<index_traits, true>;
using shared_iterator_t=shared_index_iterator<index_traits, false>;
using shared_const_iterator_t=shared_index_iterator<index_traits, true>;
using size_t=_size_t;
static constexpr uint16_t group_size=_group_size;
};


//=======
// Group
//=======

template <typename _key_t, typename _item_t, typename _size_t, uint16_t _group_size>
class index_group: public cluster_group<index_traits<_key_t, _item_t, _size_t, _group_size>>
{
public:
	// Access
	virtual uint16_t find(_key_t const& key, bool* exists, find_func func)const noexcept=0;
	virtual _item_t* get(_key_t const& key)noexcept=0;
	virtual _item_t* get(_key_t const& key, _item_t&& create, bool* created, bool again)noexcept=0;
	virtual _item_t* get_first()noexcept=0;
	virtual _item_t* get_last()noexcept=0;

	// Modification
	virtual bool remove(_key_t const& key, _item_t* item_ptr)noexcept=0;
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
	inline uint16_t find(_key_t const& key, bool* exists_ptr, find_func func)const noexcept override
		{
		bool exists=false;
		uint16_t pos=get_item_pos(key, &exists);
		if(exists)
			{
			*exists_ptr=true;
			switch(func)
				{
				case find_func::above:
					{
					if(pos+1>=this->m_item_count)
						return _group_size;
					return pos+1;
					}
				case find_func::below:
					{
					if(pos==0)
						return _group_size;
					return pos-1;
					}
				default:
					{
					break;
					}
				}
			return pos;
			}
		switch(func)
			{
			case find_func::above:
			case find_func::above_or_equal:
				{
				if(pos==this->m_item_count)
					return _group_size;
				return pos;
				}
			case find_func::any:
				{
				if(pos>0)
					return pos-1;
				break;
				}
			case find_func::below:
			case find_func::below_or_equal:
				{
				if(pos==0)
					return _group_size;
				return pos-1;
				}
			case find_func::equal:
				{
				return _group_size;
				}
			}
		return pos;
		}
	_item_t* get(_key_t const& key)noexcept override
		{
		bool exists=false;
		uint16_t pos=get_item_pos(key, &exists);
		if(!exists)
			return nullptr;
		return this->get_at(pos);
		}
	_item_t* get(_key_t const& key, _item_t&& create, bool* created, bool again)noexcept override
		{
		bool exists=false;
		uint16_t pos=get_item_pos(key, &exists);
		if(exists)
			return this->get_at(pos);
		_item_t* inserted=this->insert_item(pos, std::forward<_item_t>(create));
		if(inserted)
			{
			*created=true;
			return inserted;
			}
		return nullptr;
		}
	inline _item_t* get_first()noexcept override { return this->get_first_item(); }
	inline _item_t* get_last()noexcept override { return this->get_last_item(); }

	// Modification
	bool remove(_key_t const& key, _item_t* item_ptr)noexcept override
		{
		bool exists=false;
		uint16_t pos=get_item_pos(key, &exists);
		if(!exists)
			return false;
		return this->remove_at(pos, item_ptr);
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
	uint16_t find(_key_t const& key, bool* exists_ptr, find_func func)const noexcept override
		{
		uint16_t pos=0;
		uint16_t count=get_item_pos(key, &pos, false);
		if(count==2)
			{
			switch(func)
				{
				case find_func::above:
				case find_func::above_or_equal:
					return pos+1;
				case find_func::equal:
					return _group_size;
				default:
					break;
				}
			return pos;
			}
		switch(func)
			{
			case find_func::above:
				{
				auto group=this->get_child(pos);
				auto last=group->get_last();
				if(*last==key)
					{
					if(pos+1>=this->m_child_count)
						return _group_size;
					return pos+1;
					}
				break;
				}
			case find_func::below:
				{
				auto group=this->get_child(pos);
				auto first=group->get_first();
				if(*first==key)
					{
					if(pos==0)
						return _group_size;
					return pos-1;
					}
				break;
				}
			default:
				break;
			}
		return pos;
		}
	_item_t* get(_key_t const& key)noexcept override
		{
		uint16_t pos=0;
		uint16_t count=get_item_pos(key, &pos, true);
		for(uint16_t u=0; u<count; u++)
			{
			_item_t* item=this->m_children[pos+u]->get(key);
			if(item)
				return item;
			}
		return nullptr;
		}
	_item_t* get(_key_t const& key, _item_t&& create, bool* created, bool again)noexcept override
		{
		bool created_internal=false;
		_item_t* item=get_internal(key, std::forward<_item_t>(create), &created_internal, again);
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
	bool remove(_key_t const& key, _item_t* item_ptr)noexcept override
		{
		uint16_t pos=0;
		uint16_t count=get_item_pos(key, &pos, true);
		if(count!=1)
			return false;
		if(!this->m_children[pos]->remove(key, item_ptr))
			return false;
		this->m_item_count--;
		this->combine_children(pos);
		update_bounds();
		return true;
		}
	bool remove_at(_size_t position, _item_t* item_ptr)noexcept override
		{
		if(!_base_t::remove_at(position, item_ptr))
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
	_item_t* get_internal(_key_t const& key, _item_t&& create, bool* created, bool again)noexcept
		{
		uint16_t pos=0;
		uint16_t count=get_item_pos(key, &pos, false);
		if(!again)
			{
			for(uint16_t u=0; u<count; u++)
				{
				_item_t* item=this->m_children[pos+u]->get(key, std::forward<_item_t>(create), created, false);
				if(item)
					return item;
				}
			if(this->shift_children(pos, count))
				{
				count=get_item_pos(key, &pos, false);
				for(uint16_t u=0; u<count; u++)
					{
					_item_t* item=this->m_children[pos+u]->get(key, std::forward<_item_t>(create), created, false);
					if(item)
						return item;
					}
				}
			}
		if(!this->split_child(pos))
			return nullptr;
		count=get_item_pos(key, &pos, false);
		for(uint16_t u=0; u<count; u++)
			{
			_item_t* item=this->m_children[pos+u]->get(key, std::forward<_item_t>(create), created, false);
			if(item)
				return item;
			}
		return nullptr;
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
class index: public cluster<index_traits<_item_t, _item_t, _size_t, _group_size>>
{
public:
	// Using
	using _traits_t=index_traits<_item_t, _item_t, _size_t, _group_size>;
	using _base_t=cluster<_traits_t>;
	using _group_t=typename _traits_t::group_t;
	using iterator=typename _traits_t::iterator_t;
	using const_iterator=typename _traits_t::const_iterator_t;

	// Con-/Destructors
	index(): _base_t(nullptr) {}
	index(index&& index)noexcept: _base_t(index.m_root)
		{
		index.m_root=nullptr;
		}
	index(index const& index): _base_t(nullptr)
		{
		copy_from(index);
		}

	// Access
	inline const_iterator cfind(_item_t const& item, find_func func=find_func::equal)const noexcept
		{
		const_iterator it(this);
		it.find(item, func);
		return it;
		}
	bool contains(_item_t const& item)const noexcept
		{
		auto root=this->m_root;
		if(!root)
			return false;
		return root->get(item)!=nullptr;
		}
	inline iterator find(_item_t const& item, find_func func=find_func::equal)noexcept
		{
		iterator it(this);
		it.find(item, func);
		return it;
		}

	// Modification
	index& operator=(index&& index)noexcept
		{
		this->clear();
		this->m_root=index.m_root;
		index.m_root=nullptr;
		return *this;
		}
	inline index& operator=(index const& index)noexcept
		{
		this->copy_from(index);
		return *this;
		}
	template <typename _item_param_t> bool add(_item_param_t&& item)noexcept
		{
		_item_t create(std::forward<_item_param_t>(item));
		bool created=false;
		get_internal(create, &created);
		return created;
		}
	bool remove(_item_t const& item, _item_t* item_ptr=nullptr)
		{
		auto root=this->m_root;
		if(!root)
			return false;
		return root->remove(item, item_ptr);
		}
	template <typename _item_param_t> bool set(_item_param_t&& item)noexcept
		{
		_item_t create(std::forward<_item_param_t>(item));
		bool created=false;
		get_internal(std::forward<_item_t>(create), &created);
		if(!created)
			return false;
		return true;
		}

protected:
	// Con-/Destructors
	index(_group_t* root): _base_t(root) {}

private:
	// Common
	_item_t* get_internal(_item_t&& create, bool* created)
		{
		auto root=this->create_root();
		_item_t* got=root->get(create, std::forward<_item_t>(create), created, false);
		if(got)
			return got;
		root=this->lift_root();
		return root->get(create, std::forward<_item_t>(create), created, true);
		}
};


//==========
// Iterator
//==========

template <typename _traits_t, bool _is_const>
class index_iterator: public cluster_iterator<_traits_t, _is_const>
{
public:
	// Using
	using _base_t=cluster_iterator<_traits_t, _is_const>;
	using _key_t=typename _traits_t::key_t;
	using _item_group_t=typename _traits_t::item_group_t;
	using _parent_group_t=typename _traits_t::parent_group_t;
	using _size_t=typename _traits_t::size_t;
	static constexpr uint16_t _group_size=_traits_t::group_size;

	// Con-/Destructors
	using _base_t::_base_t;

public:
	// Navigation
	bool find(_key_t const& key, find_func func=find_func::equal)noexcept
		{
		auto group=this->m_cluster->get_root();
		if(!group)
			{
			this->end();
			return false;
			}
		uint16_t level_count=(uint16_t)(group->get_level()+1);
		this->set_level_count(level_count);
		auto it_ptr=&this->m_its[0];
		this->m_position=0;
		bool exists=false;
		while(group)
			{
			uint16_t group_pos=group->find(key, &exists, func);
			if(group_pos==_group_size)
				break;
			it_ptr->group=group;
			it_ptr->position=group_pos;
			it_ptr++;
			if(group->get_level()>0)
				{
				auto parent_group=(_parent_group_t*)group;
				group=parent_group->get_child(group_pos);
				for(uint16_t u=0; u<group_pos; u++)
					this->m_position+=parent_group->get_child(u)->get_item_count();
				continue;
				}
			auto item_group=(_item_group_t*)group;
			this->m_current=item_group->get_at(group_pos);
			this->m_position+=group_pos;
			return true;
			}
		this->end();
		return false;
		}
};


} // namespace

#endif // _CLUSTERS_INDEX_HPP
