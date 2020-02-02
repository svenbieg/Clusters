//========
// list.h
//========

// Implementation of an ordererd list.
// Items can be inserted and removed in real-time.

// Copyright 2019, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _CLUSTERS_LIST
#define _CLUSTERS_LIST


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

template <typename _item_t, unsigned int _group_size> class list;


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
	virtual unsigned int get_child_count()const noexcept=0;
	virtual std::size_t get_item_count()const noexcept=0;
	virtual unsigned int get_level()const noexcept=0;

	// Modification
	virtual bool append(_item_t const& item, bool again)noexcept=0;
	virtual std::size_t append(_item_t const* append, std::size_t count)noexcept=0;
	virtual bool insert_at(std::size_t position, _item_t const& item, bool again)noexcept=0;
	virtual bool remove_at(std::size_t position)noexcept=0;
};


//============
// Item-group
//============

template <typename _item_t, unsigned int _group_size>
class _list_item_group: public _list_group<_item_t>
{
public:
	// Con-/Destructors
	_list_item_group()noexcept: _m_item_count(0) {}
	_list_item_group(_list_item_group const& group)noexcept: _m_item_count(group._m_item_count)
		{
		_item_t* dst=get_items();
		_item_t const* src=group.get_items();
		for(unsigned int u=0; u<_m_item_count; u++)
			new (&dst[u]) _item_t(src[u]);
		}
	~_list_item_group()noexcept override
		{
		_item_t* items=get_items();
		for(unsigned int u=0; u<_m_item_count; u++)
			items[u].~_item_t();
		}

	// Access
	_item_t* get_at(std::size_t position)noexcept override
		{
		if(position>=_m_item_count)
			return nullptr;
		return &get_items()[position];
		}
	_item_t const* get_at(std::size_t position)const noexcept override
		{
		if(position>=_m_item_count)
			return nullptr;
		return &get_items()[position];
		}
	inline unsigned int get_child_count()const noexcept override { return _m_item_count; }
	inline std::size_t get_item_count()const noexcept override { return _m_item_count; }
	inline _item_t* get_items()noexcept { return (_item_t*)_m_items; }
	inline _item_t const* get_items()const noexcept { return (_item_t const*)_m_items; }
	inline unsigned int get_level()const noexcept override { return 0; }

	// Modification
	bool append(_item_t const& item, bool)noexcept override
		{
		if(_m_item_count==_group_size)
			return false;
		_item_t* items=get_items();
		new (&items[_m_item_count]) _item_t(item);
		_m_item_count++;
		return true;
		}
	std::size_t append(_item_t const* append, std::size_t count)noexcept override
		{
		unsigned int copy=_group_size-_m_item_count;
		if(copy==0)
			return 0;
		if(count<copy)
			copy=(unsigned int)count;
		_item_t* items=get_items();
		for(unsigned int u=0; u<copy; u++)
			new (&items[_m_item_count+u]) _item_t(append[u]);
		_m_item_count+=copy;
		return copy;
		}
	void append_items(_item_t* append, unsigned int count)noexcept
		{
		_item_t* items=get_items();
		for(unsigned int u=0; u<count; u++)
			new (&items[_m_item_count+u]) _item_t(std::move(append[u]));
		_m_item_count+=count;
		}
	bool insert_at(std::size_t position, _item_t const& item, bool)noexcept override
		{
		if(position>_m_item_count)
			return false;
		if(_m_item_count==_group_size)
			return false;
		_item_t* items=get_items();
		for(unsigned int u=_m_item_count; u>position; u--)
			new (&items[u]) _item_t(std::move(items[u-1]));
		new (&items[position]) _item_t(item);
		_m_item_count++;
		return true;
		}
	void insert_items(unsigned int position, _item_t* insert, unsigned int count)noexcept
		{
		_item_t* items=get_items();
		for(unsigned int u=_m_item_count+count-1; u>=position+count; u--)
			new (&items[u]) _item_t(std::move(items[u-count]));
		for(unsigned int u=0; u<count; u++)
			new (&items[position+u]) _item_t(std::move(insert[u]));
		_m_item_count+=count;
		}
	bool remove_at(std::size_t position)noexcept override
		{
		if(position>=_m_item_count)
			return false;
		_item_t* items=get_items();
		items[position].~_item_t();
		for(unsigned int u=(unsigned int)position; u+1<_m_item_count; u++)
			new (&items[u]) _item_t(std::move(items[u+1]));
		_m_item_count--;
		return true;
		}
	void remove_items(unsigned int position, unsigned int count)noexcept
		{
		_item_t* items=get_items();
		for(unsigned int u=position; u+count<_m_item_count; u++)
			new (&items[u]) _item_t(std::move(items[u+count]));
		_m_item_count-=count;
		}

private:
	// Uninitialized array of items
	unsigned int _m_item_count;
	alignas(alignof(_item_t[_group_size])) unsigned char _m_items[sizeof(_item_t[_group_size])];
};


