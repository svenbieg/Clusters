//===========
// cluster.h
//===========

// Shared classes for list and index
// Items are stored in a pyramidal directory

// Copyright 2022, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _CLUSTERS_CLUSTER_H
#define _CLUSTERS_CLUSTER_H


//=======
// Using
//=======

#include <new>
#include <stdexcept>
#include <stdint.h>
#include <utility>


//===========
// Namespace
//===========

namespace Clusters {


//===============
// Cluster-Group
//===============

template <typename _item_t, typename _size_t>
class cluster_group
{
public:
	// Con-Destructors
	virtual ~cluster_group()noexcept {}

	// Access
	virtual _item_t* get_at(_size_t position)noexcept=0;
	virtual _item_t const* get_at(_size_t position)const noexcept=0;
	virtual uint16_t get_child_count()const noexcept=0;
	virtual _size_t get_item_count()const noexcept=0;
	virtual uint16_t get_level()const noexcept=0;

	// Modification
	virtual bool remove_at(_size_t position)noexcept=0;
};


//====================
// Cluster-Item-Group
//====================

template <typename _item_t, typename _group_t, typename _item_group_t, typename _size_t, uint16_t _group_size>
class cluster_item_group: public _group_t
{
public:
	// Con-/Destructors
	cluster_item_group()noexcept: m_item_count(0) {}
	cluster_item_group(cluster_item_group const& group)noexcept: m_item_count(group.m_item_count)
		{
		_item_t* items=get_items();
		_item_t const* copy=group.get_items();
		for(uint16_t u=0; u<m_item_count; u++)
			new (&items[u]) _item_t(copy[u]);
		}
	~cluster_item_group()
		{
		auto items=get_items();
		for(uint16_t u=0; u<m_item_count; u++)
			items[u].~_item_t();
		}

	// Access
	inline _item_t* get_at(_size_t position)noexcept override { return &get_items()[position]; }
	inline _item_t const* get_at(_size_t position)const noexcept override { return &get_items()[position]; }
	inline uint16_t get_child_count()const noexcept override { return m_item_count; }
	inline _item_t* get_first_item()noexcept { return &get_items()[0]; }
	inline _size_t get_item_count()const noexcept override { return m_item_count; }
	inline _item_t* get_items()noexcept { return (_item_t*)m_items; }
	inline _item_t const* get_items()const noexcept { return (_item_t const*)m_items; }
	inline _item_t* get_last_item()noexcept { return &get_items()[m_item_count-1]; }
	inline uint16_t get_level()const noexcept override { return 0; }

	// Modification
	bool insert_items(uint16_t position, _item_t* insert, uint16_t count)
		{
		if(m_item_count+count>_group_size)
			return false;
		if(position>m_item_count)
			return false;
		_item_t* items=get_items();
		if(position==m_item_count)
			{
			for(uint16_t u=0; u<count; u++)
				new (&items[position+u]) _item_t(std::forward<_item_t>(insert[u]));
			}
		else
			{
			uint16_t u=(uint16_t)(m_item_count+count-1);
			for(; u>=m_item_count; u--)
				new (&items[u]) _item_t(std::move(items[u-count]));
			for(; u>=position+count; u--)
				items[u]=std::move(items[u-count]);
			for(uint16_t u=0; u<count; u++)
				items[position+u]=std::forward<_item_t>(insert[u]);
			}
		m_item_count+=count;
		return true;
		}
	bool insert_items(uint16_t position, _item_t const* insert, uint16_t count)
		{
		if(m_item_count+count>_group_size)
			return false;
		if(position>m_item_count)
			return false;
		_item_t* items=get_items();
		if(position==m_item_count)
			{
			for(uint16_t u=0; u<count; u++)
				new (&items[position+u]) _item_t(insert[u]);
			}
		else
			{
			uint16_t u=(uint16_t)(m_item_count+count-1);
			for(; u>=m_item_count; u--)
				new (&items[u]) _item_t(std::move(items[u-count]));
			for(; u>=position+count; u--)
				items[u]=std::move(items[u-count]);
			for(uint16_t u=0; u<count; u++)
				items[position+u]=insert[u];
			}
		m_item_count+=count;
		return true;
		}
	bool remove_at(_size_t position)noexcept override
		{
		if(position>=m_item_count)
			return false;
		_item_t* items=get_items();
		for(uint16_t u=position; u+1<m_item_count; u++)
			items[u]=std::move(items[u+1]);
		m_item_count--;
		items[m_item_count].~_item_t();
		return true;
		}
	void remove_items(uint16_t position, uint16_t count)noexcept
		{
		_item_t* items=get_items();
		for(; position+count<m_item_count; position++)
			items[position]=std::move(items[position+count]);
		for(; position<m_item_count; position++)
			items[position].~_item_t();
		m_item_count-=count;
		}

protected:
	// Common
	uint16_t m_item_count;

private:
	// Unititlized array of items
	alignas(alignof(_item_t[_group_size])) unsigned char m_items[sizeof(_item_t[_group_size])];
};


//======================
// Cluster-Parent-Group
//======================

template <typename _item_t, typename _group_t, typename _item_group_t, typename _parent_group_t, typename _size_t, uint16_t _group_size>
class cluster_parent_group: public _group_t
{
public:
	// Using
	using _item_array_t=_item_t[_group_size];

