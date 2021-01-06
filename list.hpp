//========
// list.h
//========

// Implementation of an ordererd list.
// Items can be inserted and removed in real-time.

// Copyright 2019, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _CLUSTERS_LIST_HPP
#define _CLUSTERS_LIST_HPP


//=======
// Using
//=======

#include <new>
#include <stdint.h>
#include <type_traits>
#include <utility>


//===========
// Namespace
//===========

namespace Clusters {


//======================
// Forward-Declarations
//======================

template <typename _item_t, uint16_t _group_size> class list;


//=======
// Group
//=======

template <typename _item_t>
class _list_group
{
public:
	// Con-Destructors
	virtual ~_list_group()noexcept {}

	// Access
	virtual _item_t* get_at(std::size_t position)noexcept=0;
	virtual _item_t const* get_at(std::size_t position)const noexcept=0;
	virtual uint16_t get_child_count()const noexcept=0;
	virtual std::size_t get_item_count()const noexcept=0;
	virtual uint16_t get_level()const noexcept=0;
	virtual std::size_t get_many(std::size_t position, std::size_t count, _item_t* items)const noexcept=0;

	// Modification
	virtual bool append(_item_t const& item, bool again)noexcept=0;
	virtual std::size_t append(_item_t const* append, std::size_t count)noexcept=0;
	virtual bool insert_at(std::size_t position, _item_t const& item, bool again)noexcept=0;
	virtual bool remove_at(std::size_t position)noexcept=0;
	virtual bool set_at(std::size_t position, _item_t const& item)noexcept=0;
	virtual std::size_t set_many(std::size_t position, std::size_t count, _item_t const* items)=0;
};


//============
// Item-group
//============

template <typename _item_t, uint16_t _group_size>
class _list_item_group: public _list_group<_item_t>
{
public:
	// Con-/Destructors
	_list_item_group()noexcept: m_item_count(0) {}
	_list_item_group(_list_item_group const& group)noexcept: m_item_count(group.m_item_count)
		{
		_item_t* dst=get_items();
		_item_t const* src=group.get_items();
		for(uint16_t u=0; u<m_item_count; u++)
			new (&dst[u]) _item_t(src[u]);
		}
	~_list_item_group()noexcept override
		{
		_item_t* items=get_items();
		for(uint16_t u=0; u<m_item_count; u++)
			items[u].~_item_t();
		}

	// Access
	_item_t* get_at(std::size_t position)noexcept override
		{
		if(position>=m_item_count)
			return nullptr;
		return &get_items()[position];
		}
	_item_t const* get_at(std::size_t position)const noexcept override
		{
		if(position>=m_item_count)
			return nullptr;
		return &get_items()[position];
		}
	inline uint16_t get_child_count()const noexcept override { return m_item_count; }
	inline std::size_t get_item_count()const noexcept override { return m_item_count; }
	inline _item_t* get_items()noexcept { return (_item_t*)m_items; }
	inline _item_t const* get_items()const noexcept { return (_item_t const*)m_items; }
	inline uint16_t get_level()const noexcept override { return 0; }
	std::size_t get_many(std::size_t position, std::size_t count, _item_t* items)const noexcept override
		{
		if(position>=m_item_count)
			return 0;
		uint16_t pos=(uint16_t)position;
		uint16_t max=(uint16_t)(m_item_count-pos);
		uint16_t copy=count>max? max: (uint16_t)count;
		_item_t const* src=get_items();
		for(uint16_t u=0; u<copy; u++)
			items[u]=src[pos+u];
		return copy;
		}