//==============
// Parent-group
//==============

template <typename _item_t, unsigned int _group_size>
class _list_parent_group: public _list_group<_item_t>
{
private:
	// Using
	using _group_t=_list_group<_item_t>;
	using _item_group_t=_list_item_group<_item_t, _group_size>;
	using _parent_group_t=_list_parent_group<_item_t, _group_size>;

public:
	// Con-Destructors
	_list_parent_group(unsigned int level)noexcept: _m_child_count(0), _m_item_count(0), _m_level(level) {}
	_list_parent_group(_group_t* child)noexcept:
		_m_child_count(1), _m_item_count(child->get_item_count()), _m_level(child->get_level()+1)
		{
		_m_children[0]=child;
		}
	_list_parent_group(_parent_group_t const& group)noexcept:
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
		}
	~_list_parent_group()noexcept override
		{
		for(unsigned int u=0; u<_m_child_count; u++)
				delete _m_children[u];
		}

	// Access
	_item_t* get_at(std::size_t position)noexcept override
		{
		unsigned int group=get_group(&position);
		if(group>=_group_size)
			return nullptr;
		return _m_children[group]->get_at(position);
		}
	_item_t const* get_at(std::size_t position)const noexcept override
		{
		unsigned int group=get_group(&position);
		if(group>=_group_size)
			return nullptr;
		return _m_children[group]->get_at(position);
		}
	_group_t* get_child(unsigned int position)noexcept
		{
		if(position>=_m_child_count)
			return nullptr;
		return _m_children[position];
		}
	inline unsigned int get_child_count()const noexcept override { return _m_child_count; }
	inline std::size_t get_item_count()const noexcept override { return _m_item_count; }
	inline unsigned int get_level()const noexcept override { return _m_level; }

	// Modification
	bool append(_item_t const& item, bool again)noexcept override
		{
		unsigned int group=_m_child_count-1;
		if(!again)
			{
			if(_m_children[group]->append(item, false))
				{
				_m_item_count++;
				return true;
				}
			unsigned int empty=get_nearest(group);
			if(empty<_m_child_count)
				{
				move_empty_slot(empty, group);
				_m_children[group]->append(item, false);
				_m_item_count++;
				return true;
				}
			}
		if(!split_child(group))
			return false;
		if(!_m_children[group+1]->append(item, true))
			return false;
		_m_item_count++;
		return true;
		}
	std::size_t append(_item_t const* append, std::size_t count)noexcept override
		{
		std::size_t pos=0;
		if(_m_child_count>0)
			{
			pos+=_m_children[_m_child_count-1]->append(append, count);
			if(pos>0)
				{
				_m_item_count+=pos;
				count-=pos;
				if(count==0)
					return pos;
				}
			}
		unsigned int last=minimize_internal();
		for(; last<_m_child_count; last++)
			{
			std::size_t written=_m_children[last]->append(&append[pos], count);
			if(written>0)
				{
				_m_item_count+=written;
				pos+=written;
				count-=written;
				if(count==0)
					break;
				}
			}
		if(count==0)
			{
			free_children();
			return pos;
			}
		while(count>0)
			{
			if(_m_child_count==_group_size)
				break;
			if(_m_level==1)
				{
				_m_children[_m_child_count]=new _item_group_t();
				}
			else
				{
				_m_children[_m_child_count]=new _parent_group_t(_m_level-1);
				}
			_m_child_count++;
			std::size_t written=_m_children[_m_child_count-1]->append(&append[pos], count);
			_m_item_count+=written;
			pos+=written;
			count-=written;
			}
		return pos;
		}
	void append_groups(_group_t* const* groups, unsigned int count)noexcept
		{
		memcpy(&_m_children[_m_child_count], groups, count*sizeof(_group_t*));
		for(unsigned int u=0; u<count; u++)
			_m_item_count+=groups[u]->get_item_count();
		_m_child_count+=count;
		}
	bool insert_at(std::size_t position, _item_t const& item, bool again)noexcept
		{
		if(position>_m_item_count)
			return false;
		std::size_t pos=position;
		unsigned int group=0;
		unsigned int inscount=get_insert_pos(&pos, &group);
		if(!inscount)
			return false;
		if(!again)
			{
			std::size_t at=pos;
			for(unsigned int u=0; u<inscount; u++)
				{
				if(_m_children[group+u]->insert_at(at, item, false))
					{
					_m_item_count++;
					return true;
					}
				at=0;
				}
			unsigned int empty=get_nearest(group);
			if(empty<_m_child_count)
				{
				if(inscount>1&&empty>group)
					group++;
				move_empty_slot(empty, group);
				pos=position;
				inscount=get_insert_pos(&pos, &group);
				std::size_t at=pos;
				for(unsigned int u=0; u<inscount; u++)
					{
					if(_m_children[group+u]->insert_at(at, item, false))
						{
						_m_item_count++;
						return true;
						}
					at=0;
					}
				}
			}
		if(!split_child(group))
			return false;
		std::size_t count=_m_children[group]->get_item_count();
		if(pos>=count)
			{
			group++;
			pos-=count;
			}
		_m_children[group]->insert_at(pos, item, true);
		_m_item_count++;
		return true;
		}
	void insert_groups(unsigned int position, _group_t* const* groups, unsigned int count)noexcept
		{
		for(unsigned int u=_m_child_count+count-1; u>=position+count; u--)
			_m_children[u]=_m_children[u-count];
		memcpy(&_m_children[position], groups, count*sizeof(_group_t*));
		for(unsigned int u=0; u<count; u++)
			_m_item_count+=groups[u]->get_item_count();
		_m_child_count+=count;
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
				_item_t* srcitems=src->get_items();
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
	bool remove_at(std::size_t position)noexcept override
		{
		if(position>=_m_item_count)
			return false;
		unsigned int group=get_group(&position);
		_m_children[group]->remove_at(position);
		_m_item_count--;
		combine_children(group);
		return true;
		}
	void remove_groups(unsigned int position, unsigned int count)noexcept
		{
		for(unsigned int u=0; u<count; u++)
			_m_item_count-=_m_children[position+u]->get_item_count();
		for(unsigned int u=position; u+count<_m_child_count; u++)
			_m_children[u]=_m_children[u+count];
		_m_child_count-=count;
		}
	inline void set_child_count(unsigned int count)noexcept { _m_child_count=count; }

private:
	// Access
	unsigned int get_group(std::size_t* position)const noexcept
		{
		for(unsigned int u=0; u<_m_child_count; u++)
			{
			std::size_t count=_m_children[u]->get_item_count();
			if(*position<count)
				return u;
			*position-=count;
			}
		return _group_size;
		}
	unsigned int get_insert_pos(std::size_t* position, unsigned int* group)const noexcept
		{
		std::size_t pos=*position;
		for(unsigned int u=0; u<_m_child_count; u++)
			{
			std::size_t count=_m_children[u]->get_item_count();
			if(pos<=count)
				{
				*group=u;
				*position=pos;
				if(pos==count&&u+1<_m_child_count)
					return 2;
				return 1;
				}
			pos-=count;
			}
		return 0;
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

	// Modification
	bool combine_children(unsigned int position)noexcept
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
	void free_children()noexcept
		{
		for(unsigned int u=_m_child_count; u>0; u--)
			{
			if(_m_children[u-1]->get_child_count()>0)
				break;
			delete _m_children[u-1];
			_m_child_count--;
			}
		}
	unsigned int minimize_internal()noexcept
		{
		unsigned int dst=0;
		unsigned int src=1;
		for(; dst<_m_child_count; dst++)
			{
			unsigned int free=_group_size-_m_children[dst]->get_child_count();
			if(free==0)
				continue;
			if(src<=dst)
				src=dst+1;
			for(; src<_m_child_count; src++)
				{
				unsigned int count=_m_children[src]->get_child_count();
				if(count==0)
					continue;
				unsigned int move=count<free? count: free;
				move_children(src, dst, move);
				free-=move;
				if(free==0)
					break;
				}
			if(src>=_m_child_count)
				break;
			}
		return dst;
		}
	void remove_internal(unsigned int position)noexcept
		{
		delete _m_children[position];
		for(unsigned int u=position; u+1<_m_child_count; u++)
			_m_children[u]=_m_children[u+1];
		_m_child_count--;
		}
	bool split_child(unsigned int position)noexcept
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
		move_children(position, position+1, 1);
		return true;
		}

	// Common
	unsigned int _m_child_count;
	_group_t* _m_children[_group_size];
	std::size_t _m_item_count;
	unsigned int _m_level;
};