	// Con-Destructors
	cluster_parent_group(uint16_t level)noexcept:
		m_child_count(0), m_children(), m_item_count(0), m_level(level)
		{}
	cluster_parent_group(_group_t* child)noexcept:
		m_child_count(1), m_children(), m_item_count(child->get_item_count()), m_level((uint16_t)(child->get_level()+1))
		{
		m_children[0]=child;
		}
	cluster_parent_group(_parent_group_t const& group)noexcept:
		m_child_count(group.m_child_count), m_children(), m_item_count(group.m_item_count), m_level(group.m_level)
		{
		if(m_level>1)
			{
			for(uint16_t u=0; u<m_child_count; u++)
				m_children[u]=new _parent_group_t((_parent_group_t const&)*group.m_children[u]);
			}
		else
			{
			for(uint16_t u=0; u<m_child_count; u++)
				m_children[u]=new _item_group_t((_item_group_t const&)*group.m_children[u]);
			}
		}
	~cluster_parent_group()noexcept override
		{
		for(uint16_t u=0; u<m_child_count; u++)
			delete m_children[u];
		}

	// Access
	_item_t* get_at(_size_t position)noexcept override
		{
		uint16_t group=get_group(&position);
		return m_children[group]->get_at(position);
		}
	_item_t const* get_at(_size_t position)const noexcept override
		{
		uint16_t group=get_group(&position);
		return m_children[group]->get_at(position);
		}
	inline _group_t* get_child(uint16_t position)const noexcept
		{
		return m_children[position];
		}
	uint16_t get_group(_size_t* position)const noexcept
		{
		for(uint16_t u=0; u<m_child_count; u++)
			{
			_size_t count=m_children[u]->get_item_count();
			if(*position<count)
				return u;
			*position-=count;
			}
		return _group_size;
		}
	inline uint16_t get_child_count()const noexcept override { return m_child_count; }
	inline _group_t* const* get_children() { return m_children; }
	inline _size_t get_item_count()const noexcept override { return m_item_count; }
	inline uint16_t get_level()const noexcept override { return m_level; }

