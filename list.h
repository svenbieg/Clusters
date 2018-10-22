//========
// list.h
//========

// Implementation of an ordererd list.
// Items can be inserted and removed in real-time.

// Copyright 2018, Sven Bieg (svenbieg@web.de)
// http://svenbieg.azurewebsites.net


#ifndef _CLUSTERS_LIST
#define _CLUSTERS_LIST


//=======
// Using
//=======

#include <exception>


//===========
// Namespace
//===========

namespace clusters {


//======================
// Forward-Declarations
//======================

template <typename _Item, unsigned int _Groupsize> class list;


//=======
// Group
//=======

template <typename _Item>
class _list_group
{
public:
	// Con-Destructors
	virtual ~_list_group() {}

	// Access
	virtual _Item& get_at(size_t position)=0;
	virtual _Item const& get_at(size_t position)const=0;
	virtual unsigned int get_child_count()const noexcept=0;
	virtual size_t get_item_count()const noexcept=0;
	virtual unsigned int get_level()const noexcept=0;

	// Modification
	virtual bool append(_Item const& item, bool again)=0;
	virtual bool insert_at(size_t position, _Item const& item, bool again)=0;
	virtual void remove_at(size_t position)=0;
};


//============
// Item-group
//============

template <typename _Item, unsigned int _Groupsize>
class _list_item_group: public _list_group<_Item>
{
public:
	// Con-/Destructors
	_list_item_group()noexcept: _m_item_count(0) {}
	_list_item_group(_list_item_group const& group): _m_item_count(group._m_item_count)
		{
		_Item* dst=get_items();
		_Item const* src=group.get_items();
		for(unsigned int u=0; u<_m_item_count; u++)
			new (&dst[u]) _Item(src[u]);
		}
	~_list_item_group()
		{
		_Item* items=get_items();
		for(unsigned int u=0; u<_m_item_count; u++)
			items[u].~_Item();
		}

	// Access
	inline _Item& get_at(size_t position)override
		{
		if(position>=_m_item_count)
			throw std::invalid_argument("");
		return get_items()[position];
		}
	inline _Item const& get_at(size_t position)const override
		{
		if(position>=_m_item_count)
			throw std::invalid_argument("");
		return get_items()[position];
		}
	inline unsigned int get_child_count()const noexcept override { return _m_item_count; }
	inline size_t get_item_count()const noexcept override { return _m_item_count; }
	inline _Item* get_items()noexcept { return (_Item*)_m_items; }
	inline _Item const* get_items()const noexcept { return (_Item const*)_m_items; }
	inline unsigned int get_level()const noexcept override { return 0; }

	// Modification
	bool append(_Item const& item, bool)override
		{
		if(_m_item_count==_Groupsize)
			return false;
		_Item* items=get_items();
		new (&items[_m_item_count]) _Item(item);
		_m_item_count++;
		return true;
		}
	void append_items(unsigned int count, _Item const* append)
		{
		_Item* items=get_items();
		for(unsigned int u=0; u<count; u++)
			new (&items[_m_item_count+u]) _Item(append[u]);
		_m_item_count+=count;
		}
	bool insert_at(size_t position, _Item const& item, bool)override
		{
		if(position>_m_item_count)
			throw std::invalid_argument("");
		if(position==_m_item_count)
			return append(item, false);
		if(_m_item_count==_Groupsize)
			return false;
		_Item* items=get_items();
		new (&items[_m_item_count]) _Item(items[_m_item_count-1]);
		for(unsigned int u=_m_item_count-1; u>position; u--)
			items[u]=items[u-1];
		items[position]=item;
		_m_item_count++;
		return true;
		}
	void insert_items(unsigned int position, unsigned int count, _Item const* insert)
		{
		if(position==_m_item_count)
			{
			append_items(count, insert);
			return;
			}
		_Item* items=get_items();
		for(unsigned int u=_m_item_count+count-1; u>_m_item_count-1; u--)
			new (&items[u]) _Item(items[u-count]);
		for(unsigned int u=_m_item_count-1; u+1-count>position; u--)
			items[u]=items[u-count];
		for(unsigned int u=0; u<count; u++)
			items[position+u]=insert[u];
		_m_item_count+=count;
		}
	void remove_at(size_t position)override
		{
		if(position>=_m_item_count)
			throw std::invalid_argument("");
		remove_items(position, 1);
		}
	void remove_items(unsigned int position, unsigned int count)
		{
		_Item* items=get_items();
		for(unsigned int u=position; u+count<_m_item_count; u++)
			items[u]=items[u+count];
		_m_item_count-=count;
		for(unsigned int u=0; u<count; u++)
			items[_m_item_count+u].~_Item();
		}

private:
	// Uninitialized array of items
	unsigned int _m_item_count;
	alignas(alignof(_Item[_Groupsize])) unsigned char _m_items[sizeof(_Item[_Groupsize])];
};


//==============
// Parent-group
//==============

template <typename _Item, unsigned int _Groupsize>
class _list_parent_group: public _list_group<_Item>
{
private:
	// Using
	using _group=_list_group<_Item>;
	using _item_group=_list_item_group<_Item, _Groupsize>;
	using _parent_group=_list_parent_group<_Item, _Groupsize>;

public:
	// Con-Destructors
	_list_parent_group(unsigned int level)noexcept: _m_child_count(0), _m_item_count(0), _m_level(level) {}
	_list_parent_group(_group* child)noexcept:
		_m_child_count(1), _m_item_count(child->get_item_count()), _m_level(child->get_level()+1)
		{
		_m_children[0]=child;
		}
	_list_parent_group(_parent_group const& group):
		_m_child_count(group._m_child_count), _m_item_count(group._m_item_count), _m_level(group._m_level)
		{
		if(_m_level>1)
			{
			for(unsigned int u=0; u<_m_child_count; u++)
				_m_children[u]=new _parent_group((_parent_group const&)*group._m_children[u]);
			}
		else
			{
			for(unsigned int u=0; u<_m_child_count; u++)
				_m_children[u]=new _item_group((_item_group const&)*group._m_children[u]);
			}
		}
	~_list_parent_group()override
		{
		for(unsigned int u=0; u<_m_child_count; u++)
				delete _m_children[u];
		}