//=========
// Cluster
//=========

// Forward-Declaration
template <typename _item_t, unsigned int _group_size, bool _is_const> class _list_iterator_base;

template <typename _item_t, unsigned int _group_size>
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
	inline _item_t operator[](std::size_t position)const noexcept { return get_at(position); }
	_item_t get_at(std::size_t position)const noexcept
		{
		_item_t const* item=_m_root->get_at(position);
		if(item==nullptr)
			return _item_t();
		return *item;
		}
	inline std::size_t get_count()const noexcept { return _m_root->get_item_count(); }

	// Modification
	void append(_item_t const& item)noexcept
		{
		if(_m_root->append(item, false))
			return;
		_m_root=new _parent_group_t(_m_root);
		_m_root->append(item, true);
		}
	void append(_item_t const* items, std::size_t count)noexcept
		{
		std::size_t pos=0;
		while(count>0)
			{
			std::size_t written=_m_root->append(&items[pos], count);
			count-=written;
			if(count>0)
				{
				_m_root=new _parent_group_t(_m_root);
				pos+=written;
				}
			}
		}
	void clear()noexcept
		{
		delete _m_root;
		_m_root=new _item_group_t();
		}
	inline void insert_at(std::size_t position, _item_t const& item)noexcept
		{
		if(_m_root->insert_at(position, item, false))
			return;
		_m_root=new _parent_group_t(_m_root);
		_m_root->insert_at(position, item, true);
		}
	void remove_at(std::size_t position)noexcept
		{
		if(_m_root->remove_at(position))
			update_root();
		}