	// Modification
	virtual _size_t insert_groups(uint16_t position, _group_t* const* groups, uint16_t count)noexcept
		{
		for(uint16_t u=(uint16_t)(m_child_count+count-1); u>=position+count; u--)
			m_children[u]=m_children[u-count];
		_size_t item_count=0;
		for(uint16_t u=0; u<count; u++)
			{
			m_children[position+u]=groups[u];
			item_count+=groups[u]->get_item_count();
			}
		m_child_count=(uint16_t)(m_child_count+count);
		m_item_count+=item_count;
		return item_count;
		}
	void move_children(uint16_t source, uint16_t destination, uint16_t count)noexcept
		{
		if(m_level>1)
			{
			auto src=(_parent_group_t*)m_children[source];
			auto dst=(_parent_group_t*)m_children[destination];
			auto src_groups=src->get_children();
			if(source>destination)
				{
				uint16_t dst_count=dst->get_child_count();
				auto item_count=dst->insert_groups(dst_count, src_groups, count);
				src->remove_groups(0, count, item_count);
				}
			else
				{
				uint16_t src_count=src->get_child_count();
				uint16_t src_pos=(uint16_t)(src_count-count);
				auto item_count=dst->insert_groups(0, &src_groups[src_pos], count);
				src->remove_groups(src_pos, count, item_count);
				}
			}
		else
			{
			auto src=(_item_group_t*)m_children[source];
			auto dst=(_item_group_t*)m_children[destination];
			auto src_items=src->get_items();
			if(source>destination)
				{
				uint16_t dst_count=dst->get_child_count();
				dst->insert_items(dst_count, &src_items[0], count);
				src->remove_items(0, count);
				}
			else
				{
				uint16_t src_count=src->get_child_count();
				uint16_t src_pos=(uint16_t)(src_count-count);
				dst->insert_items(0, &src_items[src_pos], count);
				src->remove_items(src_pos, count);
				}
			}
		}
	inline void move_emtpy_slot(uint16_t source, uint16_t destination)noexcept
		{
		if(source<destination)
			{
			for(uint16_t u=source; u<destination; u++)
				move_children((uint16_t)(u+1), u, 1);
			}
		else
			{
			for(uint16_t u=source; u>destination; u--)
				move_children((uint16_t)(u-1), u, 1);
			}
		}
	virtual bool remove_at(_size_t position)noexcept override
		{
		if(position>=m_item_count)
			return false;
		uint16_t group=get_group(&position);
		m_children[group]->remove_at(position);
		m_item_count--;
		combine_children(group);
		return true;
		}
	virtual void remove_groups(uint16_t position, uint16_t count, _size_t item_count)noexcept
		{
		for(uint16_t u=position; u+count<m_child_count; u++)
			m_children[u]=m_children[u+count];
		m_child_count=(uint16_t)(m_child_count-count);
		m_item_count-=item_count;
		}
	inline void set_child_count(uint16_t count) { m_child_count=count; }

protected:
	// Access
	uint16_t get_nearest_space(uint16_t position)const noexcept
		{
		int16_t before=(int16_t)(position-1);
		uint16_t after=(uint16_t)(position+1);
		while(before>=0||after<m_child_count)
			{
			if(before>=0)
				{
				if(m_children[before]->get_child_count()<_group_size)
					return before;
				before--;
				}
			if(after<m_child_count)
				{
				if(m_children[after]->get_child_count()<_group_size)
					return after;
				after++;
				}
			}
		return _group_size;
		}

	// Modification
	bool combine_children(uint16_t position)noexcept
		{
		uint16_t count=m_children[position]->get_child_count();
		if(count==0)
			{
			remove_group(position);
			return true;
			}
		if(position>0)
			{
			uint16_t before=m_children[position-1]->get_child_count();
			if(count+before<=_group_size)
				{
				move_children(position, (uint16_t)(position-1), count);
				remove_group(position);
				return true;
				}
			}
		if(position+1<m_child_count)
			{
			uint16_t after=m_children[position+1]->get_child_count();
			if(count+after<=_group_size)
				{
				move_children((uint16_t)(position+1), position, after);
				remove_group((uint16_t)(position+1));
				return true;
				}
			}
		return false;
		}
	inline void remove_group(uint16_t position)noexcept
		{
		delete m_children[position];
		for(uint16_t u=position; u+1<m_child_count; u++)
			m_children[u]=m_children[u+1];
		m_child_count--;
		}
	bool shift_children(uint16_t group, uint16_t count)
		{
		uint16_t empty=get_nearest_space(group);
		if(empty>=m_child_count)
			return false;
		if(count>1&&empty>group)
			group++;
		move_emtpy_slot(empty, group);
		return true;
		}
	bool split_child(uint16_t position)noexcept
		{
		if(m_child_count==_group_size)
			return false;
		for(uint16_t u=m_child_count; u>position+1; u--)
			m_children[u]=m_children[u-1];
		m_child_count++;
		if(m_level>1)
			{
			m_children[position+1]=new _parent_group_t((uint16_t)(m_level-1));
			}
		else
			{
			m_children[position+1]=new _item_group_t();
			}
		move_children(position, (uint16_t)(position+1), 1);
		return true;
		}
	