	// Modification
	bool append(_item_t const& item, bool)noexcept override
		{
		if(m_item_count==_group_size)
			return false;
		_item_t* items=get_items();
		new (&items[m_item_count]) _item_t(item);
		m_item_count++;
		return true;
		}
	std::size_t append(_item_t const* append, std::size_t count)noexcept override
		{
		uint16_t copy=(uint16_t)(_group_size-m_item_count);
		if(copy==0)
			return 0;
		if(count<copy)
			copy=(uint16_t)count;
		_item_t* items=get_items();
		for(uint16_t u=0; u<copy; u++)
			new (&items[m_item_count+u]) _item_t(append[u]);
		m_item_count=(uint16_t)(m_item_count+copy);
		return copy;
		}
	void append_items(_item_t* append, uint16_t count)noexcept
		{
		_item_t* items=get_items();
		for(uint16_t u=0; u<count; u++)
			new (&items[m_item_count+u]) _item_t(std::move(append[u]));
		m_item_count=(uint16_t)(m_item_count+count);
		}
	bool insert_at(std::size_t position, _item_t const& item, bool)noexcept override
		{
		if(position>m_item_count)
			return true;
		if(m_item_count==_group_size)
			return false;
		_item_t* items=get_items();
		for(uint16_t u=m_item_count; u>position; u--)
			new (&items[u]) _item_t(std::move(items[u-1]));
		new (&items[position]) _item_t(item);
		m_item_count++;
		return true;
		}
	void insert_items(uint16_t position, _item_t* insert, uint16_t count)noexcept
		{
		_item_t* items=get_items();
		for(uint16_t u=(uint16_t)(m_item_count+count-1); u>=position+count; u--)
			new (&items[u]) _item_t(std::move(items[u-count]));
		for(uint16_t u=0; u<count; u++)
			new (&items[position+u]) _item_t(std::move(insert[u]));
		m_item_count=(uint16_t)(m_item_count+count);
		}
	bool remove_at(std::size_t position)noexcept override
		{
		if(position>=m_item_count)
			return false;
		_item_t* items=get_items();
		items[position].~_item_t();
		for(uint16_t u=(uint16_t)position; u+1<m_item_count; u++)
			new (&items[u]) _item_t(std::move(items[u+1]));
		m_item_count--;
		return true;
		}
	void remove_items(uint16_t position, uint16_t count)noexcept
		{
		_item_t* items=get_items();
		for(uint16_t u=position; u+count<m_item_count; u++)
			new (&items[u]) _item_t(std::move(items[u+count]));
		m_item_count=(uint16_t)(m_item_count-count);
		}
	bool set_at(std::size_t position, _item_t const& item)noexcept override
		{
		if(position>=m_item_count)
			return false;
		_item_t* items=get_items();
		items[position]=item;
		return true;
		}
	std::size_t set_many(std::size_t position, std::size_t count, _item_t const* items)override
		{
		if(position>=m_item_count)
			return 0;
		uint16_t max=(uint16_t)(m_item_count-(uint16_t)position);
		uint16_t copy=count>max? max: (uint16_t)count;
		_item_t* dst=get_items();
		for(uint16_t u=0; u<copy; u++)
			dst[position+u]=items[u];
		return copy;
		}

private:
	// Uninitialized array of items
	uint16_t m_item_count;
	alignas(alignof(_item_t[_group_size])) unsigned char m_items[sizeof(_item_t[_group_size])];
};


//==============
// Parent-group
//==============

template <typename _item_t, uint16_t _group_size>
class _list_parent_group: public _list_group<_item_t>
{
private:
	// Using
	using _group_t=_list_group<_item_t>;
	using _item_group_t=_list_item_group<_item_t, _group_size>;
	using _parent_group_t=_list_parent_group<_item_t, _group_size>;

public:
	// Con-Destructors
	_list_parent_group(uint16_t level)noexcept: m_child_count(0), m_item_count(0), m_level(level) {}
	_list_parent_group(_group_t* child)noexcept:
		m_child_count(1), m_item_count(child->get_item_count()), m_level((uint16_t)(child->get_level()+1))
		{
		m_children[0]=child;
		}
	_list_parent_group(_parent_group_t const& group)noexcept:
		m_child_count(group.m_child_count), m_item_count(group.m_item_count), m_level(group.m_level)
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
	~_list_parent_group()noexcept override
		{
		for(uint16_t u=0; u<m_child_count; u++)
				delete m_children[u];
		}