	// Access
	_Item& get_at(size_t position)override
		{
		unsigned int group=get_group(&position);
		if(group>=_Groupsize)
			throw std::invalid_argument("");
		return _m_children[group]->get_at(position);
		}
	_Item const& get_at(size_t position)const override
		{
		unsigned int group=get_group(&position);
		if(group>=_Groupsize)
			throw std::invalid_argument("");
		return _m_children[group]->get_at(position);
		}
	inline _group* get_child(unsigned int position)
		{
		if(position>=_m_child_count)
			throw std::invalid_argument("");
		return _m_children[position];
		}
	inline unsigned int get_child_count()const noexcept override { return _m_child_count; }
	inline size_t get_item_count()const noexcept override { return _m_item_count; }
	inline unsigned int get_level()const noexcept override { return _m_level; }

	// Modification
	bool append(_Item const& item, bool again)override
		{
		unsigned int group=_m_child_count-1;
		if(!again)
			{
			if(_m_children[group]->append(item, false))
				{
				_m_item_count++;
				return true;
				}
			unsigned int dst=get_nearest(group);
			if(dst<_m_child_count)
				{
				move_children(group, dst, 1);
				_m_children[group]->append(item, false);
				_m_item_count++;
				return true;
				}
			}
		if(!split(group))
			return false;
		move_children(group, group+1, 1);
		if(!_m_children[group+1]->append(item, again))
			return false;
		_m_item_count++;
		return true;
		}
	void append_groups(unsigned int count, _group* const* groups)noexcept
		{
		for(unsigned int u=0; u<count; u++)
			{
			_m_children[_m_child_count+u]=groups[u];
			_m_item_count+=groups[u]->get_item_count();
			}
		_m_child_count+=count;
		}
	bool combine(unsigned int position)noexcept
		{
		unsigned int countat=_m_children[position]->get_child_count();
		if(position>0)
			{
			if(countat+_m_children[position-1]->get_child_count()<=_Groupsize)
				{
				move_children(position, position-1, countat);
				remove_internal(position);
				return true;
				}
			}
		if(position+1<_m_child_count)
			{
			if(countat+_m_children[position+1]->get_child_count()<=_Groupsize)
				{
				move_children(position, position+1, countat);
				remove_internal(position);
				return true;
				}
			}
		return false;
		}
	bool insert_at(size_t position, _Item const& item, bool again)
		{
		if(position>_m_item_count)
			throw std::invalid_argument("");
		size_t pos=position;
		unsigned int group=0;
		unsigned int inscount=get_insert_pos(&pos, &group);
		if(!inscount)
			return false;
		if(!again)
			{
			size_t at=pos;
			for(unsigned int u=0; u<inscount; u++)
				{
				if(_m_children[group+u]->insert_at(at, item, false))
					{
					_m_item_count++;
					return true;
					}
				at=0;
				}
			unsigned int dst=get_nearest(group);
			if(dst<_m_child_count)
				{
				if(inscount>1&&dst>group)
					group++;
				move_children(group, dst, 1);
				pos=position;
				inscount=get_insert_pos(&pos, &group);
				size_t at=pos;
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
		if(!split(group))
			return false;
		move_children(group, group+1, 1);
		size_t count=_m_children[group]->get_item_count();
		if(pos>=count)
			{
			group++;
			pos-=count;
			}
		_m_children[group]->insert_at(pos, item, again);
		_m_item_count++;
		return true;
		}
	void insert_groups(unsigned int position, unsigned int count, _group* const* groups)noexcept
		{
		if(_m_child_count>position)
			{
			for(unsigned int u=_m_child_count+count-1; u+1-count>position; u--)
				_m_children[u]=_m_children[u-count];
			}
		for(unsigned int u=0; u<count; u++)
			{
			_m_children[position+u]=groups[u];
			_m_item_count+=groups[u]->get_item_count();
			}
		_m_child_count+=count;
		}
	void move_children(unsigned int source, unsigned int destination, unsigned int count)
		{
		if(count==0)
			return;
		while(source>destination+1)
			{
			move_children(destination+1, destination, count);
			destination++;
			}
		while(source+1<destination)
			{
			move_children(destination-1, destination, count);
			destination--;
			}
		if(_m_level>1)
			{
			_parent_group* src=(_parent_group*)_m_children[source];
			_parent_group* dst=(_parent_group*)_m_children[destination];
			if(source>destination)
				{
				dst->append_groups(count, src->_m_children);
				src->remove_groups(0, count);
				}
			else
				{
				unsigned int srccount=src->get_child_count();
				dst->insert_groups(0, count, &src->_m_children[srccount-count]);
				src->remove_groups(srccount-count, count);
				}
			}
		else
			{
			_item_group* src=(_item_group*)_m_children[source];
			_item_group* dst=(_item_group*)_m_children[destination];
			if(source>destination)
				{
				dst->append_items(count, src->get_items());
				src->remove_items(0, count);
				}
			else
				{
				_Item const* srcitems=src->get_items();
				unsigned int srccount=src->get_child_count();
				dst->insert_items(0, count, &srcitems[srccount-count]);
				src->remove_items(srccount-count, count);
				}
			}
		}
	void remove_at(size_t position)override
		{
		if(position>=_m_item_count)
			throw std::invalid_argument("");
		unsigned int group=get_group(&position);
		_m_children[group]->remove_at(position);
		_m_item_count--;
		combine(group);
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
	bool split(unsigned int position)
		{
		if(_m_child_count==_Groupsize)
			return false;
		for(unsigned int u=_m_child_count; u>position+1; u--)
			_m_children[u]=_m_children[u-1];
		if(_m_level>1)
			{
			_m_children[position+1]=new _parent_group(_m_level-1);
			}
		else
			{
			_m_children[position+1]=new _item_group();
			}
		_m_child_count++;
		return true;
		}

private:
	// Common
	unsigned int get_group(size_t* position)const noexcept
		{
		for(unsigned int u=0; u<_m_child_count; u++)
			{
			size_t count=_m_children[u]->get_item_count();
			if(*position<count)
				return u;
			*position-=count;
			}
		return _Groupsize;
		}
	unsigned int get_insert_pos(size_t* position, unsigned int* group)noexcept
		{
		size_t pos=*position;
		for(unsigned int u=0; u<_m_child_count; u++)
			{
			size_t count=_m_children[u]->get_item_count();
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
	unsigned int get_nearest(unsigned int position)noexcept
		{
		int before=position-1;
		unsigned int after=position+1;
		while(before>=0||after<_m_child_count)
			{
			if(before>=0)
				{
				if(_m_children[before]->get_child_count()<_Groupsize)
					return before;
				before--;
				}
			if(after<_m_child_count)
				{
				if(_m_children[after]->get_child_count()<_Groupsize)
					return after;
				after++;
				}
			}
		return _Groupsize;
		}
	void remove_internal(unsigned int position)noexcept
		{
		for(unsigned int u=position; u+1<_m_child_count; u++)
			_m_children[u]=_m_children[u+1];
		_m_child_count--;
		}
	unsigned int _m_child_count;
	_group* _m_children[_Groupsize];
	size_t _m_item_count;
	unsigned int _m_level;
};


//=====================
// Iterator base-class
//=====================

template <typename _Item, unsigned int _Groupsize>
class _list_iterator_base
{
protected:
	// Using
	using _base=_list_iterator_base<_Item, _Groupsize>;
	using _group=_list_group<_Item>;
	using _item_group=_list_item_group<_Item, _Groupsize>;
	using _parent_group=_list_parent_group<_Item, _Groupsize>;

public:
	// Access
	inline _Item get_current()const
		{
		if(_m_current==nullptr)
			throw std::out_of_range("");
		return *_m_current;
		}
	size_t get_position()const noexcept
		{
		size_t pos=0;
		for(unsigned int u=0; u<_m_level_count-1; u++)
			{
			_parent_group* group=(_parent_group*)_m_its[u].group;
			unsigned int grouppos=_m_its[u].position;
			for(unsigned int v=0; v<grouppos; v++)
				pos+=group->get_child(v)->get_item_count();
			}
		pos+=_m_its[_m_level_count-1].position;
		return pos;
		}
	inline bool has_current()const noexcept { return _m_current!=nullptr; }

	// Modification
	bool move_next()noexcept
		{
		if(_m_current==nullptr)
			return false;
		_it_struct* it=&_m_its[_m_level_count-1];
		_item_group* itemgroup=(_item_group*)it->group;
		unsigned int count=itemgroup->get_child_count();
		if(it->position+1<count)
			{
			it->position++;
			_m_current=&itemgroup->get_at(it->position);
			return true;
			}
		for(unsigned int u=_m_level_count-1; u>0; u--)
			{
			it=&_m_its[u-1];
			_parent_group* parentgroup=(_parent_group*)it->group;
			count=parentgroup->get_child_count();
			if(it->position+1>=count)
				continue;
			it->position++;
			_group* group=it->group;
			for(; u<_m_level_count; u++)
				{
				parentgroup=(_parent_group*)group;
				group=parentgroup->get_child(it->position);
				it=&_m_its[u];
				it->group=group;
				it->position=0;
				}
			itemgroup=(_item_group*)group;
			_m_current=&itemgroup->get_at(0);
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
		_item_group* itemgroup=(_item_group*)it->group;
		if(it->position>0)
			{
			it->position--;
			_m_current=&itemgroup->get_at(it->position);
			return true;
			}
		for(unsigned int u=_m_level_count-1; u>0; u--)
			{
			it=&_m_its[u-1];
			_parent_group* parentgroup=(_parent_group*)it->group;
			if(it->position==0)
				continue;
			it->position--;
			_group* group=it->group;
			unsigned int pos=0;
			for(; u<_m_level_count; u++)
				{
				parentgroup=(_parent_group*)group;
				group=parentgroup->get_child(it->position);
				pos=group->get_child_count()-1;
				it=&_m_its[u];
				it->group=group;
				it->position=pos;
				}
			itemgroup=(_item_group*)group;
			_m_current=&itemgroup->get_at(pos);
			return true;
			}
		_m_current=nullptr;
		return false;
		}
	void set_position(size_t position)
		{
		_m_current=nullptr;
		_group* group=_m_root;
		unsigned int levelcount=group->get_level()+1;
		if(_m_level_count!=levelcount)
			{
			operator delete(_m_its);
			_m_its=(_it_struct*)operator new(levelcount*sizeof(_it_struct));
			_m_level_count=levelcount;
			}
		unsigned int pos=get_position_internal(group, &position);
		if(pos==_Groupsize)
			return;
		_m_its[0].group=group;
		_m_its[0].position=pos;
		for(unsigned int u=0; u<_m_level_count-1; u++)
			{
			_parent_group* parentgroup=(_parent_group*)_m_its[u].group;
			group=parentgroup->get_child(pos);
			pos=get_position_internal(group, &position);
			if(pos==_Groupsize)
				return;
			_m_its[u+1].group=group;
			_m_its[u+1].position=pos;
			}
		if(pos<group->get_child_count())
			{
			_item_group* itemgroup=(_item_group*)group;
			_m_current=&itemgroup->get_at(pos);
			}
		}

protected:
	// Con-/Destructors
	_list_iterator_base(_base& it):
		_m_current(it._m_current), _m_its(nullptr), _m_level_count(it._m_level_count), _m_root(it._m_root)
		{
		_m_its=(_it_struct*)operator new(_m_level_count*sizeof(_it_struct));
		for(unsigned int u=0; u<_m_level_count; u++)
			_m_its[u]=it._m_its[u];
		}
	_list_iterator_base(_group* root, size_t position):
		_m_current(nullptr), _m_its(nullptr), _m_level_count(0), _m_root(root) { set_position(position); }
	~_list_iterator_base() { if(_m_its!=nullptr)operator delete(_m_its); }

	// Helper-struct
	typedef struct
		{
		_group* group;
		unsigned int position;
		}_it_struct;

	// Common
	unsigned int get_position_internal(_group* group, size_t* pos)
		{
		unsigned int count=group->get_child_count();
		unsigned int level=group->get_level();
		if(level==0)
			{
			unsigned int u=(unsigned int)*pos;
			*pos=0;
			return u;
			}
		_parent_group* parentgroup=(_parent_group*)group;
		size_t itemcount=0;
		for(unsigned int u=0; u<count; u++)
			{
			_group* child=parentgroup->get_child(u);
			itemcount=child->get_item_count();
			if(*pos<itemcount)
				return u;
			*pos-=itemcount;
			}
		return _Groupsize;
		}
	_Item* _m_current;
	_it_struct* _m_its;
	unsigned int _m_level_count;
	_group* _m_root;
};


//==========
// Iterator
//==========

template <typename _Item, unsigned int _Groupsize>
class _list_iterator: public _list_iterator_base<_Item, _Groupsize>
{
private:
	// Using
	using _base=_list_iterator_base<_Item, _Groupsize>;
	using _group=_list_group<_Item>;
	using _it=_list_iterator<_Item, _Groupsize>;

public:
	// Con-/Destructors
	_list_iterator(_it& it): _base(it) {}
	_list_iterator(_group* root, size_t position): _base(root, position) {}

	// Modification
	void remove_current()
		{
		if(_m_current==nullptr)
			throw std::out_of_range("");
		size_t pos=get_position();
		_m_root->remove_at(pos);
		set_position(pos);
		}
	void set_current(_Item const& item)
		{
		if(_m_current==nullptr)
			throw std::out_of_range("");
		*_m_current=item;
		}
};


//================
// Const-iterator
//================

template <typename _Item, unsigned int _Groupsize>
class _list_const_iterator: public _list_iterator_base<_Item, _Groupsize>
{
private:
	// Using
	using _base=_list_iterator_base<_Item, _Groupsize>;
	using _group=_list_group<_Item>;
	using _it=_list_const_iterator<_Item, _Groupsize>;

public:
	// Con-/Destructors
	_list_const_iterator(_it& it): _base(it) {}
	_list_const_iterator(_group* root, size_t position): _base(root, position) {}
};


//======
// List
//======

template <typename _Item, unsigned int _Groupsize=100>
class list
{
private:
	// Using
	using _const_it=_list_const_iterator<_Item, _Groupsize>;
	using _group=_list_group<_Item>;
	using _it=_list_iterator<_Item, _Groupsize>;
	using _item_group=_list_item_group<_Item, _Groupsize>;
	using _parent_group=_list_parent_group<_Item, _Groupsize>;

public:
	// Con-/Destructors
	list(): _m_root(new _item_group()) {}
	list(list const& list)
		{
		if(list._m_root->get_level()>1)
			{
			_m_root=new _parent_group((_parent_group const&)*list._m_root);
			}
		else
			{
			_m_root=new _item_group((_item_group const&)*list._m_root);
			}
		}
	~list() { delete _m_root; }

	// Access
	inline _Item operator[](size_t position)const { return _m_root->get_at(position); }
	inline _Item get_at(size_t position)const { return _m_root->get_at(position); }
	inline size_t get_count()const noexcept { return _m_root->get_item_count(); }

	// Iteration
	inline _it at(size_t position) { return _it(_m_root, position); }
	inline _const_it at(size_t position)const { return _const_it(_m_root, position); }
	inline _it at(_it const& it) { return _it(it); }
	inline _const_it at(_const_it const& it)const { return _const_it(it); }
	inline _it first() { return _it(_m_root, 0); }
	inline _const_it first()const { return const_it(_m_root, 0); }
	inline _it last() { return _it(_m_root, _m_root->get_item_count()-1); }
	inline _const_it last()const { return _const_it(_m_root, _m_root->get_item_count()-1); }

	// Modification
	void append(_Item const& item)
		{
		if(_m_root->append(item, false))
			return;
		_m_root=new _parent_group(_m_root);
		_m_root->append(item, true);
		}
	void clear()
		{
		delete _m_root;
		_m_root=new _item_group();
		}
	inline void insert_at(size_t position, _Item const& item)
		{
		if(_m_root->insert_at(position, item, false))
			return;
		_m_root=new _parent_group(_m_root);
		_m_root->insert_at(position, item, true);
		}
	void remove_at(size_t position)
		{
		_m_root->remove_at(position);
		update_root();
		}

private:
	// Common
	void update_root()
		{
		if(_m_root->get_child_count()==1&&_m_root->get_level()>0)
			{
			_parent_group* root=(_parent_group*)_m_root;
			_m_root=root->get_child(0);
			root->set_child_count(0);
			delete root;
			}
		}
	_group* _m_root;
};

} // namespace

#endif // _CLUSTERS_LIST