	// Common
	uint16_t m_child_count;
	_group_t* m_children[_group_size];
	_size_t m_item_count;
	uint16_t m_level;
};


//=========
// Cluster
//=========

template <typename _item_t, typename _group_t, typename _item_group_t, typename _parent_group_t, typename _size_t, uint16_t _group_size>
class cluster
{
public:
	// Con-/Destructors
	cluster(): m_root(nullptr) {}
	~cluster()
		{
		if(m_root)
			delete m_root;
		}

	// Access
	_item_t& get_at(_size_t position)
		{
		if(!m_root)
			throw std::out_of_range(nullptr);
		_item_t* item=m_root->get_at(position);
		if(!item)
			throw std::out_of_range(nullptr);
		return *item;
		}
	_item_t get_at(_size_t position)const noexcept
		{
		if(!m_root)
			return _item_t();
		_item_t const* item=m_root->get_at(position);
		if(!item)
			return _item_t();
		return *item;
		}
	_size_t get_count()const noexcept
		{
		if(!m_root)
			return 0;
		return m_root->get_item_count();
		}
	inline _group_t* get_root()const { return m_root; }

	// Modification
	void clear()noexcept
		{
		if(m_root)
			{
			delete m_root;
			m_root=nullptr;
			}
		}
	bool remove_at(_size_t position)noexcept
		{
		if(!m_root)
			return false;
		if(!m_root->remove_at(position))
			return false;
		drop_root();
		return true;
		}

protected:
	// Common
	_group_t* create_root()noexcept
		{
		if(m_root)
			return m_root;
		m_root=new _item_group_t();
		return m_root;
		}
	void drop_root()noexcept
		{
		if(m_root->get_level()==0)
			{
			if(m_root->get_child_count()==0)
				clear();
			return;
			}
		if(m_root->get_child_count()>1)
			return;
		auto root=(_parent_group_t*)m_root;
		m_root=root->get_child(0);
		root->set_child_count(0);
		delete root;
		}
	_group_t* lift_root()noexcept
		{
		m_root=new _parent_group_t(m_root);
		return m_root;
		}
	_group_t* m_root;
};


//==================
// Cluster-Iterator
//==================

template <typename _item_t, typename _cluster_t>
struct cluster_iterator_traits
{
using cluster_ptr=_cluster_t*;
using item_ptr=_item_t*;
using item_ref=_item_t&;
};

template <typename _item_t, typename _cluster_t>
struct cluster_const_iterator_traits
{
using cluster_ptr=_cluster_t const*;
using item_ptr=_item_t const*;
using item_ref=_item_t const&;
};

template <typename _traits_t, typename _group_t, typename _item_group_t, typename _parent_group_t, typename _size_t, uint16_t _group_size>
class cluster_iterator_base
{
public:
	// Using
	using _cluster_ptr=typename _traits_t::cluster_ptr;
	using _item_ptr=typename _traits_t::item_ptr;
	using _item_ref=typename _traits_t::item_ref;

	// Pointer
	typedef struct
		{
		_group_t* group;
		uint16_t position;
		}it_ptr;

