//==========
// list.hpp
//==========

// Implementation of an ordered list
// Items can be inserted and removed in constant low time

// Copyright 2025, Sven Bieg (svenbieg@outlook.de)
// http://github.com/svenbieg/Clusters


#pragma once


//=======
// Using
//=======

#include "cluster.hpp"


//===========
// Namespace
//===========

namespace Collections {


//======================
// Forward-Declarations
//======================

template <class _item_t, typename _size_t, uint16_t _group_size> class list;
template <class _item_t, typename _size_t, uint16_t _group_size> class list_group;
template <class _item_t, typename _size_t, uint16_t _group_size> class list_item_group;
template <class _item_t, typename _size_t, uint16_t _group_size> class list_parent_group;
template <class _traits_t, bool _is_const> class shared_cluster_iterator;

template <class _item_t, typename _size_t, uint16_t _group_size>
struct list_traits
{
using item_t=_item_t;
using group_t=list_group<_item_t, _size_t, _group_size>;
using item_group_t=list_item_group<_item_t, _size_t, _group_size>;
using parent_group_t=list_parent_group<_item_t, _size_t, _group_size>;
using cluster_t=list<_item_t, _size_t, _group_size>;
using iterator_t=cluster_iterator<list_traits, false>;
using const_iterator_t=cluster_iterator<list_traits, true>;
using shared_iterator_t=shared_cluster_iterator<list_traits, false>;
using shared_const_iterator_t=shared_cluster_iterator<list_traits, true>;
using size_t=_size_t;
static constexpr uint16_t group_size=_group_size;
};


//=======
// Group
//=======

template <class _item_t, typename _size_t, uint16_t _group_size>
class list_group: public cluster_group<list_traits<_item_t, _size_t, _group_size>>
{
public:
	// Access
	virtual _size_t get_many(_size_t position, _item_t* items, _size_t count)const=0;

	// Modification
	virtual _item_t* append(_item_t const& item, bool again)=0;
	virtual _size_t append(_item_t const* append, _size_t count)=0;
	virtual _item_t* insert_at(_size_t position, _item_t const& item, bool again)=0;
	virtual _size_t set_many(_size_t position, _item_t const* many, _size_t count)=0;
};


//============
// Item-Group
//============

template <class _item_t, typename _size_t, uint16_t _group_size>
class list_item_group: public cluster_item_group<list_traits<_item_t, _size_t, _group_size>>
{
public:
	// Using
	using _traits_t=list_traits<_item_t, _size_t, _group_size>;
	using _base_t=cluster_item_group<_traits_t>;
	using _group_t=typename _traits_t::group_t;
	using _item_group_t=typename _traits_t::item_group_t;

	// Con-/Destructors
	using _base_t::_base_t;

	// Access
	_size_t get_many(_size_t position, _item_t* many, _size_t count)const override
		{
		uint16_t item_count=this->m_item_count;
		if(position>=item_count)
			throw std::out_of_range(nullptr);
		auto items=this->get_items();
		uint16_t pos=(uint16_t)position;
		uint16_t copy=(uint16_t)(item_count-pos);
		if(copy>count)
			copy=(uint16_t)count;
		for(uint16_t u=0; u<copy; u++)
			many[u]=items[pos+u];
		return copy;
		}

	// Modification
	inline _item_t* append(_item_t const& item, bool again)override
		{
		return this->insert_item(this->m_item_count, item);
		}
	_size_t append(_item_t const* append, _size_t count)override
		{
		uint16_t item_count=this->m_item_count;
		if(item_count==_group_size)
			return 0;
		uint16_t copy=(uint16_t)(_group_size-item_count);
		if(copy>count)
			copy=(uint16_t)count;
		return this->insert_items(item_count, append, copy);
		}
	_item_t* insert_at(_size_t position, _item_t const& item, bool again)override
		{
		if(position>this->m_item_count)
			throw std::out_of_range(nullptr);
		uint16_t pos=(uint16_t)position;
		return this->insert_item(pos, item);
		}
	_size_t set_many(_size_t position, _item_t const* many, _size_t count)override
		{
		uint16_t item_count=this->m_item_count;
		if(position>item_count)
			throw std::out_of_range(nullptr);
		uint16_t pos=(uint16_t)position;
		uint16_t copy=(uint16_t)(item_count-pos);
		if(copy>count)
			copy=(uint16_t)count;
		_item_t* items=this->get_items();
		for(uint16_t u=0; u<copy; u++)
			items[position+u]=many[u];
		return copy;
		}
};


//==============
// Parent-Group
//==============

template <typename _item_t, typename _size_t, uint16_t _group_size>
class list_parent_group: public cluster_parent_group<list_traits<_item_t, _size_t, _group_size>>
{
public:
	// Using
	using _traits_t=list_traits<_item_t, _size_t, _group_size>;
	using _base_t=cluster_parent_group<_traits_t>;
	using _group_t=typename _traits_t::group_t;
	using _item_group_t=typename _traits_t::item_group_t;
	using _parent_group_t=typename _traits_t::parent_group_t;

