//=========
// Index.h
//=========

#pragma once


//=======
// Using
//=======

#include "shared_index.hpp"


//===========
// Namespace
//===========

namespace Collections {


//======================
// Forward-Declarations
//======================

template <typename _id_t, typename _size_t, WORD _group_size> class IndexIterator;
template <typename _id_t, typename _size_t, WORD _group_size> class ConstIndexIterator;


//=======
// Index
//=======

template <typename _id_t, typename _size_t=UINT, WORD _group_size=10>
class Index: public Object
{
private:
	// Using
	using _index_t=Index<_id_t, _size_t, _group_size>;

public:
	// Types
	using FindFunction=find_func;
	using Iterator=IndexIterator<_id_t, _size_t, _group_size>;
	using ConstIterator=ConstIndexIterator<_id_t, _size_t, _group_size>;

	// Friends
	friend Iterator;
	friend ConstIterator;

	// Con-/Destructors
	static inline Handle<Index> Create() { return new Index(); }
	static inline Handle<Index> Create(_index_t const* Copy) { return new Index(Copy); }

	// Access
	inline Handle<Iterator> At(UINT Position) { return new Iterator(this, Position); }
	inline Handle<ConstIterator> AtConst(UINT Position) { return new ConstIterator(this, Position); }
	inline Handle<Iterator> Begin() { return new Iterator(this, 0); }
	inline Handle<ConstIterator> BeginConst() { return new ConstIterator(this, 0); }
	inline BOOL Contains(_id_t const& Id) { return m_Index.contains(Id); }
	inline Handle<Iterator> End()
		{
		auto it=new Iterator(this, -2);
		it->End();
		return it;
		}
	inline Handle<ConstIterator> EndConst()
		{
		auto it=new ConstIterator(this, -2);
		it->End();
		return it;
		}
	inline Handle<Iterator> Find(_id_t const& Id, FindFunction Function=FindFunction::equal)
		{
		auto it=new Iterator(this, -2);
		it->Find(Id, Function);
		return it;
		}
	inline Handle<ConstIterator> FindConst(_id_t const& Id, FindFunction Function=FindFunction::equal)
		{
		auto it=new ConstIterator(this, -2);
		it->Find(Id, Function);
		return it;
		}
	inline _id_t GetAt(_size_t Position) { return m_Index.get_at(Position); }
	inline _size_t GetCount() { return m_Index.get_count(); }

	// Modification
	BOOL Add(_id_t const& Id, BOOL Notify=true)
		{
		if(m_Index.add(Id))
			{
			if(Notify)
				{
				Added(this, Id);
				Changed(this);
				}
			return true;
			}
		return false;
		}
	Event<Index, _id_t> Added;
	Event<Index> Changed;
	BOOL Clear(BOOL Notify=true)
		{
		if(m_Index.clear())
			{
			if(Notify)
				Changed(this);
			return true;
			}
		return false;
		}
	BOOL Remove(_id_t const& Id, BOOL Notify=true)
		{
		if(m_Index.remove(Id))
			{
			if(Notify)
				{
				Removed(this, Id);
				Changed(this);
				}
			return true;
			}
		return false;
		}
	BOOL Set(_id_t const& Id, BOOL Notify=true)
		{
		if(m_Index.set(Id))
			{
			if(Notify)
				{
				Added(this, Id);
				Changed(this);
				}
			return true;
			}
		return false;
		}
	Event<Index, _id_t> Removed;

private:
	// Con-/Destructors
	Index() {}
	Index(_index_t const* Copy)
		{
		if(Copy)
			m_Index.copy_from(Copy->m_Index);
		}

	// Common
	shared_index<_id_t, _size_t, _group_size> m_Index;
};


//==========
// Iterator
//==========

template <typename _id_t, typename _size_t, WORD _group_size>
class IndexIterator: public Object
{
private:
	// Using
	using _index_t=Index<_id_t, _size_t, _group_size>;

public:
	// Friends
	friend _index_t;

	// Using
	using FindFunction=find_func;

	// Access
	inline _id_t GetCurrent()const { return *m_It; }
	inline BOOL HasCurrent()const { return m_It.has_current(); }

	// Navigation
	inline BOOL Find(_id_t const& Id, FindFunction Function=FindFunction::equal) { return m_It.find(Id, Function); }
	inline BOOL MoveNext() { return m_It.move_next(); }
	inline BOOL MovePrevious() { return m_It.move_previous(); }

	// Modification
	BOOL RemoveCurrent(BOOL Notify=true)
		{
		if(!Notify)
			return m_It.remove_current();
		if(!m_It.has_current())
			return false;
		_id_t id=m_It.get_current_id();
		m_It.remove_current();
		m_Index->Removed(m_Index, id);
		m_Index->Changed(m_Index);
		return true;
		}

private:
	// Con-/Destructors
	IndexIterator(_index_t* Index, _size_t Position): m_It(&Index->m_Index, Position), m_Index(Index) {}

	// Common
	typename shared_index<_id_t, _size_t, _group_size>::iterator m_It;
	Handle<_index_t> m_Index;
};


template <typename _id_t, typename _size_t, WORD _group_size>
class ConstIndexIterator: public Object
{
private:
	// Using
	using _index_t=Index<_id_t, _size_t, _group_size>;

public:
	// Friends
	friend _index_t;

	// Using
	using FindFunction=Clusters::find_func;

	// Access
	inline _id_t GetCurrent()const { return *m_It; }
	inline BOOL HasCurrent()const { return m_It.has_current(); }

	// Navigation
	inline BOOL Find(_id_t const& Id, FindFunction Function=FindFunction::equal) { return m_It.find(Id, Function); }
	inline BOOL MoveNext() { return m_It.move_next(); }
	inline BOOL MovePrevious() { return m_It.move_previous(); }

private:
	// Con-/Destructors
	ConstIndexIterator(_index_t* Index, _size_t Position): m_It(&Index->m_Index, Position), m_Index(Index) {}

	// Common
	typename shared_index<_id_t, _size_t, _group_size>::const_iterator m_It;
	Handle<_index_t> m_Index;
};

}