	// Access
	_item_t* get_at(std::size_t position)noexcept override
		{
		uint16_t group=get_group(&position);
		if(group>=_group_size)
			return nullptr;
		return m_children[group]->get_at(position);
		}
	_item_t const* get_at(std::size_t position)const noexcept override
		{
		uint16_t group=get_group(&position);
		if(group>=_group_size)
			return nullptr;
		return m_children[group]->get_at(position);
		}
	_group_t* get_child(uint16_t position)noexcept
		{
		if(position>=m_child_count)
			return nullptr;
		return m_children[position];
		}
	inline uint16_t get_child_count()const noexcept override { return m_child_count; }
	inline std::size_t get_item_count()const noexcept override { return m_item_count; }
	inline uint16_t get_level()const noexcept override { return m_level; }
	std::size_t get_many(std::size_t position, std::size_t count, _item_t* items)const noexcept override
		{
		uint16_t group=get_group(&position);
		if(group>=_group_size)
			return 0;
		std::size_t pos=0;
		while(pos<count)
			{
			pos+=m_children[group]->get_many(position, count-pos, &items[pos]);
			if(pos==count)
				return pos;
			group++;
			if(group==m_child_count)
				return pos;
			position=0;
			}
		return pos;
		}

	// Modification
	bool append(_item_t const& item, bool again)noexcept override
		{
		uint16_t group=(uint16_t)(m_child_count-1);
		if(!again)
			{
			if(m_children[group]->append(item, false))
				{
				m_item_count++;
				return true;
				}
			uint16_t empty=get_nearest_space(group);
			if(empty<m_child_count)
				{
				move_empty_slot(empty, group);
				m_children[group]->append(item, false);
				m_item_count++;
				return true;
				}
			}
		if(!split_child(group))
			return false;
		if(!m_children[group+1]->append(item, true))
			return false;
		m_item_count++;
		return true;
		}
	std::size_t append(_item_t const* append, std::size_t count)noexcept override
		{
		std::size_t pos=0;
		if(m_child_count>0)
			{
			pos+=m_children[m_child_count-1]->append(append, count);
			if(pos>0)
				{
				m_item_count+=pos;
				if(pos==count)
					return count;
				}
			}
		if(m_child_count>1)
			{
			uint16_t last=minimize_internal();
			for(; last<m_child_count; last++)
				{
				auto written=m_children[last]->append(&append[pos], count-pos);
				if(!written)
					continue;
				m_item_count+=written;
				pos+=written;
				if(pos==count)
					break;
				}
			if(pos==count)
				{
				free_children();
				return count;
				}
			}
		while(pos<count)
			{
			if(m_child_count==_group_size)
				break;
			if(m_level==1)
				{
				m_children[m_child_count]=new _item_group_t();
				}
			else
				{
				m_children[m_child_count]=new _parent_group_t((uint16_t)(m_level-1));
				}
			m_child_count++;
			std::size_t written=m_children[m_child_count-1]->append(&append[pos], count-pos);
			m_item_count+=written;
			pos+=written;
			}
		return pos;
		}
	void append_groups(_group_t* const* groups, uint16_t count)noexcept
		{
		for(uint16_t u=0; u<count; u++)
			{
			m_children[m_child_count+u]=groups[u];
			m_item_count+=groups[u]->get_item_count();
			}
		m_child_count=(uint16_t)(m_child_count+count);
		}
	bool insert_at(std::size_t position, _item_t const& item, bool again)noexcept
		{
		if(position>m_item_count)
			return true;
		std::size_t pos=position;
		uint16_t group=0;
		uint16_t inscount=get_insert_pos(&pos, &group);
		if(!inscount)
			return false;
		if(!again)
			{
			std::size_t at=pos;
			for(uint16_t u=0; u<inscount; u++)
				{
				if(m_children[group+u]->insert_at(at, item, false))
					{
					m_item_count++;
					return true;
					}
				at=0;
				}
			uint16_t empty=get_nearest_space(group);
			if(empty<m_child_count)
				{
				if(inscount>1&&empty>group)
					group++;
				move_empty_slot(empty, group);
				pos=position;
				inscount=get_insert_pos(&pos, &group);
				std::size_t at=pos;
				for(uint16_t u=0; u<inscount; u++)
					{
					if(m_children[group+u]->insert_at(at, item, false))
						{
						m_item_count++;
						return true;
						}
					at=0;
					}
				}
			}
		if(!split_child(group))
			return false;
		std::size_t count=m_children[group]->get_item_count();
		if(pos>=count)
			{
			group++;
			pos-=count;
			}
		m_children[group]->insert_at(pos, item, true);
		m_item_count++;
		return true;
		}
	void insert_groups(uint16_t position, _group_t* const* groups, uint16_t count)noexcept
		{
		for(uint16_t u=(uint16_t)(m_child_count+count-1); u>=position+count; u--)
			m_children[u]=m_children[u-count];
		for(uint16_t u=0; u<count; u++)
			{
			m_children[position+u]=groups[u];
			m_item_count+=groups[u]->get_item_count();
			}
		m_child_count=(uint16_t)(m_child_count+count);
		}
	void move_children(uint16_t source, uint16_t destination, uint16_t count)noexcept
		{
		if(count==0)
			return;
		if(m_level>1)
			{
			_parent_group_t* src=(_parent_group_t*)m_children[source];
			_parent_group_t* dst=(_parent_group_t*)m_children[destination];
			if(source>destination)
				{
				dst->append_groups(src->m_children, count);
				src->remove_groups(0, count);
				}
			else
				{
				uint16_t srccount=src->get_child_count();
				dst->insert_groups(0, &src->m_children[srccount-count], count);
				src->remove_groups((uint16_t)(srccount-count), count);
				}
			}
		else
			{
			_item_group_t* src=(_item_group_t*)m_children[source];
			_item_group_t* dst=(_item_group_t*)m_children[destination];
			if(source>destination)
				{
				dst->append_items(src->get_items(), count);
				src->remove_items(0, count);
				}
			else
				{
				_item_t* srcitems=src->get_items();
				uint16_t srccount=src->get_child_count();
				dst->insert_items(0, &srcitems[srccount-count], count);
				src->remove_items((uint16_t)(srccount-count), count);
				}
			}
		}
	void move_empty_slot(uint16_t source, uint16_t destination)noexcept
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
	bool remove_at(std::size_t position)noexcept override
		{
		if(position>=m_item_count)
			return false;
		uint16_t group=get_group(&position);
		m_children[group]->remove_at(position);
		m_item_count--;
		combine_children(group);
		return true;
		}
	void remove_groups(uint16_t position, uint16_t count)noexcept
		{
		for(uint16_t u=0; u<count; u++)
			m_item_count-=m_children[position+u]->get_item_count();
		for(uint16_t u=position; u+count<m_child_count; u++)
			m_children[u]=m_children[u+count];
		m_child_count=(uint16_t)(m_child_count-count);
		}
	bool set_at(std::size_t position, _item_t const& item)noexcept override
		{
		uint16_t group=get_group(&position);
		if(group>=_group_size)
			return false;
		return m_children[group]->set_at(position, item);
		}
	inline void set_child_count(uint16_t count)noexcept { m_child_count=count; }
	std::size_t set_many(std::size_t position, std::size_t count, _item_t const* items)override
		{
		uint16_t group=get_group(&position);
		if(group>=_group_size)
			return 0;
		std::size_t pos=0;
		while(pos<count)
			{
			pos+=m_children[group]->set_many(position, count-pos, &items[pos]);
			if(pos==count)
				return pos;
			group++;
			if(group==m_child_count)
				return pos;
			position=0;
			}
		return pos;
		}

private:
	// Access
	uint16_t get_group(std::size_t* position)const noexcept
		{
		for(uint16_t u=0; u<m_child_count; u++)
			{
			std::size_t count=m_children[u]->get_item_count();
			if(*position<count)
				return u;
			*position-=count;
			}
		return _group_size;
		}
	uint16_t get_insert_pos(std::size_t* position, uint16_t* group)const noexcept
		{
		std::size_t pos=*position;
		for(uint16_t u=0; u<m_child_count; u++)
			{
			std::size_t count=m_children[u]->get_item_count();
			if(pos<=count)
				{
				*group=u;
				*position=pos;
				if(pos==count&&u+1<m_child_count)
					return 2;
				return 1;
				}
			pos-=count;
			}
		return 0;
		}
	uint16_t get_nearest_space(uint16_t position)const noexcept
		{
		int16_t before=(int16_t)((int16_t)position-1);
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
			remove_internal(position);
			return true;
			}
		if(position>0)
			{
			uint16_t before=m_children[position-1]->get_child_count();
			if(count+before<=_group_size)
				{
				move_children(position, (uint16_t)(position-1), count);
				remove_internal(position);
				return true;
				}
			}
		if(position+1<m_child_count)
			{
			uint16_t after=m_children[position+1]->get_child_count();
			if(count+after<=_group_size)
				{
				move_children((uint16_t)(position+1), position, after);
				remove_internal((uint16_t)(position+1));
				return true;
				}
			}
		return false;
		}
	void free_children()noexcept
		{
		for(uint16_t u=m_child_count; u>0; u--)
			{
			if(m_children[u-1]->get_child_count()>0)
				break;
			delete m_children[u-1];
			m_child_count--;
			}
		}
	uint16_t minimize_internal()noexcept
		{
		uint16_t dst=0;
		uint16_t src=1;
		for(; dst<m_child_count; dst++)
			{
			uint16_t free=(uint16_t)(_group_size-m_children[dst]->get_child_count());
			if(free==0)
				continue;
			if(src<=dst)
				src=(uint16_t)(dst+1);
			for(; src<m_child_count; src++)
				{
				uint16_t count=m_children[src]->get_child_count();
				if(count==0)
					continue;
				uint16_t move=count<free? count: free;
				move_children(src, dst, move);
				free=(uint16_t)(free-move);
				if(free==0)
					break;
				}
			if(src>=m_child_count)
				break;
			}
		return dst;
		}
	void remove_internal(uint16_t position)noexcept
		{
		delete m_children[position];
		for(uint16_t u=position; u+1<m_child_count; u++)
			m_children[u]=m_children[u+1];
		m_child_count--;
		}
	bool split_child(uint16_t position)noexcept
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
	std::size_t m_item_count;
	uint16_t m_level;
};