	// Con-Destructors
	using _base_t::_base_t;

	// Access
	_size_t get_many(_size_t position, _item_t* items, _size_t count)const override
		{
		if(position>=this->m_item_count)
			throw std::out_of_range(nullptr);
		uint16_t group=this->get_group(&position);
		assert(group<this->m_child_count);
		_size_t pos=0;
		while(pos<count)
			{
			auto child=this->get_child(group);
			pos+=child->get_many(position, &items[pos], count-pos);
			if(pos==count)
				break;
			group++;
			if(group==this->m_child_count)
				break;
			position=0;
			}
		return pos;
		}

	// Modification
	_item_t* append(_item_t const& item, bool again)override
		{
		if(!again)
			{
			assert(this->m_child_count>0);
			uint16_t group=(uint16_t)(this->m_child_count-1);
			_item_t* appended=this->m_children[group]->append(item, false);
			if(appended)
				{
				this->m_item_count++;
				return appended;
				}
			uint16_t empty=this->get_nearest_space(group);
			if(empty<this->m_child_count)
				{
				this->move_emtpy_slot(empty, group);
				appended=this->m_children[group]->append(item, false);
				this->m_item_count++;
				return appended;
				}
			}
		uint16_t group=this->m_child_count;
		if(group==_group_size)
			return nullptr;
		uint16_t level=this->m_level;
		if(level>1)
			{
			this->m_children[group]=new _parent_group_t(level-1);
			}
		else
			{
			this->m_children[group]=new _item_group_t();
			}
		this->m_child_count++;
		_item_t* appended=this->m_children[group]->append(item, true);
		this->m_item_count++;
		return appended;
		}
	_size_t append(_item_t const* append, _size_t count)override
		{
		assert(append);
		assert(count>0);
		_size_t pos=0;
		uint16_t child_count=this->m_child_count;
		if(child_count>0)
			{
			auto child=this->get_child(child_count-1);
			pos+=child->append(append, count);
			this->m_item_count+=pos;
			if(pos==count)
				return count;
			}
		if(child_count>1)
			{
			uint16_t last=minimize();
			for(; last<child_count; last++)
				{
				auto child=this->get_child(last);
				auto written=child->append(&append[pos], count-pos);
				if(!written)
					continue;
				this->m_item_count+=written;
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
			child_count=this->m_child_count;
			if(child_count==_group_size)
				break;
			uint16_t level=this->m_level;
			_group_t* group=nullptr;
			if(level==1)
				{
				group=new _item_group_t();
				this->m_children[child_count]=group;
				}
			else
				{
				group=new _parent_group_t((uint16_t)(level-1));
				this->m_children[child_count]=group;
				}
			this->m_child_count++;
			auto written=group->append(&append[pos], count-pos);
			this->m_item_count+=written;
			pos+=written;
			}
		return pos;
		}
	_item_t* insert_at(_size_t position, _item_t const& item, bool again)override
		{
		if(position>this->m_item_count)
			throw std::out_of_range(nullptr);
		_size_t pos=position;
		uint16_t group=0;
		uint16_t ins_count=get_insert_pos(&pos, &group);
		assert(ins_count>0);
		if(!again)
			{
			_size_t at=pos;
			for(uint16_t u=0; u<ins_count; u++)
				{
				auto child=this->get_child(group+u);
				_item_t* inserted=child->insert_at(at, item, false);
				if(inserted)
					{
					this->m_item_count++;
					return inserted;
					}
				at=0;
				}
			if(this->shift_children(group, ins_count))
				{
				pos=position;
				ins_count=get_insert_pos(&pos, &group);
				at=pos;
				for(uint16_t u=0; u<ins_count; u++)
					{
					auto child=this->get_child(group+u);
					_item_t* inserted=child->insert_at(at, item, false);
					if(inserted)
						{
						this->m_item_count++;
						return inserted;
						}
					at=0;
					}
				}
			}
		if(!this->split_child(group))
			return nullptr;
		_size_t count=this->m_children[group]->get_item_count();
		if(pos>=count)
			{
			group++;
			pos-=count;
			}
		auto child=this->get_child(group);
		_item_t* inserted=child->insert_at(pos, item, true);
		this->m_item_count++;
		return inserted;
		}
	_size_t set_many(_size_t position, _item_t const* many, _size_t count)override
		{
		if(position>this->m_item_count)
			throw std::out_of_range(nullptr);
		if(position==this->m_item_count)
			return append(many, count);
		uint16_t group=this->get_group(&position);
		assert(group<this->m_child_count);
		_size_t pos=0;
		while(pos<count)
			{
			auto child=this->get_child(group);
			pos+=child->set_many(position, &many[pos], count-pos);
			if(pos==count)
				break;
			group++;
			if(group==this->m_child_count)
				break;
			position=0;
			}
		if(pos<count)
			pos+=append(&many[pos], count-pos);
		return pos;
		}

private:
	// Access
	uint16_t get_insert_pos(_size_t* position, uint16_t* group)const
		{
		uint16_t child_count=this->m_child_count;
		_size_t pos=*position;
		for(uint16_t u=0; u<child_count; u++)
			{
			_size_t count=this->m_children[u]->get_item_count();
			if(pos<=count)
				{
				*group=u;
				*position=pos;
				if(pos==count&&u+1<child_count)
					return 2;
				return 1;
				}
			pos-=count;
			}
		return 0;
		}

	// Modification
	void free_children()
		{
		for(uint16_t u=this->m_child_count; u>0; u--)
			{
			uint16_t pos=u-1;
			if(this->m_children[pos]->get_child_count()>0)
				break;
			this->remove_group(pos);
			}
		}
	uint16_t minimize()
		{
		uint16_t child_count=this->m_child_count;
		uint16_t dst=0;
		for(; dst+1<child_count; dst++)
			{
			uint16_t dst_count=this->m_children[dst]->get_child_count();
			if(dst_count==_group_size)
				continue;
			uint16_t free=(uint16_t)(_group_size-dst_count);
			uint16_t src=(uint16_t)(dst+1);
			for(; src<child_count; src++)
				{
				uint16_t src_count=this->m_children[src]->get_child_count();
				if(src_count==0)
					continue;
				uint16_t move=src_count;
				if(move>free)
					move=free;
				this->move_children(src, dst, move);
				free=(uint16_t)(free-move);
				if(free==0)
					break;
				}
			}
		return dst;
		}
};


//======
// List
//======

template <typename _item_t, typename _size_t=uint32_t, uint16_t _group_size=10>
class list: public cluster<list_traits<_item_t, _size_t, _group_size>>
{
public:
	// Using
	using _traits_t=list_traits<_item_t, _size_t, _group_size>;
	using _base_t=cluster<_traits_t>;
	using _group_t=typename _traits_t::group_t;

