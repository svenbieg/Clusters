//=========
// index.h
//=========

// Implementation of a sorted index.
// Ids and can be inserted, removed and looked-up in real-time.

// Copyright 2018, Sven Bieg (svenbieg@web.de)
// http://svenbieg.azurewebsites.net


#ifndef _CLUSTERS_INDEX
#define _CLUSTERS_INDEX


//=======
// Using
//=======

#include <exception>
#include <utility>


//===========
// Namespace
//===========

namespace clusters {


//======================
// Forward-Declarations
//======================

template <typename _Id, typename _Item, unsigned int _Groupsize> class index;


//======
// Item
//======

template <typename _Id, typename _Item>
class _index_item
{
public:
	_index_item()noexcept {}
	_index_item(_index_item const& item): Id(item.Id), Item(item.Item) {}
	_index_item(_index_item const&& item): Id(std::move(item.Id)), Item(std::move(item.Item)) {}
	_index_item(_Id const& id, _Item const& item): Id(id), Item(item) {}
	_Id Id;
	_Item Item;
};

template <typename _Id>
class _index_item<_Id, void>
{
public:
	_index_item() {}
	_index_item(_index_item const& item): Id(item.Id) {}
	_index_item(_index_item const&& item): Id(std::move(item.Id)) {}
	_index_item(_Id const& id): Id(id) {}
	_Id Id;
};


//=======
// Group
//=======

template <typename _Id, typename _Item>
class _index_group
{
private:
	// Using 
	using _item=_index_item<_Id, _Item>;

public:
	// Con-Destructors
	virtual ~_index_group() {}

	// Access
	virtual bool contains(_Id const& id)const noexcept=0;
	virtual int find(_Id const& id)const noexcept=0;
	virtual _item* get(_Id const& id)noexcept=0;
	virtual _item const* get(_Id const& id)const noexcept=0;
	virtual _item& get_at(size_t position)=0;
	virtual _item const& get_at(size_t position)const=0;
	virtual unsigned int get_child_count()const noexcept=0;
	virtual _item* get_first()noexcept=0;
	virtual _item const* get_first()const noexcept=0;
	virtual size_t get_item_count()const noexcept=0;
	virtual _item* get_last()noexcept=0;
	virtual _item const* get_last()const noexcept=0;
	virtual unsigned int get_level()const noexcept=0;

	// Modification
	virtual bool add(_item const& item, bool again, bool* exists)=0;
	virtual bool remove(_Id const& id)=0;
	virtual void remove_at(size_t position)=0;
};


//============
// Item-group
//============

template <typename _Id, typename _Item, unsigned int _Groupsize>
class _index_item_group: public _index_group<_Id, _Item>
{
private:
	// Using
	using _item=_index_item<_Id, _Item>;

public:
	// Con-/Destructors
	_index_item_group()noexcept: _m_item_count(0) {}
	_index_item_group(_index_item_group const& group): _m_item_count(group._m_item_count)
		{
		_item* dst=get_items();
		_item const* src=group.get_items();
		for(unsigned int u=0; u<_m_item_count; u++)
			new (&dst[u]) _item(src[u]);
		}
	~_index_item_group()
		{
		_item* items=get_items();
		for(unsigned int u=0; u<_m_item_count; u++)
			items[u].~_item();
		}

	// Access
	inline bool contains(_Id const& id)const noexcept override { return get_item_pos(id)>=0; }
	inline int find(_Id const& id)const noexcept override { return get_item_pos(id); }
	_item* get(_Id const& id)noexcept override
		{
		int ipos=get_item_pos(id);
		if(ipos<0)
			return nullptr;
		return &get_items()[ipos];
		}
	_item const* get(_Id const& id)const noexcept override
		{
		int ipos=get_item_pos(id);
		if(ipos<0)
			return nullptr;
		return &get_items()[ipos];
		}
	_item& get_at(size_t position)override
		{
		if(position>=_m_item_count)
			throw std::invalid_argument("");
		return get_items()[position];
		}
	_item const& get_at(size_t position)const override
		{
		if(position>=_m_item_count)
			throw std::invalid_argument("");
		return get_items()[position];
		}
	inline unsigned int get_child_count()const noexcept override { return _m_item_count; }
	_item* get_first()noexcept
		{
		if(_m_item_count==0)
			return nullptr;
		return get_items();
		}
	_item const* get_first()const noexcept
		{
		if(_m_item_count==0)
			return nullptr;
		return get_items();
		}
	inline size_t get_item_count()const noexcept override { return _m_item_count; }
	inline _item* get_items()noexcept { return (_item*)_m_items; }
	inline _item const* get_items()const noexcept { return (_item const*)_m_items; }
	_item* get_last()noexcept
		{
		if(_m_item_count==0)
			return nullptr;
		return &get_items()[_m_item_count-1];
		}
	_item const* get_last()const noexcept
		{
		if(_m_item_count==0)
			return nullptr;
		return &get_items()[_m_item_count-1];
		}
	inline unsigned int get_level()const noexcept override { return 0; }