//=========
// Cluster
//=========

// Forward-Declaration
template <typename _item_t, uint16_t _group_size, bool _is_const> class _list_iterator_base;

template <typename _item_t, uint16_t _group_size>
class _list_cluster
{
private:
	// Using
	using _group_t=_list_group<_item_t>;
	using _item_group_t=_list_item_group<_item_t, _group_size>;
	using _parent_group_t=_list_parent_group<_item_t, _group_size>;

public:
	// Friends
	friend class _list_iterator_base<_item_t, _group_size, true>;
	friend class _list_iterator_base<_item_t, _group_size, false>;

	// Access
	_item_t get_at(std::size_t position)const noexcept
		{
		if(!m_root)
			return _item_t();
		_item_t const* item=m_root->get_at(position);
		if(item==nullptr)
			return _item_t();
		return *item;
		}
	std::size_t get_count()const noexcept
		{
		if(!m_root)
			return 0;
		return m_root->get_item_count();
		}
	std::size_t get_many(std::size_t position, std::size_t count, _item_t* items)const noexcept
		{
		if(!m_root)
			return 0;
		return m_root->get_many(position, count, items);
		}

	// Modification
	void append(_item_t const& item)noexcept
		{
		if(!m_root)
			m_root=new _item_group_t();
		if(m_root->append(item, false))
			return;
		m_root=new _parent_group_t(m_root);
		m_root->append(item, true);
		}
	void append(_item_t const* items, std::size_t count)noexcept
		{
		if(!m_root)
			m_root=new _item_group_t();
		std::size_t pos=0;
		while(pos<count)
			{
			pos+=m_root->append(&items[pos], count-pos);
			if(pos==count)
				break;
			m_root=new _parent_group_t(m_root);
			}
		}
	void clear()noexcept
		{
		if(m_root)
			{
			delete m_root;
			m_root=nullptr;
			}
		}
	bool insert_at(std::size_t position, _item_t const& item)noexcept
		{
		if(!m_root)
			{
			if(position>0)
				return false;
			m_root=new _item_group_t();
			return m_root->append(item, false);
			}
		if(position>m_root->get_item_count())
			return false;
		if(m_root->insert_at(position, item, false))
			return true;
		m_root=new _parent_group_t(m_root);
		return m_root->insert_at(position, item, true);
		}
	bool remove_at(std::size_t position)noexcept
		{
		if(!m_root)
			return false;
		if(m_root->remove_at(position))
			{
			update_root();
			return true;
			}
		return false;
		}
	bool set_at(std::size_t position, _item_t const& item)noexcept
		{
		if(!m_root)
			return false;
		return m_root->set_at(position, item);
		}
	std::size_t set_many(std::size_t position, std::size_t count, _item_t const* items)
		{
		std::size_t pos=0;
		if(m_root)
			{
			pos=m_root->set_many(position, count, items);
			if(pos==count)
				return count;
			}
		append(&items[pos], count-pos);
		return count;
		}

protected:
	// Con-/Destructors
	_list_cluster()noexcept: m_root(nullptr) {}
	_list_cluster(_list_cluster const& list)noexcept: m_root(nullptr)
		{
		if(!list.m_root)
			return;
		if(list.m_root->get_level()>0)
			{
			m_root=new _parent_group_t((_parent_group_t const&)*list.m_root);
			}
		else
			{
			m_root=new _item_group_t((_item_group_t const&)*list.m_root);
			}
		}
	~_list_cluster()noexcept
		{
		if(m_root)
			delete m_root;
		}

private:
	// Common
	void update_root()noexcept
		{
		if(m_root->get_level()==0)
			{
			if(m_root->get_child_count()==0)
				{
				delete m_root;
				m_root=nullptr;
				}
			return;
			}
		if(m_root->get_child_count()>1)
			return;
		_parent_group_t* root=(_parent_group_t*)m_root;
		m_root=root->get_child(0);
		root->set_child_count(0);
		delete root;
		}
	_group_t* m_root;
};


