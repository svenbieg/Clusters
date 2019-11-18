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
#include <exception>
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
	//_slist_item()noexcept {}
	_slist_item(_slist_item const& item)noexcept: id(item.id), item(item.item) {}
	_slist_item(_slist_item && item)noexcept: id(std::move(item.id)), item(std::move(item.item)) {}
	_slist_item(_id_t const& id, _item_t const* item)noexcept: id(id), item(*item) {}
	/*_slist_item& operator=(_slist_item const& ii)noexcept
		{
		id=ii.id;
		item=ii.item;
		return *this;
		}*/
	_id_t id;
	_item_t item;
};

template <typename _id_t>
class _slist_item<_id_t, void>
{
public:
	//_slist_item()noexcept {}
	_slist_item(_slist_item const& item)noexcept: id(item.id) {}
	_slist_item(_slist_item && item)noexcept: id(std::move(item.id)) {}
	_slist_item(_id_t const& id, void const*)noexcept: id(id) {}
	/*_slist_item& operator=(_slist_item const& item)noexcept
		{
		id=item.id;
		return *this;
		}*/
	_id_t id;
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
	virtual _slist_item_t* get_at(size_t position)noexcept=0;
	virtual _slist_item_t const* get_at(size_t position)const noexcept=0;
	virtual unsigned int get_child_count()const noexcept=0;
	virtual _slist_item_t* get_first()noexcept=0;
	virtual _slist_item_t const* get_first()const noexcept=0;
	virtual size_t get_item_count()const noexcept=0;
	virtual _slist_item_t* get_last()noexcept=0;
	virtual _slist_item_t const* get_last()const noexcept=0;
	virtual unsigned int get_level()const noexcept=0;