	// Modification
	bool add(_item const& item, bool again, bool* exists)override
		{
		int pos=get_insert_pos(item.Id);
		if(pos<0)
			{
			*exists=true;
			return false;
			}
		if(_m_item_count==_Groupsize)
			return false;
		_item* items=get_items();
		for(int i=_m_item_count; i>pos; i--)
			new (&items[i]) _item(std::move(items[i-1]));
		new (&items[pos]) _item(item);
		_m_item_count++;
		return true;
		}
	void append_items(unsigned int count, _item const* append)
		{
		_item* items=get_items();
		for(unsigned int u=0; u<count; u++)
			new (&items[_m_item_count+u]) _item(std::move(append[u]));
		_m_item_count+=count;
		}
	void insert_items(unsigned int position, unsigned int count, _item const* insert)
		{
		if(position==_m_item_count)
			{
			append_items(count, insert);
			return;
			}
		_item* items=get_items();
		for(unsigned int u=_m_item_count+count-1; u+1-count>position; u--)
			new (&items[u]) _item(std::move(items[u-count]));
		for(unsigned int u=0; u<count; u++)
			new (&items[position+u]) _item(std::move(insert[u]));
		_m_item_count+=count;
		}
	bool remove(_Id const& id)override
		{
		int pos=get_item_pos(id);
		if(pos<0)
			return false;
		remove_at(pos);
		return true;
		}
	void remove_at(size_t position)override
		{
		if(position>=_m_item_count)
			throw std::invalid_argument("");
		_item* items=get_items();
		items[position].~_item();
		for(unsigned int u=position; u+1<_m_item_count; u++)
			new (&items[u]) _item(std::move(items[u+1]));
		_m_item_count--;
		}
	void remove_items(unsigned int position, unsigned int count)
		{
		_item* items=get_items();
		for(unsigned int u=position; u+count<_m_item_count; u++)
			new (&items[u]) _item(std::move(items[u+count]));
		_m_item_count-=count;
		}

private:
	// Common
	int get_insert_pos(_Id const& id)const noexcept
		{
		_item const* items=get_items();
		unsigned int start=0;
		unsigned int end=_m_item_count;
		while(start<end)
			{
			unsigned int u=start+(end-start)/2;
			if(items[u].Id>id)
				{
				end=u;
				continue;
				}
			if(items[u].Id<id)
				{
				start=u+1;
				continue;
				}
			return -1;
			}
		return start;
		}
	int get_item_pos(_Id const& id)const noexcept
		{
		if(!_m_item_count)
			return -1;
		_item const* items=get_items();
		_item const* item=nullptr;
		unsigned int start=0;
		unsigned int end=_m_item_count;
		unsigned int u=0;
		while(start<end)
			{
			u=start+(end-start)/2;
			item=&items[u];
			if(item->Id>id)
				{
				end=u;
				continue;
				}
			if(item->Id<id)
				{
				start=u+1;
				continue;
				}
			return u;
			}
		if(u>0&&item->Id>id)
			u--;
		return -(int)u-1;
		}

	// Uninitialized array of items
	unsigned int _m_item_count;
	alignas(alignof(_item[_Groupsize])) unsigned char _m_items[sizeof(_item[_Groupsize])];
};


//==============
// Parent-group
//==============

template <typename _Id, typename _Item, unsigned int _Groupsize>
class _index_parent_group: public _index_group<_Id, _Item>
{
private:
	// Using
	using _item=_index_item<_Id, _Item>;
	using _group=_index_group<_Id, _Item>;
	using _item_group=_index_item_group<_Id, _Item, _Groupsize>;
	using _parent_group=_index_parent_group<_Id, _Item, _Groupsize>;

public:
	// Con-Destructors
	_index_parent_group(unsigned int level)noexcept:
		_m_child_count(0), _m_first(nullptr), _m_item_count(0), _m_last(nullptr), _m_level(level) {}
	_index_parent_group(_group* child)noexcept:
		_m_child_count(1), _m_first(child->get_first()), _m_item_count(child->get_item_count()),
		_m_last(child->get_last()), _m_level(child->get_level()+1)
		{
		_m_children[0]=child;
		}
	_index_parent_group(_parent_group const& group):
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
		_m_first=_m_children[0]->get_first();
		_m_last=_m_children[_m_child_count-1]->get_last();
		}
	~_index_parent_group()override
		{
		for(unsigned int u=0; u<_m_child_count; u++)
				delete _m_children[u];
		}