	// Con-/Destructors
	cluster_iterator_base(cluster_iterator_base const& it):
		m_cluster(it.m_cluster), m_current(nullptr), m_level_count(0), m_position(-2), m_pointers(nullptr)
		{
		set_position(it.m_position);
		}
	cluster_iterator_base(_cluster_ptr cluster, _size_t position)noexcept:
		m_cluster(cluster), m_current(nullptr), m_level_count(0), m_position(-2), m_pointers(nullptr)
		{
		set_position(position);
		}
	~cluster_iterator_base()
		{
		if(m_pointers)
			operator delete(m_pointers);
		}

	// Access
	inline _item_ref operator*()const { return *m_current; }
	inline _item_ptr operator->()const { return m_current; }
	inline _item_ref get_current()const noexcept { return *m_current; }
	inline bool has_current()const noexcept { return m_pointers!=nullptr; }

	// Comparison
	inline bool operator==(cluster_iterator_base const& it)
		{
		return (m_cluster==it.m_cluster)&&(m_position==it.m_position);
		}
	inline bool operator!=(cluster_iterator_base const& it) { return !operator==(it); }

	// Navigation
	inline cluster_iterator_base& operator++()
		{
		move_next();
		return *this;
		}
	inline cluster_iterator_base& operator--()
		{
		move_previous();
		return *this;
		}
	inline _size_t get_position()const noexcept { return m_position; }
	bool move_next()noexcept
		{
		if(m_pointers==nullptr)
			return false;
		it_ptr* ptr=&m_pointers[m_level_count-1];
		_item_group_t* item_group=(_item_group_t*)ptr->group;
		uint16_t count=item_group->get_child_count();
		if(ptr->position+1<count)
			{
			ptr->position++;
			m_current=item_group->get_at(ptr->position);
			m_position++;
			return true;
			}
		for(uint16_t u=(uint16_t)(m_level_count-1); u>0; u--)
			{
			ptr=&m_pointers[u-1];
			_parent_group_t* parent_group=(_parent_group_t*)ptr->group;
			count=parent_group->get_child_count();
			if(ptr->position+1>=count)
				continue;
			ptr->position++;
			_group_t* group=ptr->group;
			for(; u<m_level_count; u++)
				{
				parent_group=(_parent_group_t*)group;
				group=parent_group->get_child(ptr->position);
				ptr=&m_pointers[u];
				ptr->group=group;
				ptr->position=0;
				}
			item_group=(_item_group_t*)group;
			m_current=item_group->get_at(0);
			m_position++;
			return true;
			}
		m_current=nullptr;
		m_position=-2;
		return false;
		}
	bool move_previous()noexcept
		{
		if(m_pointers==nullptr)
			return false;
		it_ptr* ptr=&m_pointers[m_level_count-1];
		_item_group_t* item_group=(_item_group_t*)ptr->group;
		if(ptr->position>0)
			{
			ptr->position--;
			m_current=item_group->get_at(ptr->position);
			m_position--;
			return true;
			}
		for(uint16_t u=(uint16_t)(m_level_count-1); u>0; u--)
			{
			ptr=&m_pointers[u-1];
			_parent_group_t* parent_group=(_parent_group_t*)ptr->group;
			if(ptr->position==0)
				continue;
			ptr->position--;
			_group_t* group=ptr->group;
			for(; u<m_level_count; u++)
				{
				parent_group=(_parent_group_t*)group;
				group=parent_group->get_child(ptr->position);
				ptr=&m_pointers[u];
				ptr->group=group;
				ptr->position=(uint16_t)(group->get_child_count()-1);
				}
			item_group=(_item_group_t*)group;
			m_current=item_group->get_at(ptr->position);
			m_position--;
			return true;
			}
		m_current=nullptr;
		m_position=-1;
		return false;
		}
	bool set_position(_size_t position)
		{
		m_current=nullptr;
		if(position==-1||position==-2)
			{
			m_position=position;
			return false;
			}
		m_position=-3;
		_group_t* group=m_cluster->get_root();
		if(!group)
			return false;
		_size_t offset=position;
		uint16_t group_pos=get_position_internal(group, &offset);
		if(group_pos==_group_size)
			return false;
		uint16_t level_count=(uint16_t)(group->get_level()+1);
		set_level_count(level_count);
		m_pointers[0].group=group;
		m_pointers[0].position=group_pos;
		for(uint16_t u=0; u<m_level_count-1; u++)
			{
			_parent_group_t* parent_group=(_parent_group_t*)m_pointers[u].group;
			group=parent_group->get_child(group_pos);
			group_pos=get_position_internal(group, &offset);
			if(group_pos==_group_size)
				return false;
			m_pointers[u+1].group=group;
			m_pointers[u+1].position=group_pos;
			}
		uint16_t child_count=group->get_child_count();
		if(group_pos>=child_count)
			return false;
		_item_group_t* item_group=(_item_group_t*)group;
		m_current=item_group->get_at(group_pos);
		m_position=position;
		return true;
		}