//=====================
// Iterator base-class
//=====================

template <typename _item_t, uint16_t _group_size, bool _is_const>
class _list_iterator_base
{
protected:
	// Using
	using _base_t=_list_iterator_base<_item_t, _group_size, _is_const>;
	using _group_t=_list_group<_item_t>;
	using _item_group_t=_list_item_group<_item_t, _group_size>;
	using _list_t=_list_cluster<_item_t, _group_size>;
	using _list_ptr_t=typename std::conditional<_is_const, _list_t const*, _list_t*>::type;
	using _parent_group_t=_list_parent_group<_item_t, _group_size>;

public:
	// Access
	_item_t get_current()const noexcept
		{
		if(m_current==nullptr)
			return _item_t();
		return *m_current;
		}
	std::size_t get_position()const noexcept
		{
		if(m_level_count==0)
			return 0;
		std::size_t pos=0;
		for(uint16_t u=0; u<m_level_count-1; u++)
			{
			_parent_group_t* group=(_parent_group_t*)m_its[u].group;
			uint16_t grouppos=m_its[u].position;
			for(uint16_t v=0; v<grouppos; v++)
				pos+=group->get_child(v)->get_item_count();
			}
		pos+=m_its[m_level_count-1].position;
		return pos;
		}
	inline bool has_current()const noexcept { return m_current!=nullptr; }

