//=========
// slist.h
//=========

// Implementation of a sorted list.
// Items and can be inserted, removed and looked-up in real-time.

// Copyright 2019, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _CLUSTERS_SLIST
#define _CLUSTERS_SLIST


//=======
// Using
//=======

#include <cstring>
#include <new>
#include <type_traits>
#include <utility>


//===========
// Namespace
//===========

namespace Clusters {


//======================
// Forward-Declarations
//======================

template <typename _id_t, typename _item_t, unsigned int _group_size> class slist;


//======
// Item
//======

template <typename _id_t, typename _item_t>
class _slist_item
{
public:
	_slist_item(_slist_item const& item)noexcept: m_id(item.m_id), m_item(item.m_item) {}
	_slist_item(_slist_item && item)noexcept: m_id(std::move(item.m_id)), m_item(std::move(item.m_item)) {}
	_slist_item(_id_t const& id, _item_t const* item)noexcept: m_id(id), m_item(*item) {}
	_id_t const& get_id()const { return m_id; }
	_item_t& get_item() { return m_item; }
	void set(_item_t const* item)noexcept { m_item=*item; }

private:
	_id_t m_id;
	_item_t m_item;
};

template <typename _id_t>
class _slist_item<_id_t, void>
{
public:
	_slist_item(_slist_item const& item)noexcept: m_id(item.m_id) {}
	_slist_item(_slist_item && item)noexcept: m_id(std::move(item.m_id)) {}
	_slist_item(_id_t const& id, void const*)noexcept: m_id(id) {}
	_id_t const& get_id()const { return m_id; }

private:
	_id_t m_id;
};


//=======
// Group
//=======

template <typename _id_t, typename _item_t>
class _slist_group
{
private:
	// Using 
	using _slist_item_t=_slist_item<_id_t, _item_t>;

public:
	// Con-Destructors
	virtual ~_slist_group()noexcept {}

	// Access
	virtual bool contains(_id_t const& id)const noexcept=0;
	virtual int find(_id_t const& id)const noexcept=0;
	virtual _slist_item_t* get(_id_t const& id)noexcept=0;
	virtual _slist_item_t const* get(_id_t const& id)const noexcept=0;
	virtual _slist_item_t* get_at(std::size_t position)noexcept=0;
	virtual _slist_item_t const* get_at(std::size_t position)const noexcept=0;
	virtual unsigned int get_child_count()const noexcept=0;
	virtual _slist_item_t* get_first()noexcept=0;
	virtual _slist_item_t const* get_first()const noexcept=0;
	virtual std::size_t get_item_count()const noexcept=0;
	virtual _slist_item_t* get_last()noexcept=0;
	virtual _slist_item_t const* get_last()const noexcept=0;
	virtual unsigned int get_level()const noexcept=0;

	// Modification
	virtual bool add(_id_t const& id, _item_t const* item, bool again, bool* exists)noexcept=0;
	virtual bool remove(_id_t const& id)noexcept=0;
	virtual bool remove_at(std::size_t position)noexcept=0;
	virtual bool set(_id_t const& id, _item_t const* item, bool again, bool* exists)noexcept=0;
};


//============
// Item-group
//============

template <typename _id_t, typename _item_t, unsigned int _group_size>
class _slist_item_group: public _slist_group<_id_t, _item_t>
{
private:
	// Using
	using _slist_item_t=_slist_item<_id_t, _item_t>;

public:
	// Con-/Destructors
	_slist_item_group()noexcept: m_item_count(0) {}
	_slist_item_group(_slist_item_group const& group)noexcept: m_item_count(group.m_item_count)
		{
		_slist_item_t* dst=get_items();
		_slist_item_t const* src=group.get_items();
		for(unsigned int u=0; u<m_item_count; u++)
			new (&dst[u]) _slist_item_t(src[u]);
		}
	~_slist_item_group()noexcept override
		{
		_slist_item_t* items=get_items();
		for(unsigned int u=0; u<m_item_count; u++)
			items[u].~_slist_item_t();
		}

	// Access
	inline bool contains(_id_t const& id)const noexcept override { return get_item_pos(id)>=0; }
	inline int find(_id_t const& id)const noexcept override { return get_item_pos(id); }
	_slist_item_t* get(_id_t const& id)noexcept override
		{
		int ipos=get_item_pos(id);
		if(ipos<0)
			return nullptr;
		return &get_items()[ipos];
		}
	_slist_item_t const* get(_id_t const& id)const noexcept override
		{
		int ipos=get_item_pos(id);
		if(ipos<0)
			return nullptr;
		return &get_items()[ipos];
		}
	_slist_item_t* get_at(std::size_t position)noexcept override
		{
		if(position>=m_item_count)
			return nullptr;
		return &get_items()[position];
		}
	_slist_item_t const* get_at(std::size_t position)const noexcept override
		{
		if(position>=m_item_count)
			return nullptr;
		return &get_items()[position];
		}
	inline unsigned int get_child_count()const noexcept override { return m_item_count; }
	_slist_item_t* get_first()noexcept
		{
		if(m_item_count==0)
			return nullptr;
		return get_items();
		}
	_slist_item_t const* get_first()const noexcept
		{
		if(m_item_count==0)
			return nullptr;
		return get_items();
		}
	inline std::size_t get_item_count()const noexcept override { return m_item_count; }
	inline _slist_item_t* get_items()noexcept { return (_slist_item_t*)m_items; }
	inline _slist_item_t const* get_items()const noexcept { return (_slist_item_t const*)m_items; }
	_slist_item_t* get_last()noexcept
		{
		if(m_item_count==0)
			return nullptr;
		return &get_items()[m_item_count-1];
		}
	_slist_item_t const* get_last()const noexcept
		{
		if(m_item_count==0)
			return nullptr;
		return &get_items()[m_item_count-1];
		}
	inline unsigned int get_level()const noexcept override { return 0; }