	// Modification
	bool remove_current()
		{
		if(!m_cluster->remove_at(m_position))
			return false;
		set_position(m_position);
		return true;
		}

private:
	// Common
	uint16_t get_position_internal(_group_t* group, _size_t* pos)const noexcept
		{
		uint16_t level=group->get_level();
		if(level==0)
			{
			uint16_t group_pos=(uint16_t)*pos;
			*pos=0;
			return group_pos;
			}
		_parent_group_t* parent_group=(_parent_group_t*)group;
		uint16_t child_count=parent_group->get_child_count();
		for(uint16_t u=0; u<child_count; u++)
			{
			_group_t* child=parent_group->get_child(u);
			_size_t item_count=child->get_item_count();
			if(*pos<item_count)
				return u;
			*pos-=item_count;
			}
		return _group_size;
		}
	void set_level_count(uint16_t level_count)noexcept
		{
		if(m_level_count==level_count)
			return;
		if(m_pointers)
			operator delete(m_pointers);
		m_pointers=(it_ptr*)operator new(level_count*sizeof(it_ptr));
		m_level_count=level_count;
		}

	_cluster_ptr m_cluster;
	_item_ptr m_current;
	uint16_t m_level_count;
	_size_t m_position;
	it_ptr* m_pointers;
};


//==================
// Iterable Cluster
//==================

template <typename _item_t, typename _group_t, typename _item_group_t, typename _parent_group_t, typename _size_t, uint16_t _group_size>
class iterable_cluster: public cluster<_item_t, _group_t, _item_group_t, _parent_group_t, _size_t, _group_size>
{
public:
	// Using
	using _cluster_t=cluster<_item_t, _group_t, _item_group_t, _parent_group_t, _size_t, _group_size>;
	using _it_traits_t=cluster_iterator_traits<_item_t, _cluster_t>;
	using _const_it_traits_t=cluster_const_iterator_traits<_item_t, _cluster_t>;
	using iterator=cluster_iterator_base<_it_traits_t, _group_t, _item_group_t, _parent_group_t, _size_t, _group_size>;
	using const_iterator=cluster_iterator_base<_const_it_traits_t, _group_t, _item_group_t, _parent_group_t, _size_t, _group_size>;

	// Access
	inline iterator begin() { return iterator(this, 0); }
	inline iterator begin(_size_t position) { return iterator(this, position); }
	inline const_iterator begin()const { return const_iterator(this, 0); }
	inline const_iterator begin(_size_t position)const { return const_iterator(this, position); }
	inline const_iterator cbegin()const { return const_iterator(this, 0); }
	inline const_iterator cbegin(_size_t position)const { return const_iterator(this, position); }
	inline const_iterator cend()const { return const_iterator(this, -2); }
	inline const_iterator crend()const { return const_iterator(this, -1); }
	inline iterator end() { return iterator(this, -2); }
	inline iterator rend() { return iterator(this, -1); }
};


} // namespace

#endif // _CLUSTERS_CLUSTER_H