	// Modification
	_base_t& operator=(_base_t const& it)noexcept
		{
		m_current=it.m_current;
		m_list=it.m_list;
		if(set_level_count(it.m_level_count))
			memcpy(m_its, it.m_its, m_level_count*sizeof(_it_struct));
		return *this;
		}
	bool move_next()noexcept
		{
		if(m_its==nullptr)
			return false;
		_it_struct* it=&m_its[m_level_count-1];
		_item_group_t* itemgroup=(_item_group_t*)it->group;
		uint16_t count=itemgroup->get_child_count();
		if(it->position+1<count)
			{
			it->position++;
			m_current=itemgroup->get_at(it->position);
			return true;
			}
		for(uint16_t u=(uint16_t)(m_level_count-1); u>0; u--)
			{
			it=&m_its[u-1];
			_parent_group_t* parentgroup=(_parent_group_t*)it->group;
			count=parentgroup->get_child_count();
			if(it->position+1>=count)
				continue;
			it->position++;
			_group_t* group=it->group;
			for(; u<m_level_count; u++)
				{
				parentgroup=(_parent_group_t*)group;
				group=parentgroup->get_child(it->position);
				it=&m_its[u];
				it->group=group;
				it->position=0;
				}
			itemgroup=(_item_group_t*)group;
			m_current=itemgroup->get_at(0);
			return true;
			}
		m_current=nullptr;
		return false;
		}
	bool move_previous()noexcept
		{
		if(m_its==nullptr)
			return false;
		_it_struct* it=&m_its[m_level_count-1];
		_item_group_t* itemgroup=(_item_group_t*)it->group;
		if(it->position>0)
			{
			it->position--;
			m_current=itemgroup->get_at(it->position);
			return true;
			}
		for(uint16_t u=m_level_count-1; u>0; u--)
			{
			it=&m_its[u-1];
			_parent_group_t* parentgroup=(_parent_group_t*)it->group;
			if(it->position==0)
				continue;
			it->position--;
			_group_t* group=it->group;
			uint16_t pos=0;
			for(; u<m_level_count; u++)
				{
				parentgroup=(_parent_group_t*)group;
				group=parentgroup->get_child(it->position);
				pos=group->get_child_count()-1;
				it=&m_its[u];
				it->group=group;
				it->position=pos;
				}
			itemgroup=(_item_group_t*)group;
			m_current=itemgroup->get_at(pos);
			return true;
			}
		m_current=nullptr;
		return false;
		}
	bool set_position(std::size_t position)noexcept
		{
		m_current=nullptr;
		_group_t* group=m_list->m_root;
		if(!group)
			return false;
		uint16_t levelcount=(uint16_t)(group->get_level()+1);
		if(!set_level_count(levelcount))
			return false;
		uint16_t pos=get_position_internal(group, &position);
		if(pos==_group_size)
			return false;
		m_its[0].group=group;
		m_its[0].position=pos;
		for(uint16_t u=0; u<m_level_count-1; u++)
			{
			_parent_group_t* parentgroup=(_parent_group_t*)m_its[u].group;
			group=parentgroup->get_child(pos);
			pos=get_position_internal(group, &position);
			if(pos==_group_size)
				return false;
			m_its[u+1].group=group;
			m_its[u+1].position=pos;
			}
		if(pos<group->get_child_count())
			{
			_item_group_t* itemgroup=(_item_group_t*)group;
			m_current=itemgroup->get_at(pos);
			return true;
			}
		return false;
		}

protected:
	// Con-/Destructors
	_list_iterator_base(_base_t const& it)noexcept:
		m_current(nullptr), m_its(nullptr), m_level_count(0), m_list(it.m_list)
		{
		if(set_level_count(it.m_level_count))
			{
			for(uint16_t u=0; u<m_level_count; u++)
				m_its[u]=it.m_its[u];
			m_current=it.m_current;
			}
		}
	_list_iterator_base(_list_ptr_t list)noexcept:
		m_current(nullptr), m_its(nullptr), m_level_count(0), m_list(list) {}
	_list_iterator_base(_list_ptr_t list, std::size_t position)noexcept:
		_list_iterator_base(list) { set_position(position); }
	~_list_iterator_base()noexcept
		{
		if(m_its!=nullptr)
			operator delete(m_its);
		}