	// Modification
	bool add(_id_t const& id, _item_t const* item, bool again, bool* exists)noexcept override
		{
		int pos=get_insert_pos(id, exists);
		if(*exists)
			return false;
		return add_internal(id, item, pos);
		}
	void append_items(_slist_item_t* append, unsigned int count)noexcept
		{
		_slist_item_t* items=get_items();
		for(unsigned int u=0; u<count; u++)
			new (&items[m_item_count+u]) _slist_item_t(std::move(append[u]));
		m_item_count+=count;
		}
	void insert_items(unsigned int position, _slist_item_t* insert, unsigned int count)noexcept
		{
		_slist_item_t* items=get_items();
		for(unsigned int u=m_item_count+count-1; u>=position+count; u--)
			new (&items[u]) _slist_item_t(std::move(items[u-count]));
		for(unsigned int u=0; u<count; u++)
			new (&items[position+u]) _slist_item_t(std::move(insert[u]));
		m_item_count+=count;
		}
	bool remove(_id_t const& id)noexcept override
		{
		int pos=get_item_pos(id);
		if(pos<0)
			return false;
		return remove_at(pos);
		}
	bool remove_at(std::size_t position)noexcept override
		{
		if(position>=m_item_count)
			return false;
		_slist_item_t* items=get_items();
		items[position].~_slist_item_t();
		for(unsigned int u=(unsigned int)position; u+1<m_item_count; u++)
			new (&items[u]) _slist_item_t(std::move(items[u+1]));
		m_item_count--;
		return true;
		}
	void remove_items(unsigned int position, unsigned int count)noexcept
		{
		_slist_item_t* items=get_items();
		for(unsigned int u=position; u+count<m_item_count; u++)
			new (&items[u]) _slist_item_t(std::move(items[u+count]));
		m_item_count-=count;
		}
	bool set(_id_t const& id, _item_t const* item, bool again, bool *exists)noexcept override
		{
		int pos=get_insert_pos(id, exists);
		if(*exists)
			{
			_slist_item_t* items=get_items();
			items[pos].set(item);
			return true;
			}
		return add_internal(id, item, pos);
		}

private:
	// Access
	int get_insert_pos(_id_t const& id, bool* exists)const noexcept
		{
		_slist_item_t const* items=get_items();
		unsigned int start=0;
		unsigned int end=m_item_count;
		while(start<end)
			{
			unsigned int u=start+(end-start)/2;
			if(items[u].get_id()>id)
				{
				end=u;
				continue;
				}
			if(items[u].get_id()<id)
				{
				start=u+1;
				continue;
				}
			*exists=true;
			return u;
			}
		return start;
		}
	int get_item_pos(_id_t const& id)const noexcept
		{
		if(!m_item_count)
			return -1;
		_slist_item_t const* items=get_items();
		_slist_item_t const* item=nullptr;
		unsigned int start=0;
		unsigned int end=m_item_count;
		unsigned int u=0;
		while(start<end)
			{
			u=start+(end-start)/2;
			item=&items[u];
			if(item->get_id()>id)
				{
				end=u;
				continue;
				}
			if(item->get_id()<id)
				{
				start=u+1;
				continue;
				}
			return u;
			}
		if(u>0&&item->get_id()>id)
			u--;
		return -(int)u-1;
		}

	// Modification
	bool add_internal(_id_t const& id, _item_t const* item, unsigned int pos)
		{
		if(m_item_count==_group_size)
			return false;
		_slist_item_t* items=get_items();
		for(unsigned int u=m_item_count; u>pos; u--)
			new (&items[u]) _slist_item_t(std::move(items[u-1]));
		new (&items[pos]) _slist_item_t(id, item);
		m_item_count++;
		return true;
		}