	// Access
	inline bool contains(_Id const& id)const noexcept override { return get_item_pos(id)>=0; }
	inline int find(_Id const& id)const noexcept override { return get_item_pos(id); }
	_item* get(_Id const& id)noexcept override
		{
		int ipos=get_item_pos(id);
		if(ipos<0)
			return nullptr;
		return _m_children[ipos]->get(id);
		}
	_item const* get(_Id const& id)const noexcept override
		{
		int ipos=get_item_pos(id);
		if(ipos<0)
			return nullptr;
		return _m_children[ipos]->get(id);
		}
	_item& get_at(size_t position)override
		{
		unsigned int group=get_group(&position);
		if(group>=_Groupsize)
			throw std::invalid_argument("");
		return _m_children[group]->get_at(position);
		}
	_item const& get_at(size_t position)const override
		{
		unsigned int group=get_group(&position);
		if(group>=_Groupsize)
			throw std::invalid_argument("");
		return _m_children[group]->get_at(position);
		}
	_group* get_child(unsigned int position)
		{
		if(position>=_m_child_count)
			throw std::invalid_argument("");
		return _m_children[position];
		}
	inline unsigned int get_child_count()const noexcept override { return _m_child_count; }
	inline _item* get_first()noexcept override { return _m_first; }
	inline _item const* get_first()const noexcept override { return const_cast<_item const*>(_m_first); }
	inline size_t get_item_count()const noexcept override { return _m_item_count; }
	inline _item* get_last()noexcept override { return _m_last; }
	inline _item const* get_last()const noexcept override { return const_cast<_item const*>(_m_last); }
	inline unsigned int get_level()const noexcept override { return _m_level; }