	// Modification
	virtual bool add(_id_t const& id, _item_t const* item, bool again, bool once, bool* exists)noexcept=0;
	virtual bool remove(_id_t const& id)noexcept=0;
	virtual bool remove_at(size_t position)noexcept=0;
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
	_slist_item_group()noexcept: _m_item_count(0) {}
	_slist_item_group(_slist_item_group const& group)noexcept: _m_item_count(group._m_item_count)
		{
		_slist_item_t* dst=get_items();
		_slist_item_t const* src=group.get_items();
		for(unsigned int u=0; u<_m_item_count; u++)
			new (&dst[u]) _slist_item_t(src[u]);
		}
	~_slist_item_group()noexcept override
		{
		_slist_item_t* items=get_items();
		for(unsigned int u=0; u<_m_item_count; u++)
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
	_slist_item_t* get_at(size_t position)noexcept override
		{
		if(position>=_m_item_count)
			return nullptr;
		return &get_items()[position];
		}
	_slist_item_t const* get_at(size_t position)const noexcept override
		{
		if(position>=_m_item_count)
			return nullptr;
		return &get_items()[position];
		}
	inline unsigned int get_child_count()const noexcept override { return _m_item_count; }
	_slist_item_t* get_first()noexcept
		{
		if(_m_item_count==0)
			return nullptr;
		return get_items();
		}
	_slist_item_t const* get_first()const noexcept
		{
		if(_m_item_count==0)
			return nullptr;
		return get_items();
		}
	inline size_t get_item_count()const noexcept override { return _m_item_count; }
	inline _slist_item_t* get_items()noexcept { return (_slist_item_t*)_m_items; }
	inline _slist_item_t const* get_items()const noexcept { return (_slist_item_t const*)_m_items; }
	_slist_item_t* get_last()noexcept
		{
		if(_m_item_count==0)
			return nullptr;
		return &get_items()[_m_item_count-1];
		}
	_slist_item_t const* get_last()const noexcept
		{
		if(_m_item_count==0)
			return nullptr;
		return &get_items()[_m_item_count-1];
		}
	inline unsigned int get_level()const noexcept override { return 0; }

	// Modification
	bool add(_id_t const& id, _item_t const* item, bool again, bool once, bool* exists)noexcept override
		{
		int pos=get_insert_pos(id, exists);
		if(once&&*exists)
			return false;
		if(_m_item_count==_group_size)
			return false;
		_slist_item_t* items=get_items();
		for(int i=_m_item_count; i>pos; i--)
			new (&items[i]) _slist_item_t(std::move(items[i-1]));
		new (&items[pos]) _slist_item_t(id, item);
		_m_item_count++;
		return true;
		}
	void append_items(_slist_item_t const* append, unsigned int count)noexcept
		{
		_slist_item_t* items=get_items();
		for(unsigned int u=0; u<count; u++)
			new (&items[_m_item_count+u]) _slist_item_t(std::move(append[u]));
		_m_item_count+=count;
		}
	void insert_items(unsigned int position, _slist_item_t const* insert, unsigned int count)noexcept
		{
		_slist_item_t* items=get_items();
		for(unsigned int u=_m_item_count+count-1; u>=position+count; u--)
			new (&items[u]) _slist_item_t(std::move(items[u-count]));
		for(unsigned int u=0; u<count; u++)
			new (&items[position+u]) _slist_item_t(std::move(insert[u]));
		_m_item_count+=count;
		}
	bool remove(_id_t const& id)noexcept override
		{
		int pos=get_item_pos(id);
		if(pos<0)
			return false;
		return remove_at(pos);
		}
	bool remove_at(size_t position)noexcept override
		{
		if(position>=_m_item_count)
			return false;
		_slist_item_t* items=get_items();
		items[position].~_slist_item_t();
		for(unsigned int u=(unsigned int)position; u+1<_m_item_count; u++)
			new (&items[u]) _slist_item_t(std::move(items[u+1]));
		_m_item_count--;
		return true;
		}
	void remove_items(unsigned int position, unsigned int count)noexcept
		{
		_slist_item_t* items=get_items();
		for(unsigned int u=position; u+count<_m_item_count; u++)
			new (&items[u]) _slist_item_t(std::move(items[u+count]));
		_m_item_count-=count;
		}

private:
	// Common
	int get_insert_pos(_id_t const& id, bool* exists)const noexcept
		{
		_slist_item_t const* items=get_items();
		unsigned int start=0;
		unsigned int end=_m_item_count;
		while(start<end)
			{
			unsigned int u=start+(end-start)/2;
			if(items[u].id>id)
				{
				end=u;
				continue;
				}
			if(items[u].id<id)
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
		if(!_m_item_count)
			return -1;
		_slist_item_t const* items=get_items();
		_slist_item_t const* item=nullptr;
		unsigned int start=0;
		unsigned int end=_m_item_count;
		unsigned int u=0;
		while(start<end)
			{
			u=start+(end-start)/2;
			item=&items[u];
			if(item->id>id)
				{
				end=u;
				continue;
				}
			if(item->id<id)
				{
				start=u+1;
				continue;
				}
			return u;
			}
		if(u>0&&item->id>id)
			u--;
		return -(int)u-1;
		}

	// Uninitialized array of items
	unsigned int _m_item_count;
	alignas(alignof(_slist_item_t[_group_size])) unsigned char _m_items[sizeof(_slist_item_t[_group_size])];
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
		_m_child_count(0), _m_first(nullptr), _m_item_count(0), _m_last(nullptr), _m_level(level) {}
	_slist_parent_group(_group_t* child)noexcept:
		_m_child_count(1), _m_first(child->get_first()), _m_item_count(child->get_item_count()),
		_m_last(child->get_last()), _m_level(child->get_level()+1)
		{
		_m_children[0]=child;
		}
	_slist_parent_group(_parent_group_t const& group)noexcept:
		_m_child_count(group._m_child_count), _m_item_count(group._m_item_count), _m_level(group._m_level)
		{
		if(_m_level>1)
			{
			for(unsigned int u=0; u<_m_child_count; u++)
				_m_children[u]=new _parent_group_t((_parent_group_t const&)*group._m_children[u]);
			}
		else
			{
			for(unsigned int u=0; u<_m_child_count; u++)
				_m_children[u]=new _item_group_t((_item_group_t const&)*group._m_children[u]);
			}
		_m_first=_m_children[0]->get_first();
		_m_last=_m_children[_m_child_count-1]->get_last();
		}
	~_slist_parent_group()noexcept override
		{
		for(unsigned int u=0; u<_m_child_count; u++)
				delete _m_children[u];
		}

	// Access
	inline bool contains(_id_t const& id)const noexcept override { return get_item_pos(id)>=0; }
	inline int find(_id_t const& id)const noexcept override { return get_item_pos(id); }
	_slist_item_t* get(_id_t const& id)noexcept override
		{
		int ipos=get_item_pos(id);
		if(ipos<0)
			return nullptr;
		return _m_children[ipos]->get(id);
		}
	_slist_item_t const* get(_id_t const& id)const noexcept override
		{
		int ipos=get_item_pos(id);
		if(ipos<0)
			return nullptr;
		return _m_children[ipos]->get(id);
		}
	_slist_item_t* get_at(size_t position)noexcept override
		{
		unsigned int group=get_group(&position);
		if(group>=_group_size)
			return nullptr;
		return _m_children[group]->get_at(position);
		}
	_slist_item_t const* get_at(size_t position)const noexcept override
		{
		unsigned int group=get_group(&position);
		if(group>=_group_size)
			return nullptr;
		return _m_children[group]->get_at(position);
		}
	_group_t* get_child(unsigned int position)const noexcept
		{
		if(position>=_m_child_count)
			return nullptr;
		return _m_children[position];
		}
	inline unsigned int get_child_count()const noexcept override { return _m_child_count; }
	inline _slist_item_t* get_first()noexcept override { return _m_first; }
	inline _slist_item_t const* get_first()const noexcept override { return const_cast<_slist_item_t const*>(_m_first); }
	inline size_t get_item_count()const noexcept override { return _m_item_count; }
	inline _slist_item_t* get_last()noexcept override { return _m_last; }
	inline _slist_item_t const* get_last()const noexcept override { return const_cast<_slist_item_t const*>(_m_last); }
	inline unsigned int get_level()const noexcept override { return _m_level; }

	// Modification
	bool add(_id_t const& id, _item_t const* item, bool again, bool once, bool* exists)noexcept override
		{
		if(!add_internal(id, item, again, once, exists))
			return false;
		_m_item_count++;
		update_bounds();
		return true;
		}
	void append_groups(_group_t* const* groups, unsigned int count)noexcept
		{
		memcpy(&_m_children[_m_child_count], groups, count*sizeof(_group_t*));
		for(unsigned int u=0; u<count; u++)
			_m_item_count+=groups[u]->get_item_count();
		_m_child_count+=count;
		update_bounds();
		}
	bool combine(unsigned int position)noexcept
		{
		unsigned int countat=_m_children[position]->get_child_count();
		if(position>0)
			{
			if(countat+_m_children[position-1]->get_child_count()<=_group_size)
				{
				move_children(position, position-1, countat);
				remove_internal(position);
				return true;
				}
			}
		if(position+1<_m_child_count)
			{
			if(countat+_m_children[position+1]->get_child_count()<=_group_size)
				{
				move_children(position, position+1, countat);
				remove_internal(position);
				return true;
				}
			}
		return false;
		}
	void insert_groups(unsigned int position, _group_t* const* groups, unsigned int count)noexcept
		{
		for(unsigned int u=_m_child_count+count-1; u>=position+count; u--)
			_m_children[u]=_m_children[u-count];
		memcpy(&_m_children[position], groups, count*sizeof(_group_t*));
		for(unsigned int u=0; u<count; u++)
			_m_item_count+=groups[u]->get_item_count();
		_m_child_count+=count;
		update_bounds();
		}
	void move_children(unsigned int source, unsigned int destination, unsigned int count)noexcept
		{
		if(count==0)
			return;
		if(_m_level>1)
			{
			_parent_group_t* src=(_parent_group_t*)_m_children[source];
			_parent_group_t* dst=(_parent_group_t*)_m_children[destination];
			if(source>destination)
				{
				dst->append_groups(src->_m_children, count);
				src->remove_groups(0, count);
				}
			else
				{
				unsigned int srccount=src->get_child_count();
				dst->insert_groups(0, &src->_m_children[srccount-count], count);
				src->remove_groups(srccount-count, count);
				}
			}
		else
			{
			_item_group_t* src=(_item_group_t*)_m_children[source];
			_item_group_t* dst=(_item_group_t*)_m_children[destination];
			if(source>destination)
				{
				dst->append_items(src->get_items(), count);
				src->remove_items(0, count);
				}
			else
				{
				_slist_item_t const* srcitems=src->get_items();
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
		if(!_m_children[pos]->remove(id))
			return false;
		_m_item_count--;
		combine(pos);
		update_bounds();
		return true;
		}
	bool remove_at(size_t position)noexcept override
		{
		if(position>=_m_item_count)
			return false;
		unsigned int group=get_group(&position);
		_m_children[group]->remove_at(position);
		_m_item_count--;
		combine(group);
		update_bounds();
		return true;
		}
	void remove_groups(unsigned int position, unsigned int count)noexcept
		{
		for(unsigned int u=0; u<count; u++)
			_m_item_count-=_m_children[position+u]->get_item_count();
		for(unsigned int u=position; u+count<_m_child_count; u++)
			_m_children[u]=_m_children[u+count];
		_m_child_count-=count;
		update_bounds();
		}
	inline void set_child_count(unsigned int count)noexcept { _m_child_count=count; }
	bool split(unsigned int position)noexcept
		{
		if(_m_child_count==_group_size)
			return false;
		for(unsigned int u=_m_child_count; u>position+1; u--)
			_m_children[u]=_m_children[u-1];
		if(_m_level>1)
			{
			_m_children[position+1]=new _parent_group_t(_m_level-1);
			}
		else
			{
			_m_children[position+1]=new _item_group_t();
			}
		_m_child_count++;
		return true;
		}

private:
	// Common
	bool add_internal(_id_t const& id, _item_t const* item, bool again, bool once, bool* exists)noexcept
		{
		unsigned int group=0;
		unsigned int count=get_insert_pos(id, &group, exists);
		if(once&&*exists)
			return false;
		if(!again)
			{
			for(unsigned int u=0; u<count; u++)
				{
				if(_m_children[group+u]->add(id, item, false, once, exists))
					return true;
				if(once&&*exists)
					return false;
				}
			unsigned int empty=get_nearest(group);
			if(empty<_m_child_count)
				{
				if(count>1&&empty>group)
					group++;
				move_empty_slot(empty, group);
				if(_m_children[group]->add(id, item, false, once, exists))
					return true;
				}
			}
		if(!split(group))
			return false;
		move_children(group, group+1, 1);
		count=get_insert_pos(id, &group, exists);
		if(once&&*exists)
			return false;
		for(unsigned int u=0; u<count; u++)
			{
			if(_m_children[group+u]->add(id, item, false, once, exists))
				return true;
			if(once&&*exists)
				return false;
			}
		return false;
		}
	unsigned int get_group(size_t* position)const noexcept
		{
		for(unsigned int u=0; u<_m_child_count; u++)
			{
			size_t count=_m_children[u]->get_item_count();
			if(*position<count)
				return u;
			*position-=count;
			}
		return _group_size;
		}
	int get_item_pos(_id_t const& id)const noexcept
		{
		if(!_m_child_count)
			return -1;
		unsigned int start=0;
		unsigned int end=_m_child_count;
		unsigned int u=0;
		_slist_item_t* first=nullptr;
		_slist_item_t* last=nullptr;
		while(start<end)
			{
			u=start+(end-start)/2;
			first=_m_children[u]->get_first();
			if(first->id>id)
				{
				end=u;
				continue;
				}
			last=_m_children[u]->get_last();
			if(last->id<id)
				{
				start=u+1;
				continue;
				}
			return u;
			}
		if(u>0&&first->id>id)
			u--;
		return -(INT)u-1;
		}
	unsigned int get_insert_pos(_id_t const& id, unsigned int* group, bool* exists)const noexcept
		{
		if(!_m_child_count)
			return 0;
		unsigned int start=0;
		unsigned int end=_m_child_count;
		_slist_item_t* first=nullptr;
		_slist_item_t* last=nullptr;
		while(start<end)
			{
			unsigned int u=start+(end-start)/2;
			first=_m_children[u]->get_first();
			last=_m_children[u]->get_last();
			if(first->id>=id)
				{
				if(first->id==id)
					*exists=true;
				end=u;
				continue;
				}
			if(last->id<=id)
				{
				if(last->id==id)
					*exists=true;
				start=u+1;
				continue;
				}
			start=u;
			break;
			}
		if(start>_m_child_count-1)
			start=_m_child_count-1;
		*group=start;
		if(start>0)
			{
			first=_m_children[start]->get_first();
			if(first->id>=id)
				{
				*group=start-1;
				return 2;
				}
			}
		if(start+1<_m_child_count)
			{
			last=_m_children[start]->get_last();
			if(last->id<=id)
				return 2;
			}
		return 1;
		}
	unsigned int get_nearest(unsigned int position)const noexcept
		{
		int before=position-1;
		unsigned int after=position+1;
		while(before>=0||after<_m_child_count)
			{
			if(before>=0)
				{
				if(_m_children[before]->get_child_count()<_group_size)
					return before;
				before--;
				}
			if(after<_m_child_count)
				{
				if(_m_children[after]->get_child_count()<_group_size)
					return after;
				after++;
				}
			}
		return _group_size;
		}
	void remove_internal(unsigned int position)noexcept
		{
		delete _m_children[position];
		for(unsigned int u=position; u+1<_m_child_count; u++)
			_m_children[u]=_m_children[u+1];
		_m_child_count--;
		}
	void update_bounds()noexcept
		{
		if(!_m_child_count)
			return;
		_m_first=_m_children[0]->get_first();
		_m_last=_m_children[_m_child_count-1]->get_last();
		}
	unsigned int _m_child_count;
	_group_t* _m_children[_group_size];
	_slist_item_t* _m_first;
	size_t _m_item_count;
	_slist_item_t* _m_last;
	unsigned int _m_level;
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
	inline bool contains(_id_t const& id)const noexcept { return _m_root->contains(id); }
	inline size_t get_count()const noexcept { return _m_root->get_item_count(); }

	// Modification
	void clear()noexcept
		{
		delete _m_root;
		_m_root=new _item_group_t();
		}
	bool remove(_id_t const& id)noexcept
		{
		if(_m_root->remove(id))
			{
			update_root();
			return true;
			}
		return false;
		}
	void remove_at(size_t position)noexcept
		{
		if(_m_root->remove_at(position))
			update_root();
		}

protected:
	// Con-/Destructors
	_slist_cluster(): _m_root(new _item_group_t()) {}
	_slist_cluster(_slist_cluster const& slist)
		{
		if(slist._m_root->get_level()>0)
			{
			_m_root=new _parent_group_t((_parent_group_t const&)*slist._m_root);
			}
		else
			{
			_m_root=new _item_group_t((_item_group_t const&)*slist._m_root);
			}
		}
	~_slist_cluster()noexcept { delete _m_root; }

	// Common
	void update_root()noexcept
		{
		if(_m_root->get_level()==0)
			return;
		if(_m_root->get_child_count()>1)
			return;
		_parent_group_t* root=(_parent_group_t*)_m_root;
		_m_root=root->get_child(0);
		root->set_child_count(0);
		delete root;
		}
	_group_t* _m_root;
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
	size_t get_position()const noexcept
		{
		size_t pos=0;
		for(unsigned int u=0; u<_m_level_count-1; u++)
			{
			_parent_group_t* group=(_parent_group_t*)_m_its[u].group;
			unsigned int grouppos=_m_its[u].position;
			for(unsigned int v=0; v<grouppos; v++)
				pos+=group->get_child(v)->get_item_count();
			}
		pos+=_m_its[_m_level_count-1].position;
		return pos;
		}
	inline bool has_current()const noexcept { return _m_current!=nullptr; }

	// Assignment
	_base_t& operator=(_base_t const& it)
		{
		_m_current=it._m_current;
		_m_slist=it._m_slist;
		set_level_count(it._m_level_count);
		memcpy(_m_its, it._m_its, _m_level_count*sizeof(_it_struct));
		return *this;
		}

	// Modification
	bool find(_id_t const& id)
		{
		_m_current=nullptr;
		bool bfound=true;
		_group_t* group=_m_slist->_m_root;
		unsigned int levelcount=group->get_level()+1;
		set_level_count(levelcount);
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
			_m_its[u].group=group;
			_m_its[u].position=pos;
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
		_m_its[levelcount-1].group=group;
		_m_its[levelcount-1].position=pos;
		_m_current=itemgroup->get_at(pos);
		return bfound;
		}
	bool move_next()noexcept
		{
		if(_m_current==nullptr)
			return false;
		_it_struct* it=&_m_its[_m_level_count-1];
		_item_group_t* itemgroup=(_item_group_t*)it->group;
		unsigned int count=itemgroup->get_child_count();
		if(it->position+1<count)
			{
			it->position++;
			_m_current=itemgroup->get_at(it->position);
			return true;
			}
		for(unsigned int u=_m_level_count-1; u>0; u--)
			{
			it=&_m_its[u-1];
			_parent_group_t* parentgroup=(_parent_group_t*)it->group;
			count=parentgroup->get_child_count();
			if(it->position+1>=count)
				continue;
			it->position++;
			_group_t* group=it->group;
			for(; u<_m_level_count; u++)
				{
				parentgroup=(_parent_group_t*)group;
				group=parentgroup->get_child(it->position);
				it=&_m_its[u];
				it->group=group;
				it->position=0;
				}
			itemgroup=(_item_group_t*)group;
			_m_current=itemgroup->get_at(0);
			return true;
			}
		_m_current=nullptr;
		return false;
		}
	bool move_previous()noexcept
		{
		if(_m_current==nullptr)
			return false;
		_it_struct* it=&_m_its[_m_level_count-1];
		_item_group_t* itemgroup=(_item_group_t*)it->group;
		if(it->position>0)
			{
			it->position--;
			_m_current=itemgroup->get_at(it->position);
			return true;
			}
		for(unsigned int u=_m_level_count-1; u>0; u--)
			{
			it=&_m_its[u-1];
			_parent_group_t* parentgroup=(_parent_group_t*)it->group;
			if(it->position==0)
				continue;
			it->position--;
			_group_t* group=it->group;
			unsigned int pos=0;
			for(; u<_m_level_count; u++)
				{
				parentgroup=(_parent_group_t*)group;
				group=parentgroup->get_child(it->position);
				pos=group->get_child_count()-1;
				it=&_m_its[u];
				it->group=group;
				it->position=pos;
				}
			itemgroup=(_item_group_t*)group;
			_m_current=itemgroup->get_at(pos);
			return true;
			}
		_m_current=nullptr;
		return false;
		}
	void set_position(size_t position)
		{
		_m_current=nullptr;
		_group_t* group=_m_slist->_m_root;
		unsigned int levelcount=group->get_level()+1;
		set_level_count(levelcount);
		unsigned int pos=get_position_internal(group, &position);
		_m_its[0].group=group;
		_m_its[0].position=pos;
		for(unsigned int u=0; u<_m_level_count-1; u++)
			{
			_parent_group_t* parentgroup=(_parent_group_t*)_m_its[u].group;
			group=parentgroup->get_child(pos);
			pos=get_position_internal(group, &position);
			_m_its[u+1].group=group;
			_m_its[u+1].position=pos;
			}
		if(pos<group->get_child_count())
			{
			_item_group_t* itemgroup=(_item_group_t*)group;
			_m_current=itemgroup->get_at(pos);
			}
		}

protected:
	// Con-/Destructors
	_slist_iterator_base(_base_t const& it):
		_m_current(it._m_current), _m_slist(it._m_slist), _m_its(nullptr), _m_level_count(it._m_level_count)
		{
		_m_its=(_it_struct*)operator new(_m_level_count*sizeof(_it_struct));
		if(_m_its==nullptr)
			throw std::exception();
		memcpy(_m_its, it._m_its, _m_level_count*sizeof(_it_struct));
		}
	_slist_iterator_base(_slist_ptr_t slist)noexcept:
		_m_slist(slist), _m_its(nullptr), _m_level_count(0) {}
	~_slist_iterator_base()noexcept { if(_m_its!=nullptr)operator delete(_m_its); }

	// Helper-struct
	typedef struct
		{
		_group_t* group;
		unsigned int position;
		}_it_struct;

	// Common
	unsigned int get_position_internal(_group_t* group, size_t* pos)const noexcept
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
		size_t itemcount=0;
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
	void set_level_count(unsigned int levelcount)
		{
		if(_m_level_count==levelcount)
			return;
		if(_m_its!=nullptr)
			operator delete(_m_its);
		_m_its=(_it_struct*)operator new(levelcount*sizeof(_it_struct));
		if(_m_its==nullptr)
			throw std::exception();
		_m_level_count=levelcount;
		}
	_slist_item_t* _m_current;
	_slist_ptr_t _m_slist;
	_it_struct* _m_its;
	unsigned int _m_level_count;
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
	_slist_iterator(_it_t const& it): _base_t(it) {}
	_slist_iterator(_slist_t* slist, size_t position): _base_t(slist) { this->set_position(position); }
	_slist_iterator(_slist_t* slist, size_t, _id_t const& id): _base_t(slist) { this->find(id); }

	// Access
	_id_t const& get_current_id()const
		{
		if(this->_m_current==nullptr)
			throw std::exception();
		return this->_m_current->id;
		}
	_item_t& get_current_item()const
		{
		if(this->_m_current==nullptr)
			throw std::exception();
		return this->_m_current->item;
		}

	// Modification
	void remove_current()noexcept
		{
		if(this->_m_current==nullptr)
			return;
		size_t pos=this->get_position();
		this->_m_slist->remove_at(pos);
		this->set_position(pos);
		}
	void set_current_item(_item_t const& item)noexcept
		{
		if(this->_m_current==nullptr)
			return;
		this->_m_current->item=item;
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
	_slist_iterator(_it_t const& it): _base_t(it) {}
	_slist_iterator(_slist_t* slist, size_t position): _base_t(slist) { this->set_position(position); }
	_slist_iterator(_slist_t* slist, size_t, _id_t const& id): _base_t(slist) { this->find(id); }

	// Access
	inline _id_t const& get_current()const
		{
		if(this->_m_current==nullptr)
			throw std::exception();
		return this->_m_current->id;
		}

	// Modification
	void remove_current()
		{
		if(this->_m_current==nullptr)
			throw std::exception();
		size_t pos=this->get_position();
		this->_m_slist->remove_at(pos);
		this->set_position(pos);
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
	_slist_const_iterator(_slist_t const* slist, size_t position)noexcept: _base_t(slist) { this->set_position(position); }
	_slist_const_iterator(_slist_t const* slist, size_t, _id_t const& id)noexcept: _base_t(slist) { this->find(id); }

	// Access
	inline _id_t const& get_current_id()const
		{
		if(this->_m_current==nullptr)
			throw std::exception();
		return this->_m_current->id;
		}
	inline _item_t& get_current_item()const
		{
		if(this->_m_current==nullptr)
			throw std::exception();
		return this->_m_current->item;
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
	_slist_const_iterator(_it_t const& it): _base_t(it) {}
	_slist_const_iterator(_slist_t const* slist, size_t position): _base_t(slist) { this->set_position(position); }
	_slist_const_iterator(_slist_t const* slist, size_t, _id_t const& id): _base_t(slist) { this->find(id); }

	// Access
	inline _id_t const& get_current()const noexcept
		{
		if(this->_m_current==nullptr)
			throw std::exception();
		return this->_m_current->id;
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
	inline _it_t at(size_t position) { return _it_t(this, position); }
	inline _const_it_t at(size_t position)const { return _const_it_t(this, position); }
	inline _it_t at(_it_t const& it) { return _it_t(it); }
	inline _const_it_t at(_const_it_t const& it)const { return _const_it_t(it); }
	inline _it_t find(_id_t const& id) { return _it_t(this, 0, id); }
	inline _const_it_t find(_id_t const& id)const { return _const_it_t(this, 0, id); }
	inline _it_t first() { return _it_t(this, 0); }
	inline _const_it_t first()const { return _const_it_t(this, 0); }
	inline _it_t last() { return _it_t(this, this->get_count()-1); }
	inline _const_it_t last()const { return _const_it_t(this, this->get_count()-1); }

protected:
	// Con-/Destructors
	_slist_base() {}
	_slist_base(_slist_base const& slist): _base_t(slist) {}

	// Modification
	bool add_internal(_id_t const& id, _item_t const* item, bool once=true)
		{
		bool exists=false;
		if(this->_m_root->add(id, item, false, once, &exists))
			return true;
		if(once&&exists)
			return false;
		this->_m_root=new _parent_group_t(this->_m_root);
		return this->_m_root->add(id, item, true, once, &exists);
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
	_slist_typed() {}
	_slist_typed(_base_t const& base): _base_t(base) {}

	// Access
	inline _item_t& operator[](_id_t const& id) { return get(id); }
	inline _item_t const& operator[](_id_t const& id)const { return get(id); }
	_item_t& get(_id_t const& id)
		{
		_slist_item_t* item=this->_m_root->get(id);
		if(item==nullptr)
			throw std::exception();
		return item->item;
		}
	_item_t const& get(_id_t const& id)const
		{
		_slist_item_t* item=this->_m_root->get(id);
		if(item==nullptr)
			throw std::exception();
		return item->item;
		}
	_item_t* try_get(_id_t const& id)noexcept
		{
		_slist_item_t* item=this->_m_root->get(id);
		if(item==nullptr)
			return nullptr;
		return &item->item;
		}
	_item_t const* try_get(_id_t const& id)const noexcept
		{
		_slist_item_t* item=this->_m_root->get(id);
		if(item==nullptr)
			return nullptr;
		return &item->item;
		}

	// Modification
	inline bool add(_id_t const& id, _item_t const& item, bool once=true) { return this->add_internal(id, &item, once); }
	void set(_id_t const& id, _item_t const& item)
		{
		_slist_item_t* sli=this->_m_root->get(id);
		if(sli!=nullptr)
			{
			sli->item=item;
			return;
			}
		add(id, item);
		}
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
	_slist_typed() {}
	_slist_typed(_base_t const& base): _base_t(base) {}

	// Access
	inline _id_t const& operator[](size_t position)const { return get_at(position); }
	_id_t const& get(_id_t const& id)const
		{
		_slist_item_t* item=this->_m_root->get(id);
		if(item==nullptr)
			throw std::exception();
		return item->id;
		}
	_id_t const& get_at(size_t position)const
		{
		_slist_item_t* item=this->_m_root->get_at(position);
		if(item==nullptr)
			throw std::exception();
		return item->id;
		}
	_id_t const* try_get(_id_t const& id)const noexcept
		{
		_slist_item_t* item=this->_m_root->get(id);
		if(item==nullptr)
			return nullptr;
		return &item->id;
		}

	// Modification
	inline bool add(_id_t const& id, bool once=true) { return this->add_internal(id, nullptr, once); }
};


//=======
// SList
//=======

template <typename _id_t, typename _item_t=void, unsigned int _group_size=100>
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
	slist() {}
	slist(slist const& slist): _base_t(slist) {}
};

} // namespace

#endif // _CLUSTERS_SLIST