	// Uninitialized array of items
	unsigned int m_item_count;
	alignas(alignof(_slist_item_t[_group_size])) unsigned char m_items[sizeof(_slist_item_t[_group_size])];
};


//==============
// Parent-group
//==============

template <typename _id_t, typename _item_t, unsigned int _group_size>
class _slist_parent_group: public _slist_group<_id_t, _item_t>
{
private:
	// Using
	using _group_t=_slist_group<_id_t, _item_t>;
	using _item_group_t=_slist_item_group<_id_t, _item_t, _group_size>;
	using _parent_group_t=_slist_parent_group<_id_t, _item_t, _group_size>;
	using _slist_item_t=_slist_item<_id_t, _item_t>;

public:
	// Con-Destructors
	_slist_parent_group(unsigned int level)noexcept:
		m_child_count(0), m_first(nullptr), m_item_count(0), m_last(nullptr), m_level(level) {}
	_slist_parent_group(_group_t* child)noexcept:
		m_child_count(1), m_first(child->get_first()), m_item_count(child->get_item_count()),
		m_last(child->get_last()), m_level(child->get_level()+1)
		{
		m_children[0]=child;
		}
	_slist_parent_group(_parent_group_t const& group)noexcept:
		m_child_count(group.m_child_count), m_item_count(group.m_item_count), m_level(group.m_level)
		{
		if(m_level>1)
			{
			for(unsigned int u=0; u<m_child_count; u++)
				m_children[u]=new _parent_group_t((_parent_group_t const&)*group.m_children[u]);
			}
		else
			{
			for(unsigned int u=0; u<m_child_count; u++)
				m_children[u]=new _item_group_t((_item_group_t const&)*group.m_children[u]);
			}
		m_first=m_children[0]->get_first();
		m_last=m_children[m_child_count-1]->get_last();
		}
	~_slist_parent_group()noexcept override
		{
		for(unsigned int u=0; u<m_child_count; u++)
				delete m_children[u];
		}

	// Access
	inline bool contains(_id_t const& id)const noexcept override { return get_item_pos(id)>=0; }
	inline int find(_id_t const& id)const noexcept override { return get_item_pos(id); }
	_slist_item_t* get(_id_t const& id)noexcept override
		{
		int ipos=get_item_pos(id);
		if(ipos<0)
			return nullptr;
		return m_children[ipos]->get(id);
		}
	_slist_item_t const* get(_id_t const& id)const noexcept override
		{
		int ipos=get_item_pos(id);
		if(ipos<0)
			return nullptr;
		return m_children[ipos]->get(id);
		}
	_slist_item_t* get_at(std::size_t position)noexcept override
		{
		unsigned int group=get_group(&position);
		if(group>=_group_size)
			return nullptr;
		return m_children[group]->get_at(position);
		}
	_slist_item_t const* get_at(std::size_t position)const noexcept override
		{
		unsigned int group=get_group(&position);
		if(group>=_group_size)
			return nullptr;
		return m_children[group]->get_at(position);
		}
	_group_t* get_child(unsigned int position)const noexcept
		{
		if(position>=m_child_count)
			return nullptr;
		return m_children[position];
		}
	inline unsigned int get_child_count()const noexcept override { return m_child_count; }
	inline _slist_item_t* get_first()noexcept override { return m_first; }
	inline _slist_item_t const* get_first()const noexcept override { return const_cast<_slist_item_t const*>(m_first); }
	inline std::size_t get_item_count()const noexcept override { return m_item_count; }
	inline _slist_item_t* get_last()noexcept override { return m_last; }
	inline _slist_item_t const* get_last()const noexcept override { return const_cast<_slist_item_t const*>(m_last); }
	inline unsigned int get_level()const noexcept override { return m_level; }

	// Modification
	bool add(_id_t const& id, _item_t const* item, bool again, bool* exists)noexcept override
		{
		if(add_internal(id, item, again, exists))
			{
			m_item_count++;
			update_bounds();
			return true;
			}
		return false;
		}
	void append_groups(_group_t* const* groups, unsigned int count)noexcept
		{
		memcpy(&m_children[m_child_count], groups, count*sizeof(_group_t*));
		for(unsigned int u=0; u<count; u++)
			m_item_count+=groups[u]->get_item_count();
		m_child_count+=count;
		update_bounds();
		}
	void insert_groups(unsigned int position, _group_t* const* groups, unsigned int count)noexcept
		{
		for(unsigned int u=m_child_count+count-1; u>=position+count; u--)
			m_children[u]=m_children[u-count];
		memcpy(&m_children[position], groups, count*sizeof(_group_t*));
		for(unsigned int u=0; u<count; u++)
			m_item_count+=groups[u]->get_item_count();
		m_child_count+=count;
		update_bounds();
		}
	void move_children(unsigned int source, unsigned int destination, unsigned int count)noexcept
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
				unsigned int srccount=src->get_child_count();
				dst->insert_groups(0, &src->m_children[srccount-count], count);
				src->remove_groups(srccount-count, count);
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
				_slist_item_t* srcitems=src->get_items();
				unsigned int srccount=src->get_child_count();
				dst->insert_items(0, &srcitems[srccount-count], count);
				src->remove_items(srccount-count, count);
				}
			}
		}
	void move_empty_slot(unsigned int source, unsigned int destination)noexcept
		{
		if(source<destination)
			{
			for(unsigned int u=source; u<destination; u++)
				move_children(u+1, u, 1);
			}
		else
			{
			for(unsigned int u=source; u>destination; u--)
				move_children(u-1, u, 1);
			}
		}
	bool remove(_id_t const& id)noexcept override
		{
		int pos=get_item_pos(id);
		if(pos<0)
			return false;
		if(!m_children[pos]->remove(id))
			return false;
		m_item_count--;
		combine_children(pos);
		update_bounds();
		return true;
		}
	bool remove_at(std::size_t position)noexcept override
		{
		if(position>=m_item_count)
			return false;
		unsigned int group=get_group(&position);
		m_children[group]->remove_at(position);
		m_item_count--;
		combine_children(group);
		update_bounds();
		return true;
		}
	void remove_groups(unsigned int position, unsigned int count)noexcept
		{
		for(unsigned int u=0; u<count; u++)
			m_item_count-=m_children[position+u]->get_item_count();
		for(unsigned int u=position; u+count<m_child_count; u++)
			m_children[u]=m_children[u+count];
		m_child_count-=count;
		update_bounds();
		}
	bool set(_id_t const& id, _item_t const* item, bool again, bool* exists)noexcept override
		{
		if(set_internal(id, item, again, exists))
			{
			if(!(*exists))
				{
				m_item_count++;
				update_bounds();
				}
			return true;
			}
		return false;
		}
	inline void set_child_count(unsigned int count)noexcept { m_child_count=count; }

