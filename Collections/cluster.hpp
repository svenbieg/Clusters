//=============
// cluster.hpp
//=============

// Implementation of a pyramidal directory
// Shared classes for list and index

// Copyright 2025, Sven Bieg (svenbieg@outlook.de)
// http://github.com/svenbieg/Clusters


#pragma once


//=======
// Using
//=======

#include <assert.h>
#include <new>
#include <stdexcept>
#include <stdint.h>
#include <type_traits>
#include <utility>


//===========
// Namespace
//===========

namespace Collections {


//======================
// Forward-Declarations
//======================

template <class _traits_t, bool _is_const> class cluster_iterator_base;


//=======
// Group
//=======

template <class _traits_t>
class cluster_group
{
public:
	// Using
	using _item_t=typename _traits_t::item_t;
	using _size_t=typename _traits_t::size_t;

	// Con-Destructors
	virtual ~cluster_group() {}

	// Access
	virtual _item_t& get_at(_size_t position)=0;
	virtual _item_t const& get_at(_size_t position)const=0;
	virtual uint16_t get_child_count()const=0;
	virtual _size_t get_item_count()const=0;
	virtual uint16_t get_level()const=0;

	// Modification
	virtual void remove_at(_size_t position, _item_t* item_ptr)=0;
};


//============
// Item-Group
//============

template <class _traits_t>
class cluster_item_group: public _traits_t::group_t
{
public:
	// Using
	using _item_t=typename _traits_t::item_t;
	using _group_t=typename _traits_t::group_t;
	using _item_group_t=typename _traits_t::item_group_t;
	using _size_t=typename _traits_t::size_t;
	static constexpr uint16_t _group_size=_traits_t::group_size;

	// Con-/Destructors
	cluster_item_group(): m_item_count(0), m_items() {}
	cluster_item_group(cluster_item_group const& group): m_item_count(group.m_item_count)
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
	_item_t& get_at(_size_t position)override
		{
		if(position>=m_item_count)
			throw std::out_of_range(nullptr);
		return get_items()[position];
		}
	_item_t const& get_at(_size_t position)const override
		{
		if(position>=m_item_count)
			throw std::out_of_range(nullptr);
		return get_items()[position];
		}
	inline uint16_t get_child_count()const override { return m_item_count; }
	inline _item_t const& get_first_item()const
		{
		assert(m_item_count>0);
		return get_items()[0];
		}
	inline _size_t get_item_count()const override { return m_item_count; }
	inline _item_t* get_items() { return (_item_t*)m_items; }
	inline _item_t const* get_items()const { return (_item_t const*)m_items; }
	inline _item_t const& get_last_item()const
		{
		assert(m_item_count>0);
		return get_items()[m_item_count-1];
		}
	inline uint16_t get_level()const override { return 0; }