	// Helper-struct
	typedef struct
		{
		_group_t* group;
		uint16_t position;
		}_it_struct;

	// Common
	uint16_t get_position_internal(_group_t* group, std::size_t* pos)const noexcept
		{
		uint16_t count=group->get_child_count();
		uint16_t level=group->get_level();
		if(level==0)
			{
			uint16_t u=(uint16_t)*pos;
			*pos=0;
			return u;
			}
		_parent_group_t* parentgroup=(_parent_group_t*)group;
		std::size_t itemcount=0;
		for(uint16_t u=0; u<count; u++)
			{
			_group_t* child=parentgroup->get_child(u);
			itemcount=child->get_item_count();
			if(*pos<itemcount)
				return u;
			*pos-=itemcount;
			}
		return _group_size;
		}
	bool set_level_count(uint16_t levelcount)noexcept
		{
		if(m_level_count==levelcount)
			return true;
		if(m_its!=nullptr)
			operator delete(m_its);
		m_its=(_it_struct*)operator new(levelcount*sizeof(_it_struct));
		m_level_count=m_its? levelcount: 0;
		return m_level_count==levelcount;
		}
	_item_t* m_current;
	_it_struct* m_its;
	uint16_t m_level_count;
	_list_ptr_t m_list;
};


//==========
// Iterator
//==========