private:
	// Access
	unsigned int get_group(std::size_t* position)const noexcept
		{
		for(unsigned int u=0; u<m_child_count; u++)
			{
			std::size_t count=m_children[u]->get_item_count();
			if(*position<count)
				return u;
			*position-=count;
			}
		return _group_size;
		}
	int get_item_pos(_id_t const& id)const noexcept
		{
		if(!m_child_count)
			return -1;
		unsigned int start=0;
		unsigned int end=m_child_count;
		unsigned int u=0;
		_slist_item_t* first=nullptr;
		_slist_item_t* last=nullptr;
		while(start<end)
			{
			u=start+(end-start)/2;
			first=m_children[u]->get_first();
			if(first->get_id()>id)
				{
				end=u;
				continue;
				}
			last=m_children[u]->get_last();
			if(last->get_id()<id)
				{
				start=u+1;
				continue;
				}
			return u;
			}
		if(u>0&&first->get_id()>id)
			u--;
		return -(int)u-1;
		}
	unsigned int get_insert_pos(_id_t const& id, unsigned int* group, bool* exists)const noexcept
		{
		if(!m_child_count)
			return 0;
		unsigned int start=0;
		unsigned int end=m_child_count;
		_slist_item_t* first=nullptr;
		_slist_item_t* last=nullptr;
		while(start<end)
			{
			unsigned int u=start+(end-start)/2;
			first=m_children[u]->get_first();
			last=m_children[u]->get_last();
			if(first->get_id()>=id)
				{
				if(first->get_id()==id)
					*exists=true;
				end=u;
				continue;
				}
			if(last->get_id()<=id)
				{
				if(last->get_id()==id)
					*exists=true;
				start=u+1;
				continue;
				}
			start=u;
			break;
			}
		if(start>m_child_count-1)
			start=m_child_count-1;
		*group=start;
		if(start>0)
			{
			first=m_children[start]->get_first();
			if(first->get_id()>=id)
				{
				*group=start-1;
				return 2;
				}
			}
		if(start+1<m_child_count)
			{
			last=m_children[start]->get_last();
			if(last->get_id()<=id)
				return 2;
			}
		return 1;
		}
	unsigned int get_nearest(unsigned int position)const noexcept
		{
		int before=position-1;
		unsigned int after=position+1;
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
	bool add_internal(_id_t const& id, _item_t const* item, bool again, bool* exists)noexcept
		{
		unsigned int group=0;
		unsigned int count=get_insert_pos(id, &group, exists);
		if(*exists)
			return false;
		if(!again)
			{
			for(unsigned int u=0; u<count; u++)
				{
				if(m_children[group+u]->add(id, item, false, exists))
					return true;
				if(*exists)
					return false;
				}
			if(shift_children(group, count))
				{
				count=get_insert_pos(id, &group, exists);
				if(*exists)
					return false;
				for(unsigned int u=0; u<count; u++)
					{
					if(m_children[group+u]->add(id, item, false, exists))
						return true;
					if(*exists)
						return false;
					}
				}
			}
		if(!split_child(group))
			return false;
		get_insert_pos(id, &group, exists);
		if(*exists)
			return false;
		return m_children[group]->add(id, item, true, exists);
		}
	bool combine_children(unsigned int position)noexcept
		{
		unsigned int count=m_children[position]->get_child_count();
		if(count==0)
			{
			remove_internal(position);
			return true;
			}
		if(position>0)
			{
			if(count+m_children[position-1]->get_child_count()<=_group_size)
				{
				move_children(position, position-1, count);
				remove_internal(position);
				return true;
				}
			}
		if(position+1<m_child_count)
			{
			if(count+m_children[position+1]->get_child_count()<=_group_size)
				{
				move_children(position, position+1, count);
				remove_internal(position);
				return true;
				}
			}
		return false;
		}
	void remove_internal(unsigned int position)noexcept
		{
		delete m_children[position];
		for(unsigned int u=position; u+1<m_child_count; u++)
			m_children[u]=m_children[u+1];
		m_child_count--;
		}
	bool set_internal(_id_t const& id, _item_t const* item, bool again, bool* exists)noexcept
		{
		unsigned int group=0;
		unsigned int count=get_insert_pos(id, &group, exists);
		if(!again)
			{
			for(unsigned int u=0; u<count; u++)
				{
				if(m_children[group+u]->set(id, item, false, exists))
					return true;
				}
			if(shift_children(group, count))
				{
				count=get_insert_pos(id, &group, exists);
				for(unsigned int u=0; u<count; u++)
					{
					if(m_children[group+u]->set(id, item, false, exists))
						return true;
					}
				}
			}
		if(!split_child(group))
			return false;
		get_insert_pos(id, &group, exists);
		return m_children[group]->set(id, item, true, exists);
		}
	bool shift_children(unsigned int group, unsigned int count)
		{
		unsigned int empty=get_nearest(group);
		if(empty>=m_child_count)
			return false;
		if(count>1&&empty>group)
			group++;
		move_empty_slot(empty, group);
		return true;
		}
	bool split_child(unsigned int position)noexcept
		{
		if(m_child_count==_group_size)
			return false;
		for(unsigned int u=m_child_count; u>position+1; u--)
			m_children[u]=m_children[u-1];
		if(m_level>1)
			{
			m_children[position+1]=new _parent_group_t(m_level-1);
			}
		else
			{
			m_children[position+1]=new _item_group_t();
			}
		m_child_count++;
		move_children(position, position+1, 1);
		return true;
		}
	void update_bounds()noexcept
		{
		if(!m_child_count)
			return;
		m_first=m_children[0]->get_first();
		m_last=m_children[m_child_count-1]->get_last();
		}
	
	// Common
	unsigned int m_child_count;
	_group_t* m_children[_group_size];
	_slist_item_t* m_first;
	std::size_t m_item_count;
	_slist_item_t* m_last;
	unsigned int m_level;
};