protected:
	// Con-/Destructors
	_list_cluster()noexcept: _m_root(new _item_group_t()) {}
	_list_cluster(_list_cluster const& list)noexcept
		{
		if(list._m_root->get_level()>0)
			{
			_m_root=new _parent_group_t((_parent_group_t const&)*list._m_root);
			}
		else
			{
			_m_root=new _item_group_t((_item_group_t const&)*list._m_root);
			}
		}
	~_list_cluster()noexcept { delete _m_root; }

private:
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

template <typename _item_t, unsigned int _group_size, bool _is_const>
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
		if(_m_current==nullptr)
			return _item_t();
		return *_m_current;
		}
	std::size_t get_position()const noexcept
		{
		if(_m_level_count==0)
			return 0;
		std::size_t pos=0;
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

	// Modification
	_base_t& operator=(_base_t const& it)noexcept
		{
		_m_current=it._m_current;
		_m_list=it._m_list;
		if(set_level_count(it._m_level_count))
			memcpy(_m_its, it._m_its, _m_level_count*sizeof(_it_struct));
		return *this;
		}
	bool move_next()noexcept
		{
		if(_m_its==nullptr)
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
		if(_m_its==nullptr)
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
	bool set_position(std::size_t position)noexcept
		{
		_m_current=nullptr;
		_group_t* group=_m_list->_m_root;
		unsigned int levelcount=group->get_level()+1;
		if(!set_level_count(levelcount))
			return false;
		unsigned int pos=get_position_internal(group, &position);
		if(pos==_group_size)
			return false;
		_m_its[0].group=group;
		_m_its[0].position=pos;
		for(unsigned int u=0; u<_m_level_count-1; u++)
			{
			_parent_group_t* parentgroup=(_parent_group_t*)_m_its[u].group;
			group=parentgroup->get_child(pos);
			pos=get_position_internal(group, &position);
			if(pos==_group_size)
				return false;
			_m_its[u+1].group=group;
			_m_its[u+1].position=pos;
			}
		if(pos<group->get_child_count())
			{
			_item_group_t* itemgroup=(_item_group_t*)group;
			_m_current=itemgroup->get_at(pos);
			return true;
			}
		return false;
		}

protected:
	// Con-/Destructors
	_list_iterator_base(_base_t const& it)noexcept:
		_m_current(nullptr), _m_its(nullptr), _m_level_count(0), _m_list(it._m_list)
		{
		if(set_level_count(it._m_level_count))
			{
			memcpy(_m_its, it._m_its, _m_level_count*sizeof(_it_struct));
			_m_current=it._m_current;
			}
		}
	_list_iterator_base(_list_ptr_t list)noexcept:
		_m_current(nullptr), _m_its(nullptr), _m_level_count(0), _m_list(list) {}
	_list_iterator_base(_list_ptr_t list, std::size_t position)noexcept:
		_list_iterator_base(list) { set_position(position); }
	~_list_iterator_base()noexcept
		{
		if(_m_its!=nullptr)
			operator delete(_m_its);
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
		if(_m_level_count==levelcount)
			return true;
		if(_m_its!=nullptr)
			operator delete(_m_its);
		_m_its=(_it_struct*)operator new(levelcount*sizeof(_it_struct));
		_m_level_count=_m_its? levelcount: 0;
		return _m_level_count==levelcount;
		}
	_item_t* _m_current;
	_it_struct* _m_its;
	unsigned int _m_level_count;
	_list_ptr_t _m_list;
};


//==========
// Iterator
//==========

template <typename _item_t, unsigned int _group_size>
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
		if(this->_m_current==nullptr)
			return false;
		std::size_t pos=this->get_position();
		this->_m_list->remove_at(pos);
		this->set_position(pos);
		return true;
		}
	void set_current(_item_t const& item)noexcept
		{
		if(this->_m_current==nullptr)
			return;
		*(this->_m_current)=item;
		}
};


//================
// Const-iterator
//================

template <typename _item_t, unsigned int _group_size>
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

template <typename _item_t, unsigned int _group_size=10>
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

#endif // _CLUSTERS_LIST