template <typename _item_t, uint16_t _group_size>
class _list_iterator: public _list_iterator_base<_item_t, _group_size, false>
{
private:
	// Using
	using _base_t=_list_iterator_base<_item_t, _group_size, false>;
	using _it_t=_list_iterator<_item_t, _group_size>;
	using _list_t=_list_cluster<_item_t, _group_size>;

public:
	// Con-/Destructors
	_list_iterator(_it_t const& it)noexcept: _base_t(it) {}
	_list_iterator(_list_t* list)noexcept: _base_t(list) {}
	_list_iterator(_list_t* list, std::size_t position)noexcept: _base_t(list, position) {}

	// Modification
	bool remove_current()noexcept
		{
		if(this->m_current==nullptr)
			return false;
		std::size_t pos=this->get_position();
		this->m_list->remove_at(pos);
		this->set_position(pos);
		return true;
		}
	void set_current(_item_t const& item)noexcept
		{
		if(this->m_current==nullptr)
			return;
		*(this->m_current)=item;
		}
};


//================
// Const-iterator
//================

template <typename _item_t, uint16_t _group_size>
class _list_const_iterator: public _list_iterator_base<_item_t, _group_size, true>
{
private:
	// Using
	using _base_t=_list_iterator_base<_item_t, _group_size, true>;
	using _it_t=_list_const_iterator<_item_t, _group_size>;
	using _list_t=_list_cluster<_item_t, _group_size>;

public:
	// Con-/Destructors
	_list_const_iterator(_it_t const& it)noexcept: _base_t(it) {}
	_list_const_iterator(_list_t const* list)noexcept: _base_t(list) {}
	_list_const_iterator(_list_t const* list, std::size_t position)noexcept: _base_t(list, position) {}
};


//======
// List
//======

template <typename _item_t, uint16_t _group_size=10>
class list: public _list_cluster<_item_t, _group_size>
{
private:
	// Using
	using _base_t=_list_cluster<_item_t, _group_size>;

public:
	// Typedefs
	typedef _list_const_iterator<_item_t, _group_size> const_iterator;
	typedef _list_iterator<_item_t, _group_size> iterator;

	// Con-/Destructors
	list()noexcept {}
	list(list const& list)noexcept: _base_t(list) {}

	// Iteration
	inline iterator at(std::size_t position)noexcept { return iterator(this, position); }
	inline const_iterator at(std::size_t position)const noexcept { return const_iterator(this, position); }
	inline iterator at(iterator const& it)noexcept { return iterator(it); }
	inline const_iterator at(const_iterator const& it)const noexcept { return const_iterator(it); }
	inline iterator first()noexcept { return iterator(this, 0); }
	inline const_iterator first()const noexcept { return const_iterator(this, 0); }
	inline iterator last()noexcept { return iterator(this, this->get_count()-1); }
	inline const_iterator last()const noexcept { return const_iterator(this, this->get_count()-1); }
};

} // namespace

#endif // _CLUSTERS_LIST_HPP