//=========
// Cluster
//=========

// Forward-Declaration
template <typename _id_t, typename _item_t, unsigned int _group_size, bool _is_const> class _slist_iterator_base;

template <typename _id_t, typename _item_t, unsigned int _group_size>
class _slist_cluster
{
private:
	// Using
	using _group_t=_slist_group<_id_t, _item_t>;
	using _item_group_t=_slist_item_group<_id_t, _item_t, _group_size>;
	using _parent_group_t=_slist_parent_group<_id_t, _item_t, _group_size>;

public:
	// Friends
	friend class _slist_iterator_base<_id_t, _item_t, _group_size, true>;
	friend class _slist_iterator_base<_id_t, _item_t, _group_size, false>;

		// Access
	inline bool contains(_id_t const& id)const noexcept { return m_root->contains(id); }
	inline std::size_t get_count()const noexcept { return m_root->get_item_count(); }

	// Modification
	void clear()noexcept
		{
		delete m_root;
		m_root=new _item_group_t();
		}
	bool remove(_id_t const& id)noexcept
		{
		if(m_root->remove(id))
			{
			update_root();
			return true;
			}
		return false;
		}
	void remove_at(std::size_t position)noexcept
		{
		if(m_root->remove_at(position))
			update_root();
		}

protected:
	// Con-/Destructors
	_slist_cluster(): m_root(new _item_group_t()) {}
	_slist_cluster(_slist_cluster const& slist)
		{
		if(slist.m_root->get_level()>0)
			{
			m_root=new _parent_group_t((_parent_group_t const&)*slist.m_root);
			}
		else
			{
			m_root=new _item_group_t((_item_group_t const&)*slist.m_root);
			}
		}
	~_slist_cluster()noexcept { delete m_root; }