	// Con-/Destructors
	list(): _base_t(nullptr) {}
	list(list const& list): _base_t(nullptr)
		{
		this->copy_from(list);
		}

	// Access
	inline _item_t& operator[](_size_t position) { return this->get_at(position); }
	inline _item_t const& operator[](_size_t position)const { return this->get_at(position); }
	inline bool contains(_item_t const& item) { return index_of(item, nullptr); }
	_size_t get_many(_size_t position, _item_t* items, _size_t count)const
		{
		auto root=this->m_root;
		if(!root)
			throw std::out_of_range(nullptr);
		return root->get_many(position, items, count);
		}
	bool index_of(_item_t const& item, _size_t* position)const
		{
		_size_t pos=0;
		for(auto it=this->cbegin(); it.has_current(); it.move_next())
			{
			if(*it==item)
				{
				if(position!=nullptr)
					*position=pos;
				return true;
				}
			pos++;
			}
		return false;
		}

	// Modification
	inline list& operator=(list const& list)
		{
		this->copy_from(list);
		return *this;
		}
	bool add(_item_t const& item)
		{
		if(this->contains(item))
			return false;
		append(item);
		return true;
		}
	inline _item_t& append() { return append(_item_t()); }
	_item_t& append(_item_t const& item)
		{
		auto root=this->create_root();
		_item_t* appended=root->append(item, false);
		if(appended)
			return *appended;
		root=this->lift_root();
		return *root->append(item, true);
		}
	void append(_item_t const* items, _size_t count)
		{
		auto root=this->create_root();
		_size_t pos=0;
		while(1)
			{
			pos+=root->append(&items[pos], count-pos);
			if(pos==count)
				break;
			root=this->lift_root();
			}
		}
	inline _item_t& insert_at(_size_t position) { return insert_at(position, _item_t()); }
	_item_t& insert_at(_size_t position, _item_t const& item)
		{
		auto root=this->m_root;
		if(!root)
			{
			if(position>0)
				throw std::out_of_range(nullptr);
			root=this->create_root();
			}
		_item_t* inserted=root->insert_at(position, item, false);
		if(inserted)
			return *inserted;
		root=this->lift_root();
		return *root->insert_at(position, item, true);
		}
	bool remove(_item_t const& item)
		{
		for(auto it=this->begin(); it.has_current(); it.move_next())
			{
			if(*it==item)
				{
				it.remove_current();
				return true;
				}
			}
		return false;
		}
	bool set_at(_size_t position, _item_t const& item)
		{
		_item_t& got=get_at(position);
		if(*got==item)
			return false;
		*got=item;
		return true;
		}
	_size_t set_many(_size_t position, _item_t const* items, _size_t count)
		{
		auto root=this->m_root;
		if(!root)
			{
			if(position>0)
				throw std::out_of_range(nullptr);
			root=this->create_root();
			}
		for(_size_t pos=0; pos<count; )
			{
			pos+=root->set_many(position+pos, &items[pos], count-pos);
			if(pos==count)
				break;
			root=this->lift_root();
			}
		return count;
		}

protected:
	// Con-/Destructors
	list(_group_t* root): _base_t(root) {}
};

}