	// Modification
	_item_t* insert_item(uint16_t position, _item_t const& insert)
		{
		assert(position<=m_item_count);
		if(m_item_count+1>_group_size)
			return nullptr;
		_item_t* items=get_items();
		for(uint16_t u=m_item_count; u>=position+1; u--)
			new (&items[u]) _item_t(std::move(items[u-1]));
		new (&items[position]) _item_t(insert);
		m_item_count++;
		return &items[position];
		}
	_item_t* insert_item(uint16_t position, _item_t&& insert)
		{
		assert(position<=m_item_count);
		if(m_item_count+1>_group_size)
			return nullptr;
		_item_t* items=get_items();
		for(uint16_t u=m_item_count; u>=position+1; u--)
			new (&items[u]) _item_t(std::move(items[u-1]));
		new (&items[position]) _item_t(std::move(insert));
		m_item_count++;
		return &items[position];
		}
	uint16_t insert_items(uint16_t position, _item_t* insert, uint16_t count)
		{
		assert(position<=m_item_count);
		assert(m_item_count+count<=_group_size);
		_item_t* items=get_items();
		uint16_t u=(uint16_t)(m_item_count+count-1);
		for(; u>=position+count; u--)
			new (&items[u]) _item_t(std::move(items[u-count]));
		for(u=0; u<count; u++)
			new (&items[position+u]) _item_t(std::move(insert[u]));
		m_item_count+=count;
		return count;
		}
	uint16_t insert_items(uint16_t position, _item_t const* insert, uint16_t count)
		{
		assert(position<=m_item_count);
		if(m_item_count==_group_size)
			return 0;
		uint16_t copy=count;
		if(copy>_group_size-m_item_count)
			copy=_group_size-m_item_count;
		_item_t* items=get_items();
		uint16_t u=(uint16_t)(m_item_count+copy-1);
		for(; u>=position+copy; u--)
			new (&items[u]) _item_t(std::move(items[u-copy]));
		for(u=0; u<copy; u++)
			new (&items[position+u]) _item_t(insert[u]);
		m_item_count+=copy;
		return copy;
		}
	void remove_at(_size_t position, _item_t* item_ptr)override
		{
		if(position>=m_item_count)
			throw std::out_of_range(nullptr);
		_item_t* items=get_items();
		if(item_ptr)
			{
			item_ptr->~_item_t();
			new (item_ptr) _item_t(std::move(items[position]));
			}
		else
			{
			items[position].~_item_t();
			}
		for(uint16_t u=(uint16_t)position; u+1<m_item_count; u++)
			new (&items[u]) _item_t(std::move(items[u+1]));
		m_item_count--;
		}
	void remove_items(uint16_t position, uint16_t count)
		{
		_item_t* items=get_items();
		for(uint16_t u=0; u<count; u++)
			items[position+u].~_item_t();
		for(; position+count<m_item_count; position++)
			{
			items[position].~_item_t();
			new (&items[position]) _item_t(std::move(items[position+count]));
			}
		m_item_count-=count;
		}

protected:
	// Common
	uint16_t m_item_count;

private:
	// Uninitialized array of items
	alignas(alignof(_item_t[_group_size])) char m_items[sizeof(_item_t[_group_size])];
};


//==============
// Parent-Group
//==============

template <class _traits_t>
class cluster_parent_group: public _traits_t::group_t
{
public:
	// Using
	using _item_t=typename _traits_t::item_t;
	using _group_t=typename _traits_t::group_t;
	using _item_group_t=typename _traits_t::item_group_t;
	using _parent_group_t=typename _traits_t::parent_group_t;
	using _size_t=typename _traits_t::size_t;
	static constexpr uint16_t _group_size=_traits_t::group_size;

	// Con-/Destructors
	cluster_parent_group(uint16_t level=1):
		m_child_count(0), m_children(), m_item_count(0), m_level(level)
		{}
	cluster_parent_group(cluster_parent_group const& group):
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
	~cluster_parent_group()override
		{
		for(uint16_t u=0; u<m_child_count; u++)
			delete m_children[u];
		}

	// Access
	_item_t& get_at(_size_t position)override
		{
		if(position>=m_item_count)
			throw std::out_of_range(nullptr);
		uint16_t group=get_group(&position);
		assert(group<m_child_count);
		return m_children[group]->get_at(position);
		}
	_item_t const& get_at(_size_t position)const override
		{
		if(position>=m_item_count)
			throw std::out_of_range(nullptr);
		uint16_t group=get_group(&position);
		assert(group<m_child_count);
		return m_children[group]->get_at(position);
		}
	inline _group_t* get_child(uint16_t position)const
		{
		assert(position<m_child_count);
		return m_children[position];
		}
	inline uint16_t get_child_count()const override { return m_child_count; }
	inline _group_t* const* get_children()const { return m_children; }
	uint16_t get_group(_size_t* position)const
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
	inline _size_t get_item_count()const override { return m_item_count; }
	inline uint16_t get_level()const override { return m_level; }