	// Modification
	bool add(_item const& item, bool again, bool* exists)override
		{
		if(!add_internal(item, again, exists))
			return false;
		_m_item_count++;
		update_bounds();
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
		update_bounds();
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
		update_bounds();
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
				_item const* srcitems=src->get_items();
				unsigned int srccount=src->get_child_count();
				dst->insert_items(0, count, &srcitems[srccount-count]);
				src->remove_items(srccount-count, count);
				}
			}
		}
	bool remove(_Id const& id)override
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
	void remove_at(size_t position)override
		{
		if(position>=_m_item_count)
			throw std::invalid_argument("");
		unsigned int group=get_group(&position);
		_m_children[group]->remove_at(position);
		_m_item_count--;
		combine(group);
		update_bounds();
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
	bool add_internal(_item const& item, bool again, bool* exists)
		{
		unsigned int group=0;
		unsigned int count=get_insert_pos(item.Id, &group);
		if(count==_Groupsize)
			{
			*exists=true;
			return false;
			}
		if(!again)
			{
			for(unsigned int u=0; u<count; u++)
				{
				if(_m_children[group+u]->add(item, false, exists))
					return true;
				if(*exists)
					return false;
				}
			unsigned int dst=get_nearest(group);
			if(dst<_m_child_count)
				{
				if(count>1&&dst>group)
					group++;
				move_children(group, dst, 1);
				if(_m_children[group]->add(item, false, exists))
					return true;
				}
			}
		if(!split(group))
			return false;
		move_children(group, group+1, 1);
		_item* last=_m_children[group]->get_last();
		if(last->Id<item.Id)
			group++;
		_m_children[group]->add(item, again, exists);
		return true;
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
		return _Groupsize;
		}
	int get_item_pos(_Id const& id)const
		{
		if(!_m_child_count)
			return -1;
		unsigned int start=0;
		unsigned int end=_m_child_count;
		unsigned int u=0;
		_item* first=nullptr;
		_item* last=nullptr;
		while(start<end)
			{
			u=start+(end-start)/2;
			first=_m_children[u]->get_first();
			if(first->Id>id)
				{
				end=u;
				continue;
				}
			last=_m_children[u]->get_last();
			if(last->Id<id)
				{
				start=u+1;
				continue;
				}
			return u;
			}
		if(u>0&&first->Id>id)
			u--;
		return -(INT)u-1;
		}
	unsigned int get_insert_pos(_Id const& id, unsigned int* group)const
		{
		if(!_m_child_count)
			return 0;
		unsigned int start=0;
		unsigned int end=_m_child_count;
		_item* first=nullptr;
		_item* last=nullptr;
		while(start<end)
			{
			unsigned int u=start+(end-start)/2;
			first=_m_children[u]->get_first();
			last=_m_children[u]->get_last();
			if(first->Id==id)
				return _Groupsize;
			if(last->Id==id)
				return _Groupsize;
			if(first->Id>id)
				{
				end=u;
				continue;
				}
			if(last->Id<id)
				{
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
			if(first->Id>id)
				{
				*group=start-1;
				return 2;
				}
			}
		if(start+1<_m_child_count)
			{
			last=_m_children[start]->get_last();
			if(last->Id<id)
				return 2;
			}
		return 1;
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
	void update_bounds()
		{
		if(!_m_child_count)
			return;
		_m_first=_m_children[0]->get_first();
		_m_last=_m_children[_m_child_count-1]->get_last();
		}
	unsigned int _m_child_count;
	_group* _m_children[_Groupsize];
	_item* _m_first;
	size_t _m_item_count;
	_item* _m_last;
	unsigned int _m_level;
};


//=====================
// Iterator base-class
//=====================

template <typename _Id, typename _Item, unsigned int _Groupsize>
class _index_iterator_base
{
protected:
	// Using
	using _base=_index_iterator_base<_Id, _Item, _Groupsize>;
	using _item=_index_item<_Id, _Item>;
	using _group=_index_group<_Id, _Item>;
	using _item_group=_index_item_group<_Id, _Item, _Groupsize>;
	using _parent_group=_index_parent_group<_Id, _Item, _Groupsize>;

public:
	// Access
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
	bool find(_Id const& id)
		{
		_m_current=nullptr;
		bool bfound=true;
		_group* group=_m_root;
		unsigned int levelcount=group->get_level()+1;
		set_level_count(levelcount);
		for(unsigned int u=0; u<levelcount-1; u++)
			{
			_parent_group* parentgroup=(_parent_group*)group;
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
		_item_group* itemgroup=(_item_group*)group;
		int pos=itemgroup->find(id);
		if(pos<0)
			{
			bfound=false;
			pos++;
			pos*=-1;
			}
		_m_its[levelcount-1].group=group;
		_m_its[levelcount-1].position=pos;
		_m_current=&itemgroup->get_at(pos);
		return bfound;
		}
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
		set_level_count(levelcount);
		unsigned int pos=get_position_internal(group, &position);
		_m_its[0].group=group;
		_m_its[0].position=pos;
		for(unsigned int u=0; u<_m_level_count-1; u++)
			{
			_parent_group* parentgroup=(_parent_group*)_m_its[u].group;
			group=parentgroup->get_child(pos);
			pos=get_position_internal(group, &position);
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
	_index_iterator_base(_base& it):
		_m_current(it._m_current), _m_its(nullptr), _m_level_count(it._m_level_count), _m_root(it._m_root)
		{
		_m_its=(_it_struct*)operator new(_m_level_count*sizeof(_it_struct));
		for(unsigned int u=0; u<_m_level_count; u++)
			_m_its[u]=it._m_its[u];
		}
	_index_iterator_base(_group* root, size_t position):
		_m_its(nullptr), _m_level_count(0), _m_root(root) { set_position(position); }
	_index_iterator_base(_group* root, _Id const& id):
		_m_its(nullptr), _m_level_count(0), _m_root(root) { find(id); }
	~_index_iterator_base() { if(_m_its!=nullptr)operator delete(_m_its); }

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
	void set_level_count(unsigned int levelcount)
		{
		if(_m_level_count==levelcount)
			return;
		operator delete(_m_its);
		_m_its=(_it_struct*)operator new(levelcount*sizeof(_it_struct));
		_m_level_count=levelcount;
		}
	_item* _m_current;
	_it_struct* _m_its;
	unsigned int _m_level_count;
	_group* _m_root;
};


//==========
// Iterator
//==========

template <typename _Id, typename _Item, unsigned int _Groupsize>
class _index_iterator: public _index_iterator_base<_Id, _Item, _Groupsize>
{
private:
	// Using
	using _base=_index_iterator_base<_Id, _Item, _Groupsize>;
	using _group=_index_group<_Id, _Item>;
	using _it=_index_iterator<_Id, _Item, _Groupsize>;

public:
	// Con-/Destructors
	_index_iterator(_it& it): _base(it) {}
	_index_iterator(_group* root, size_t position): _base(root, position) {}
	_index_iterator(_group* root, _Id const& id): _base(root, id) {}

	// Access
	_Id get_current_id()const
		{
		if(_m_current==nullptr)
			throw std::out_of_range("");
		return _m_current->Id;
		}
	_Item get_current_item()const
		{
		if(_m_current==nullptr)
			throw std::out_of_range("");
		return _m_current->Item;
		}

	// Modification
	void remove_current()
		{
		if(_m_current==nullptr)
			throw std::out_of_range("");
		size_t pos=get_position();
		_m_root->remove_at(pos);
		set_position(pos);
		}
	void set_current_item(_Item const& item)
		{
		if(_m_current==nullptr)
			throw std::out_of_range("");
		_m_current->Item=item;
		}
};

template <typename _Id, unsigned int _Groupsize>
class _index_iterator<_Id, void, _Groupsize>: public _index_iterator_base<_Id, void, _Groupsize>
{
private:
	// Using
	using _base=_index_iterator_base<_Id, void, _Groupsize>;
	using _group=_index_group<_Id, void>;
	using _it=_index_iterator<_Id, void, _Groupsize>;

public:
	// Con-/Destructors
	_index_iterator(_it& it): _base(it) {}
	_index_iterator(_group* root, size_t position): _base(root, position) {}
	_index_iterator(_group* root, _Id const& id): _base(root, id) {}

	// Access
	inline _Id get_current()const
		{
		if(_m_current==nullptr)
			throw std::out_of_range("");
		return _m_current->Id;
		}

	// Modification
	void remove_current()
		{
		if(_m_current==nullptr)
			throw std::out_of_range("");
		size_t pos=get_position();
		_m_root->remove_at(pos);
		set_position(pos);
		}
};


//================
// Const-iterator
//================

template <typename _Id, typename _Item, unsigned int _Groupsize>
class _index_const_iterator: public _index_iterator_base<_Id, _Item, _Groupsize>
{
private:
	// Using
	using _base=_index_iterator_base<_Id, _Item, _Groupsize>;
	using _group=_index_group<_Id, _Item>;
	using _it=_index_const_iterator<_Id, _Item, _Groupsize>;

public:
	// Con-/Destructors
	_index_const_iterator(_it& it): _base(it) {}
	_index_const_iterator(_group* root, size_t position): _base(root, position) {}
	_index_const_iterator(_group* root, _Id const& id): _base(root, id) {}

	// Access
	inline _Id get_current_id()const
		{
		if(_m_current==nullptr)
			throw std::out_of_range("");
		return _m_current->Id;
		}
	inline _Id get_current_item()const
		{
		if(_m_current==nullptr)
			throw std::out_of_range("");
		return _m_current->Item;
		}
};

template <typename _Id, unsigned int _Groupsize>
class _index_const_iterator<_Id, void, _Groupsize>: public _index_iterator_base<_Id, void, _Groupsize>
{
private:
	// Using
	using _base=_index_iterator_base<_Id, void, _Groupsize>;
	using _group=_index_group<_Id, void>;
	using _it=_index_const_iterator<_Id, void, _Groupsize>;

public:
	// Con-/Destructors
	_index_const_iterator(_it& it): _base(it) {}
	_index_const_iterator(_group* root, size_t position): _base(root, position) {}
	_index_const_iterator(_group* root, _Id const& id): _base(root, id) {}

	// Access
	inline _Id get_current()const
		{
		if(_m_current==nullptr)
			throw std::out_of_range("");
		return _m_current->Id;
		}
};


//==================
// Index base-class
//==================

template <typename _Id, typename _Item, unsigned int _Groupsize>
class _index_base
{
private:
	// Using
	using _const_it=_index_const_iterator<_Id, _Item, _Groupsize>;
	using _group=_index_group<_Id, _Item>;
	using _it=_index_iterator<_Id, _Item, _Groupsize>;
	using _item_group=_index_item_group<_Id, _Item, _Groupsize>;
	using _parent_group=_index_parent_group<_Id, _Item, _Groupsize>;

public:
	// Con-/Destructors
	_index_base(): _m_root(new _item_group()) {}
	_index_base(_index_base const& index)
		{
		if(index._m_root->get_level()>1)
			{
			_m_root=new _parent_group((_parent_group const&)*index._m_root);
			}
		else
			{
			_m_root=new _item_group((_item_group const&)*index._m_root);
			}
		}
	~_index_base() { delete _m_root; }

	// Access
	inline bool contains(_Id const& id)const noexcept { return _m_root->contains(id); }
	inline size_t get_count()const noexcept { return _m_root->get_item_count(); }

	// Iteration
	inline _it at(size_t position) { return _it(_m_root, position); }
	inline _const_it at(size_t position)const { return _const_it(_m_root, position); }
	inline _it at(_it const& it) { return _it(it); }
	inline _const_it at(_const_it const& it)const { return _const_it(it); }
	inline _it find(_Id const& id) { return _it(_m_root, id); }
	inline _const_it find(_Id const& id)const { return _const_it(_m_root, id); }
	inline _it first() { return _it(_m_root, 0); }
	inline _const_it first()const { return const_it(_m_root, 0); }
	inline _it last() { return _it(_m_root, _m_root->get_item_count()-1); }
	inline _const_it last()const { return _const_it(_m_root, _m_root->get_item_count()-1); }

	// Modification
	void clear()
		{
		delete _m_root;
		_m_root=new _item_group();
		}
	bool remove(_Id const& id)
		{
		if(_m_root->remove(id))
			{
			update_root();
			return true;
			}
		return false;
		}
	void remove_at(size_t position)
		{
		_m_root->remove_at(position);
		update_root();
		}

protected:
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


//=============
// Index typed
//=============

template <typename _Id, typename _Item, unsigned int _Groupsize>
class _index_typed: public _index_base<_Id, _Item, _Groupsize>
{
private:
	// Using
	using _base=_index_base<_Id, _Item, _Groupsize>;
	using _item=_index_item<_Id, _Item>;
	using _parent_group=_index_parent_group<_Id, _Item, _Groupsize>;

public:
	// Con-/Destructors
	_index_typed() {}
	_index_typed(_base const& base): _base(base) {}

	// Access
	inline _Item operator[](_Id const& id)const { return get(id); }
	_Item get(_Id const& id)const
		{
		_item* item=_m_root->get(id);
		if(item==nullptr)
			throw std::invalid_argument("");
		return item->Item;
		}
	bool try_get(_Id const& id, _Item* item)const noexcept
		{
		_item* ii=_m_root->get(id);
		if(ii==nullptr)
			return false;
		*item=ii->Item;
		return true;
		}

	// Modification
	bool add(_Id const& id, _Item const& item)
		{
		_item ii(id, item);
		bool exists=false;
		if(_m_root->add(ii, false, &exists))
			return true;
		if(exists)
			return false;
		_m_root=new _parent_group(_m_root);
		return _m_root->add(ii, true, &exists);
		}
	void set(_Id const& id, _Item const& item)
		{
		_item* p=_m_root->get(id);
		if(p==nullptr)
			{
			add(id, item);
			return;
			}
		p->Item=item;
		}
};

template <typename _Id, unsigned int _Groupsize>
class _index_typed<_Id, void, _Groupsize>: public _index_base<_Id, void, _Groupsize>
{
private:
	// Using
	using _base=_index_base<_Id, void, _Groupsize>;
	using _item=_index_item<_Id, void>;
	using _parent_group=_index_parent_group<_Id, void, _Groupsize>;

public:
	// Con-/Destructors
	_index_typed() {}
	_index_typed(_base const& base): _base(base) {}

	// Access
	inline _Id operator[](size_t position)const { return _m_root->get_at(position)->Id; }
	inline _Id get_at(size_t position)const { return _m_root->get_at(position)->Id; }

	// Modification
	bool add(_Id const& id)
		{
		_item item(id);
		bool exists=false;
		if(_m_root->add(item, false, &exists))
			return true;
		if(exists)
			return false;
		_m_root=new _parent_group(_m_root);
		return _m_root->add(item, true, &exists);
		}
};


//=======
// Index
//=======

template <typename _Id, typename _Item=void, unsigned int _Groupsize=100>
class index: public _index_typed<_Id, _Item, _Groupsize>
{
private:
	// Using
	using _base=_index_typed<_Id, _Item, _Groupsize>;

public:
	// Con-/Destructors
	index() {}
	index(index const& index): _base(index) {}
};

} // namespace

#endif // _CLUSTERS_INDEX