	// Common
	void update_root()noexcept
		{
		if(m_root->get_level()==0)
			return;
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

template <typename _id_t, typename _item_t, unsigned int _group_size, bool _is_const>
class _slist_iterator_base
{
protected:
	// Using
	using _base_t=_slist_iterator_base<_id_t, _item_t, _group_size, _is_const>;
	using _group_t=_slist_group<_id_t, _item_t>;
	using _item_group_t=_slist_item_group<_id_t, _item_t, _group_size>;
	using _parent_group_t=_slist_parent_group<_id_t, _item_t, _group_size>;
	using _slist_item_t=_slist_item<_id_t, _item_t>;
	using _slist_t=_slist_cluster<_id_t, _item_t, _group_size>;
	using _slist_ptr_t=typename std::conditional<_is_const, _slist_t const*, _slist_t*>::type;

public:
	// Access
	std::size_t get_position()const noexcept
		{
		if(m_its==nullptr)
			return 0;
		std::size_t pos=0;
		for(unsigned int u=0; u<m_level_count-1; u++)
			{
			_parent_group_t* group=(_parent_group_t*)m_its[u].group;
			unsigned int grouppos=m_its[u].position;
			for(unsigned int v=0; v<grouppos; v++)
				pos+=group->get_child(v)->get_item_count();
			}
		pos+=m_its[m_level_count-1].position;
		return pos;
		}
	inline bool has_current()const noexcept { return m_current!=nullptr; }

	// Assignment
	_base_t& operator=(_base_t const& it)noexcept
		{
		m_current=it.m_current;
		m_slist=it.m_slist;
		if(set_level_count(it.m_level_count))
			memcpy(m_its, it.m_its, m_level_count*sizeof(_it_struct));
		return *this;
		}

	// Modification
	bool find(_id_t const& id)noexcept
		{
		m_current=nullptr;
		bool bfound=true;
		_group_t* group=m_slist->m_root;
		unsigned int levelcount=group->get_level()+1;
		if(!set_level_count(levelcount))
			return false;
		for(unsigned int u=0; u<levelcount-1; u++)
			{
			_parent_group_t* parentgroup=(_parent_group_t*)group;
			int pos=parentgroup->find(id);
			if(pos<0)
				{
				bfound=false;
				pos++;
				pos*=-1;
				}
			m_its[u].group=group;
			m_its[u].position=pos;
			group=parentgroup->get_child(pos);
			}
		_item_group_t* itemgroup=(_item_group_t*)group;
		int pos=itemgroup->find(id);
		if(pos<0)
			{
			bfound=false;
			pos++;
			pos*=-1;
			}
		m_its[levelcount-1].group=group;
		m_its[levelcount-1].position=pos;
		m_current=itemgroup->get_at(pos);
		return bfound;
		}
	bool move_next()noexcept
		{
		if(m_its==nullptr)
			return false;
		_it_struct* it=&m_its[m_level_count-1];
		_item_group_t* itemgroup=(_item_group_t*)it->group;
		unsigned int count=itemgroup->get_child_count();
		if(it->position+1<count)
			{
			it->position++;
			m_current=itemgroup->get_at(it->position);
			return true;
			}
		for(unsigned int u=m_level_count-1; u>0; u--)
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
		for(unsigned int u=m_level_count-1; u>0; u--)
			{
			it=&m_its[u-1];
			_parent_group_t* parentgroup=(_parent_group_t*)it->group;
			if(it->position==0)
				continue;
			it->position--;
			_group_t* group=it->group;
			unsigned int pos=0;
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
		_group_t* group=m_slist->m_root;
		unsigned int levelcount=group->get_level()+1;
		if(!set_level_count(levelcount))
			return false;
		unsigned int pos=get_position_internal(group, &position);
		m_its[0].group=group;
		m_its[0].position=pos;
		for(unsigned int u=0; u<m_level_count-1; u++)
			{
			_parent_group_t* parentgroup=(_parent_group_t*)m_its[u].group;
			group=parentgroup->get_child(pos);
			pos=get_position_internal(group, &position);
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
	_slist_iterator_base(_base_t const& it)noexcept:
		m_current(nullptr), m_slist(it.m_slist), m_its(nullptr), m_level_count(0)
		{
		if(set_level_count(it.m_level_count))
			{
			memcpy(m_its, it.m_its, m_level_count*sizeof(_it_struct));
			m_current=it.m_current;
			}
		}
	_slist_iterator_base(_slist_ptr_t slist)noexcept:
		m_slist(slist), m_its(nullptr), m_level_count(0) {}
	~_slist_iterator_base()noexcept
		{
		if(m_its!=nullptr)
			operator delete(m_its);
		}

	// Helper-struct
	typedef struct
		{
		_group_t* group;
		unsigned int position;
		}_it_struct;

	// Common
	unsigned int get_position_internal(_group_t* group, std::size_t* pos)const noexcept
		{
		unsigned int count=group->get_child_count();
		unsigned int level=group->get_level();
		if(level==0)
			{
			unsigned int u=(unsigned int)*pos;
			*pos=0;
			return u;
			}
		_parent_group_t* parentgroup=(_parent_group_t*)group;
		std::size_t itemcount=0;
		for(unsigned int u=0; u<count; u++)
			{
			_group_t* child=parentgroup->get_child(u);
			itemcount=child->get_item_count();
			if(*pos<itemcount)
				return u;
			*pos-=itemcount;
			}
		return _group_size;
		}
	bool set_level_count(unsigned int levelcount)noexcept
		{
		if(m_level_count==levelcount)
			return true;
		if(m_its!=nullptr)
			operator delete(m_its);
		m_its=(_it_struct*)operator new(levelcount*sizeof(_it_struct));
		m_level_count=m_its? levelcount: 0;
		return m_level_count==levelcount;
		}
	_slist_item_t* m_current;
	_slist_ptr_t m_slist;
	_it_struct* m_its;
	unsigned int m_level_count;
};


//==========
// Iterator
//==========

template <typename _id_t, typename _item_t, unsigned int _group_size>
class _slist_iterator: public _slist_iterator_base<_id_t, _item_t, _group_size, false>
{
private:
	// Using
	using _base_t=_slist_iterator_base<_id_t, _item_t, _group_size, false>;
	using _it_t=_slist_iterator<_id_t, _item_t, _group_size>;
	using _slist_t=_slist_cluster<_id_t, _item_t, _group_size>;

public:
	// Con-/Destructors
	_slist_iterator(_it_t const& it)noexcept: _base_t(it) {}
	_slist_iterator(_slist_t* slist)noexcept: _base_t(slist) {}
	_slist_iterator(_slist_t* slist, std::size_t position)noexcept: _base_t(slist) { this->set_position(position); }
	_slist_iterator(_slist_t* slist, std::size_t, _id_t const& id)noexcept: _base_t(slist) { this->find(id); }

	// Access
	_id_t get_current_id()const noexcept
		{
		if(this->m_current==nullptr)
			return _id_t();
		return this->m_current->get_id();
		}
	_item_t get_current_item()const noexcept
		{
		if(this->m_current==nullptr)
			return _item_t();
		return this->m_current->get_item();
		}

	// Modification
	bool remove_current()noexcept
		{
		if(this->m_current==nullptr)
			return false;
		std::size_t pos=this->get_position();
		this->m_slist->remove_at(pos);
		this->set_position(pos);
		return true;
		}
	void set_current_item(_item_t const& item)noexcept
		{
		if(this->m_current==nullptr)
			return;
		this->m_current->set(item);
		}
};

template <typename _id_t, unsigned int _group_size>
class _slist_iterator<_id_t, void, _group_size>: public _slist_iterator_base<_id_t, void, _group_size, false>
{
private:
	// Using
	using _base_t=_slist_iterator_base<_id_t, void, _group_size, false>;
	using _slist_t=_slist_cluster<_id_t, void, _group_size>;
	using _it_t=_slist_iterator<_id_t, void, _group_size>;

public:
	// Con-/Destructors
	_slist_iterator(_it_t const& it)noexcept: _base_t(it) {}
	_slist_iterator(_slist_t* slist)noexcept: _base_t(slist) {}
	_slist_iterator(_slist_t* slist, std::size_t position)noexcept: _base_t(slist) { this->set_position(position); }
	_slist_iterator(_slist_t* slist, std::size_t, _id_t const& id)noexcept: _base_t(slist) { this->find(id); }

	// Access
	inline _id_t get_current()const noexcept
		{
		if(this->m_current==nullptr)
			return _id_t();
		return this->m_current->get_id();
		}

	// Modification
	bool remove_current()noexcept
		{
		if(this->m_current==nullptr)
			return false;
		std::size_t pos=this->get_position();
		this->m_slist->remove_at(pos);
		this->set_position(pos);
		return true;
		}
};


//================
// Const-iterator
//================

template <typename _id_t, typename _item_t, unsigned int _group_size>
class _slist_const_iterator: public _slist_iterator_base<_id_t, _item_t, _group_size, true>
{
private:
	// Using
	using _base_t=_slist_iterator_base<_id_t, _item_t, _group_size, true>;
	using _it_t=_slist_const_iterator<_id_t, _item_t, _group_size>;
	using _slist_t=_slist_cluster<_id_t, _item_t, _group_size>;

public:
	// Con-/Destructors
	_slist_const_iterator(_it_t const& it)noexcept: _base_t(it) {}
	_slist_const_iterator(_slist_t const* slist)noexcept: _base_t(slist) {}
	_slist_const_iterator(_slist_t const* slist, std::size_t position)noexcept: _base_t(slist) { this->set_position(position); }
	_slist_const_iterator(_slist_t const* slist, std::size_t, _id_t const& id)noexcept: _base_t(slist) { this->find(id); }

	// Access
	inline _id_t get_current_id()const noexcept
		{
		if(this->m_current==nullptr)
			return _id_t();
		return this->m_current->get_id();
		}
	inline _item_t get_current_item()const noexcept
		{
		if(this->m_current==nullptr)
			return _item_t();
		return this->m_current->get_item();
		}
};

template <typename _id_t, unsigned int _group_size>
class _slist_const_iterator<_id_t, void, _group_size>: public _slist_iterator_base<_id_t, void, _group_size, true>
{
private:
	// Using
	using _base_t=_slist_iterator_base<_id_t, void, _group_size, true>;
	using _it_t=_slist_const_iterator<_id_t, void, _group_size>;
	using _slist_t=_slist_cluster<_id_t, void, _group_size>;

public:
	// Con-/Destructors
	_slist_const_iterator(_it_t const& it)noexcept: _base_t(it) {}
	_slist_const_iterator(_slist_t const* slist)noexcept: _base_t(slist) {}
	_slist_const_iterator(_slist_t const* slist, std::size_t position)noexcept: _base_t(slist) { this->set_position(position); }
	_slist_const_iterator(_slist_t const* slist, std::size_t, _id_t const& id)noexcept: _base_t(slist) { this->find(id); }

	// Access
	inline _id_t get_current()const noexcept
		{
		if(this->m_current==nullptr)
			return _id_t();
		return this->m_current->get_id();
		}
};


//==================
// SList base-class
//==================

template <typename _id_t, typename _item_t, unsigned int _group_size>
class _slist_base: public _slist_cluster<_id_t, _item_t, _group_size>
{
private:
	// Using
	using _base_t=_slist_cluster<_id_t, _item_t, _group_size>;
	using _const_it_t=_slist_const_iterator<_id_t, _item_t, _group_size>;
	using _group_t=_slist_group<_id_t, _item_t>;
	using _it_t=_slist_iterator<_id_t, _item_t, _group_size>;
	using _item_group_t=_slist_item_group<_id_t, _item_t, _group_size>;
	using _parent_group_t=_slist_parent_group<_id_t, _item_t, _group_size>;
	using _slist_item_t=_slist_item<_id_t, _item_t>;

public:
	// Iteration
	inline _it_t at(std::size_t position)noexcept { return _it_t(this, position); }
	inline _const_it_t at(std::size_t position)const noexcept { return _const_it_t(this, position); }
	inline _it_t at(_it_t const& it)noexcept { return _it_t(it); }
	inline _const_it_t at(_const_it_t const& it)const noexcept { return _const_it_t(it); }
	inline _it_t find(_id_t const& id)noexcept { return _it_t(this, 0, id); }
	inline _const_it_t find(_id_t const& id)const noexcept { return _const_it_t(this, 0, id); }
	inline _it_t first()noexcept { return _it_t(this, 0); }
	inline _const_it_t first()const noexcept { return _const_it_t(this, 0); }
	inline _it_t last()noexcept { return _it_t(this, this->get_count()-1); }
	inline _const_it_t last()const noexcept { return _const_it_t(this, this->get_count()-1); }

protected:
	// Con-/Destructors
	_slist_base()noexcept {}
	_slist_base(_slist_base const& slist)noexcept: _base_t(slist) {}

	// Modification
	bool add_internal(_id_t const& id, _item_t const* item)noexcept
		{
		bool exists=false;
		if(this->m_root->add(id, item, false, &exists))
			return true;
		if(exists)
			return false;
		this->m_root=new _parent_group_t(this->m_root);
		return this->m_root->add(id, item, true, &exists);
		}
	bool set_internal(_id_t const& id, _item_t const* item)noexcept
		{
		bool exists=false;
		if(this->m_root->set(id, item, false, &exists))
			return true;
		this->m_root=new _parent_group_t(this->m_root);
		return this->m_root->set(id, item, true, &exists);
		}
};


//=============
// SList typed
//=============

template <typename _id_t, typename _item_t, unsigned int _group_size>
class _slist_typed: public _slist_base<_id_t, _item_t, _group_size>
{
private:
	// Using
	using _base_t=_slist_base<_id_t, _item_t, _group_size>;
	using _parent_group_t=_slist_parent_group<_id_t, _item_t, _group_size>;
	using _slist_item_t=_slist_item<_id_t, _item_t>;

public:
	// Con-/Destructors
	_slist_typed()noexcept {}
	_slist_typed(_base_t const& base)noexcept: _base_t(base) {}

	// Access
	inline _item_t operator[](_id_t const& id)const { return get(id); }
	_item_t get(_id_t const& id)const noexcept
		{
		_slist_item_t* item=this->m_root->get(id);
		if(item==nullptr)
			return _item_t();
		return item->get_item();
		}
	bool try_get(_id_t const& id, _item_t* item)const noexcept
		{
		_slist_item_t* pitem=this->m_root->get(id);
		if(pitem==nullptr)
			return false;
		*item=pitem->get_item();
		return true;
		}

	// Modification
	inline bool add(_id_t const& id, _item_t const& item)noexcept { return this->add_internal(id, &item); }
	inline bool set(_id_t const& id, _item_t const& item)noexcept { return this->set_internal(id, &item); }
};

template <typename _id_t, unsigned int _group_size>
class _slist_typed<_id_t, void, _group_size>: public _slist_base<_id_t, void, _group_size>
{
private:
	// Using
	using _base_t=_slist_base<_id_t, void, _group_size>;
	using _parent_group_t=_slist_parent_group<_id_t, void, _group_size>;
	using _slist_item_t=_slist_item<_id_t, void>;

public:
	// Con-/Destructors
	_slist_typed()noexcept {}
	_slist_typed(_base_t const& base)noexcept: _base_t(base) {}

	// Access
	inline _id_t const operator[](std::size_t position)const noexcept { return get_at(position); }
	_id_t get(_id_t const& id)const noexcept
		{
		_slist_item_t* item=this->m_root->get(id);
		if(item==nullptr)
			return _id_t();
		return item->get_id();
		}
	_id_t get_at(std::size_t position)const noexcept
		{
		_slist_item_t* item=this->m_root->get_at(position);
		if(item==nullptr)
			return _id_t();
		return item->get_id();
		}

	// Modification
	inline bool add(_id_t const& id)noexcept { return this->add_internal(id, nullptr); }
};


//=======
// SList
//=======

template <typename _id_t, typename _item_t=void, unsigned int _group_size=10>
class slist: public _slist_typed<_id_t, _item_t, _group_size>
{
private:
	// Using
	using _base_t=_slist_typed<_id_t, _item_t, _group_size>;

public:
	// Typedefs
	typedef _slist_const_iterator<_id_t, _item_t, _group_size> const_iterator;
	typedef _slist_item<_id_t, _item_t> item;
	typedef _slist_iterator<_id_t, _item_t, _group_size> iterator;

	// Con-/Destructors
	slist()noexcept {}
	slist(slist const& slist)noexcept: _base_t(slist) {}
};

} // namespace

#endif // _CLUSTERS_SLIST