	// Modification
	virtual _size_t insert_groups(uint16_t position, _group_t* const* groups, uint16_t count)
		{
		assert(position<=m_child_count);
		assert(m_child_count+count<=_group_size);
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
	void move_children(uint16_t source, uint16_t destination, uint16_t count)
		{
		assert(source<=m_child_count);
		assert(destination<=m_child_count);
		assert(source!=destination);
		assert(count>0);
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
	void move_emtpy_slot(uint16_t source, uint16_t destination)
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
	virtual void remove_at(_size_t position, _item_t* item_ptr)override
		{
		if(position>=m_item_count)
			throw std::out_of_range(nullptr);
		uint16_t group=get_group(&position);
		assert(group<m_child_count);
		m_children[group]->remove_at(position, item_ptr);
		m_item_count--;
		combine_children(group);
		}
	virtual void remove_groups(uint16_t position, uint16_t count, _size_t item_count)
		{
		assert(position+count<=m_child_count);
		assert(count>0);
		for(uint16_t u=position; u+count<m_child_count; u++)
			m_children[u]=m_children[u+count];
		m_child_count-=count;
		m_item_count-=item_count;
		}
	virtual void set_child(_group_t* child)
		{
		assert(m_child_count==0);
		m_children[0]=child;
		m_child_count=1;
		m_item_count=child->get_item_count();
		m_level=child->get_level()+1;
		}
	inline void set_child_count(uint16_t count) { m_child_count=count; }

protected:
	// Access
	uint16_t get_nearest_space(uint16_t position)const
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
	bool combine_children(uint16_t position)
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
	void remove_group(uint16_t position)
		{
		delete m_children[position];
		for(uint16_t u=position; u+1<m_child_count; u++)
			m_children[u]=m_children[u+1];
		m_child_count--;
		}
	bool shift_children(uint16_t group, uint16_t ins_count)
		{
		uint16_t empty=get_nearest_space(group);
		if(empty>=m_child_count)
			return false;
		if(ins_count>1&&empty>group)
			group++;
		move_emtpy_slot(empty, group);
		return true;
		}
	bool split_child(uint16_t position)
		{
		if(m_child_count==_group_size)
			return false;
		for(uint16_t u=m_child_count; u>position+1; u--)
			m_children[u]=m_children[u-1];
		if(m_level>1)
			{
			m_children[position+1]=new _parent_group_t((uint16_t)(m_level-1));
			}
		else
			{
			m_children[position+1]=new _item_group_t();
			}
		m_child_count++;
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

template <class _traits_t>
class cluster
{
public:
	// Using
	using _item_t=typename _traits_t::item_t;
	using _group_t=typename _traits_t::group_t;
	using _item_group_t=typename _traits_t::item_group_t;
	using _parent_group_t=typename _traits_t::parent_group_t;
	using _cluster_t=typename _traits_t::cluster_t;
	using _size_t=typename _traits_t::size_t;
	using iterator=typename _traits_t::iterator_t;
	using const_iterator=typename _traits_t::const_iterator_t;

	// Friends
	friend iterator;
	friend const_iterator;
	friend cluster_iterator_base<_traits_t, false>;
	friend cluster_iterator_base<_traits_t, true>;

	// Access
	inline operator bool()const { return m_root!=nullptr; }
	inline iterator begin() { return iterator(this, 0); }
	inline iterator begin(_size_t position) { return iterator(this, position); }
	inline const_iterator begin()const { return const_iterator(this, 0); }
	inline const_iterator begin(_size_t position)const { return const_iterator(this, position); }
	inline const_iterator cbegin()const { return const_iterator(this, 0); }
	inline const_iterator cbegin(_size_t position)const { return const_iterator(this, position); }
	inline const_iterator cend()const { return const_iterator(this, -2); }
	inline const_iterator crend()const { return const_iterator(this, -1); }
	inline iterator end() { return iterator(this, -2); }
	inline const_iterator end()const { return const_iterator(this, -2); }
	_item_t& get_at(_size_t position)
		{
		if(!m_root)
			throw std::out_of_range(nullptr);
		return m_root->get_at(position);
		}
	_item_t const& get_at(_size_t position)const
		{
		if(!m_root)
			throw std::out_of_range(nullptr);
		return m_root->get_at(position);
		}
	_size_t get_count()const
		{
		if(!m_root)
			return 0;
		return m_root->get_item_count();
		}
	inline _group_t* get_root()const { return m_root; }
	inline iterator rend() { return iterator(this, -1); }

	// Modification
	bool clear()
		{
		if(m_root)
			{
			delete m_root;
			m_root=nullptr;
			return true;
			}
		return false;
		}
	void copy_from(_cluster_t const& cluster)
		{
		clear();
		auto root=cluster.get_root();
		if(!root)
			return;
		auto level=root->get_level();
		if(level>0)
			{
			auto parent_group=(_parent_group_t const*)root;
			m_root=new _parent_group_t(*parent_group);
			}
		else
			{
			auto item_group=(_item_group_t const*)root;
			m_root=new _item_group_t(*item_group);
			}
		}
	void remove_at(_size_t position, _item_t* item_ptr=nullptr)
		{
		if(!m_root)
			throw std::out_of_range(nullptr);
		m_root->remove_at(position, item_ptr);
		drop_root();
		}

protected:
	// Con-/Destructors
	cluster(_group_t* root): m_root(root) {}
	~cluster()
		{
		if(m_root)
			{
			delete m_root;
			m_root=nullptr;
			}
		}

	// Common
	_group_t* create_root()
		{
		if(m_root)
			return m_root;
		m_root=new _item_group_t();
		return m_root;
		}
	void drop_root()
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
	_group_t* lift_root()
		{
		auto root=new _parent_group_t();
		root->set_child(m_root);
		m_root=root;
		return m_root;
		}
	_group_t* m_root;
};


//==========
// Iterator
//==========

template <class _traits_t, bool _is_const>
class cluster_iterator_base
{
public:
	// Using
	using _cluster_t=cluster<_traits_t>;
	using _cluster_ptr=typename std::conditional<_is_const, _cluster_t const*, _cluster_t*>::type;
	using _item_t=typename _traits_t::item_t;
	using _item_ptr=typename std::conditional<_is_const, _item_t const*, _item_t*>::type;
	using _item_ref=typename std::conditional<_is_const, _item_t const&, _item_t&>::type;
	using _group_t=typename _traits_t::group_t;
	using _item_group_t=typename _traits_t::item_group_t;
	using _parent_group_t=typename _traits_t::parent_group_t;
	using _size_t=typename _traits_t::size_t;
	static constexpr uint16_t _group_size=_traits_t::group_size;

	// Con-/Destructors
	cluster_iterator_base(cluster_iterator_base const& it):
		m_cluster(it.m_cluster), m_current(nullptr), m_level_count(0), m_position(-2), m_its(nullptr)
		{
		set_position(it.m_position);
		}
	cluster_iterator_base(_cluster_ptr cluster):
		m_cluster(cluster), m_current(nullptr), m_level_count(0), m_position(-2), m_its(nullptr)
		{}
	cluster_iterator_base(_cluster_ptr cluster, _size_t position):
		m_cluster(cluster), m_current(nullptr), m_level_count(0), m_position(-2), m_its(nullptr)
		{
		set_position(position);
		}
	~cluster_iterator_base()
		{
		if(m_its)
			{
			operator delete(m_its);
			m_its=nullptr;
			}
		}

	// Access
	inline _item_ref operator*()const { return get_current(); }
	inline _item_ptr operator->()const { return m_current; }
	_item_ref get_current()const
		{
		if(!m_current)
			throw std::out_of_range(nullptr);
		return *m_current;
		}
	inline bool has_current()const { return m_current!=nullptr; }

	// Comparison
	inline bool operator==(cluster_iterator_base const& it)const
		{
		return (m_cluster==it.m_cluster)&&(m_position==it.m_position);
		}
	inline bool operator!=(cluster_iterator_base const& it)const { return !operator==(it); }

	// Navigation
	inline cluster_iterator_base& operator++()
		{
		this->move_next();
		return *this;
		}
	inline cluster_iterator_base& operator--()
		{
		this->move_previous();
		return *this;
		}
	inline bool begin() { return set_position(0); }
	inline void end() { reset(-2); }
	inline _size_t get_position()const { return m_position; }
	virtual bool move_next()
		{
		if(m_position==-2)
			return false;
		if(m_position==-1)
			return begin();
		auto it_ptr=&m_its[m_level_count-1];
		auto item_group=(_item_group_t*)it_ptr->group;
		uint16_t count=item_group->get_child_count();
		if(it_ptr->position+1<count)
			{
			it_ptr->position++;
			m_current=&item_group->get_at(it_ptr->position);
			m_position++;
			return true;
			}
		for(uint16_t u=(uint16_t)(m_level_count-1); u>0; u--)
			{
			it_ptr--;
			auto parent_group=(_parent_group_t*)it_ptr->group;
			count=parent_group->get_child_count();
			if(it_ptr->position+1>=count)
				continue;
			it_ptr->position++;
			auto group=it_ptr->group;
			for(; u<m_level_count; u++)
				{
				parent_group=(_parent_group_t*)group;
				group=parent_group->get_child(it_ptr->position);
				it_ptr++;
				it_ptr->group=group;
				it_ptr->position=0;
				}
			item_group=(_item_group_t*)group;
			m_current=&item_group->get_at(0);
			m_position++;
			return true;
			}
		reset(-2);
		return false;
		}
	virtual bool move_previous()
		{
		if(m_position==-1)
			return false;
		if(m_position==-2)
			return rbegin();
		auto it_ptr=&m_its[m_level_count-1];
		auto item_group=(_item_group_t*)it_ptr->group;
		if(it_ptr->position>0)
			{
			it_ptr->position--;
			m_current=&item_group->get_at(it_ptr->position);
			m_position--;
			return true;
			}
		for(uint16_t u=(uint16_t)(m_level_count-1); u>0; u--)
			{
			it_ptr--;
			auto parent_group=(_parent_group_t*)it_ptr->group;
			if(it_ptr->position==0)
				continue;
			it_ptr->position--;
			auto group=it_ptr->group;
			for(; u<m_level_count; u++)
				{
				parent_group=(_parent_group_t*)group;
				group=parent_group->get_child(it_ptr->position);
				it_ptr++;
				it_ptr->group=group;
				it_ptr->position=(uint16_t)(group->get_child_count()-1);
				}
			item_group=(_item_group_t*)group;
			m_current=&item_group->get_at(it_ptr->position);
			m_position--;
			return true;
			}
		reset(-1);
		return false;
		}
	bool rbegin()
		{
		_size_t item_count=m_cluster->get_count();
		if(item_count==0||!set_position(item_count-1))
			{
			rend();
			return false;
			}
		return true;
		}
	inline void rend() { reset(-1); }
	virtual bool set_position(_size_t position)
		{
		if(is_outside(position))
			{
			reset(position);
			return false;
			}
		_group_t* group=m_cluster->get_root();
		if(!group)
			{
			reset(-2);
			return false;
			}
		_size_t offset=position;
		uint16_t group_pos=get_position_internal(group, &offset);
		if(group_pos==_group_size)
			{
			reset(-2);
			return false;
			}
		uint16_t level_count=(uint16_t)(group->get_level()+1);
		set_level_count(level_count);
		auto it_ptr=&m_its[0];
		it_ptr->group=group;
		it_ptr->position=group_pos;
		for(uint16_t u=0; u<m_level_count-1; u++)
			{
			auto parent_group=(_parent_group_t*)it_ptr->group;
			group=parent_group->get_child(group_pos);
			group_pos=get_position_internal(group, &offset);
			if(group_pos==_group_size)
				{
				reset(-2);
				return false;
				}
			it_ptr++;
			it_ptr->group=group;
			it_ptr->position=group_pos;
			}
		auto item_group=(_item_group_t*)group;
		m_current=&item_group->get_at(group_pos);
		m_position=position;
		return true;
		}

protected:
	// Iterator-Pointer
	struct it_pointer
		{
		_group_t* group;
		uint16_t position;
		};

	// Common
	uint16_t get_position_internal(_group_t* group, _size_t* position)const
		{
		uint16_t child_count=group->get_child_count();
		uint16_t level=group->get_level();
		if(level==0)
			{
			uint16_t group_pos=(uint16_t)*position;
			*position=0;
			if(group_pos>=child_count)
				return _group_size;
			return group_pos;
			}
		_parent_group_t* parent_group=(_parent_group_t*)group;
		for(uint16_t u=0; u<child_count; u++)
			{
			_group_t* child=parent_group->get_child(u);
			_size_t item_count=child->get_item_count();
			if(*position<item_count)
				return u;
			*position-=item_count;
			}
		return _group_size;
		}
	inline bool is_outside()const { return is_outside(m_position); }
	bool is_outside(_size_t position)const
		{
		return (position==-2||position==-1);
		}
	void reset(_size_t position)
		{
		set_level_count(0);
		m_current=nullptr;
		m_position=position;
		}
	void set_level_count(uint16_t level_count)
		{
		if(m_level_count==level_count)
			return;
		if(m_its)
			{
			operator delete(m_its);
			m_its=nullptr;
			}
		if(level_count>0)
			m_its=(it_pointer*)operator new(level_count*sizeof(it_pointer));
		m_level_count=level_count;
		}
	_cluster_ptr m_cluster;
	_item_ptr m_current;
	it_pointer* m_its;
	uint16_t m_level_count;
	_size_t m_position;
};

template <typename _traits_t, bool _is_const>
class cluster_iterator: public cluster_iterator_base<_traits_t, false>
{
public:
	// Using
	using _base_t=cluster_iterator_base<_traits_t, false>;
	using _item_t=typename _traits_t::item_t;

	// Con-/Destructors
	using _base_t::_base_t;

	// Modification
	bool remove_current(_item_t* item_ptr=nullptr)
		{
		if(!this->has_current())
			return false;
		auto position=this->m_position;
		this->m_cluster->remove_at(position, item_ptr);
		this->set_position(position);
		return true;
		}
};

template <typename _traits_t>
class cluster_iterator<_traits_t, true>: public cluster_iterator_base<_traits_t, true>
{
public:
	// Using
	using _base_t=cluster_iterator_base<_traits_t, true>;

	// Con-/Destructors
	using _base_t::_base_t;
};

}