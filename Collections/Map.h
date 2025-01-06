//=======
// Map.h
//=======

#pragma once


//=======
// Using
//=======

#include "shared_map.hpp"


//===========
// Namespace
//===========

namespace Collections {


//======================
// Forward-Declarations
//======================

template <typename _key_t, typename _value_t, typename _size_t, WORD _group_size> class MapIterator;
template <typename _key_t, typename _value_t, typename _size_t, WORD _group_size> class ConstMapIterator;


//=====
// Map
//=====

template <typename _key_t, typename _value_t, typename _size_t=UINT, WORD _group_size=10>
class Map: public Object
{
private:
	// Using
	using _map_t=Map<_key_t, _value_t, _size_t, _group_size>;

public:
	// Using
	using FindFunction=find_func;
	using Iterator=MapIterator<_key_t, _value_t, _size_t, _group_size>;
	using ConstIterator=ConstMapIterator<_key_t, _value_t, _size_t, _group_size>;

	// Friends
	friend Iterator;
	friend ConstIterator;

	// Con-/Destructors
	static inline Handle<Map> Create() { return new Map(); }
	static inline Handle<Map> Create(_map_t const* Copy) { return new Map(Copy); }

	// Access
	inline Handle<Iterator> At(UINT Position) { return new Iterator(this, Position); }
	inline Handle<ConstIterator> AtConst(UINT Position) { return new ConstIterator(this, Position); }
	inline Handle<Iterator> Begin() { return new Iterator(this, 0); }
	inline Handle<ConstIterator> BeginConst() { return new ConstIterator(this, 0); }
	inline BOOL Contains(_key_t const& Key) { return m_Map.contains(Key); }
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
	inline Handle<Iterator> Find(_key_t const& Key, FindFunction Function=FindFunction::equal)
		{
		auto it=new Iterator(this, -2);
		it->Find(Key, Function);
		return it;
		}
	inline Handle<ConstIterator> FindConst(_key_t const& Key, FindFunction Function=FindFunction::equal)
		{
		auto it=new ConstIterator(this, -2);
		it->Find(Key, Function);
		return it;
		}
	inline _value_t Get(_key_t const& Key) { return m_Map.get(Key); }
	inline _size_t GetCount() { return m_Map.get_count(); }
	inline BOOL TryGet(_key_t const& Key, _value_t* Value) { return m_Map.try_get(Key, Value); }

	// Modification
	BOOL Add(_key_t const& Key, _value_t const& Value, BOOL Notify=true)
		{
		if(m_Map.add(Key, Value))
			{
			if(Notify)
				{
				Added(this, Key, Value);
				Changed(this);
				}
			return true;
			}
		return false;
		}
	Event<Map, _key_t, _value_t> Added;
	Event<Map> Changed;
	BOOL Clear(BOOL Notify=true)
		{
		if(m_Map.clear())
			{
			if(Notify)
				Changed(this);
			return true;
			}
		return false;
		}
	BOOL Remove(_key_t const& Key, BOOL Notify=true)
		{
		if(!Notify)
			return m_Map.remove(Key);
		auto it=m_Map.find(Key);
		if(!it.has_current())
			return false;
		_value_t value=it->get_value();
		it.remove_current();
		Removed(this, Key, value);
		Changed(this);
		return true;
		}
	BOOL RemoveAt(_size_t Position, BOOL Notify=true)
		{
		if(!Notify)
			return m_Map.remove_at(Position);
		auto it=m_Map.begin(Position);
		if(!it.has_current())
			return false;
		_key_t key=it->get_key();
		_value_t value=it->get_value();
		it.remove_current();
		Removed(this, key, value);
		Changed(this);
		return true;
		}
	Event<Map, _key_t, _value_t> Removed;
	BOOL Set(_key_t const& Key, _value_t const& Value, BOOL Notify=true)
		{
		if(m_Map.set(Key, Value))
			{
			if(Notify)
				Changed(this);
			return true;
			}
		return false;
		}

protected:
	// Con-/Destructors
	Map() {}
	Map(_map_t* Copy)
		{
		if(Copy)
			m_Map.copy_from(Copy->m_Map);
		}

	// Common
	shared_map<_key_t, _value_t, _size_t, _group_size> m_Map;
};


//==========
// Iterator
//==========

template <typename _key_t, typename _value_t, typename _size_t, WORD _group_size>
class MapIterator: public Object
{
private:
	// Using
	using _map_t=Map<_key_t, _value_t, _size_t, _group_size>;

public:
	// Friends
	friend _map_t;

	// Using
	using FindFunction=find_func;

	// Access
	inline _key_t GetKey()const { return m_It->get_key(); }
	inline _value_t GetValue()const { return m_It->get_value(); }
	inline BOOL HasCurrent()const { return m_It.has_current(); }

	// Navigation
	inline BOOL Begin() { return m_It.begin(); }
	inline BOOL End() { return m_It.rbegin(); }
	inline BOOL Find(_key_t const& Key, FindFunction Function=FindFunction::equal) { return m_It.find(Key, Function); }
	inline _size_t GetPosition() { return m_It.get_position(); }
	inline BOOL MoveNext() { return m_It.move_next(); }
	inline BOOL MovePrevious() { return m_It.move_previous(); }
	
	// Modification
	BOOL RemoveCurrent(BOOL Notify=true)
		{
		if(!Notify)
			return m_It.remove_current();
		if(!m_It.has_current())
			return false;
		_key_t id=m_It->get_key();
		_value_t item=m_It->get_value();
		m_It.remove_current();
		m_Map->Removed(m_Map, id, item);
		m_Map->Changed(m_Map);
		return true;
		}
	inline VOID SetValue(_value_t const& Value)
		{
		m_It->set_value(Value);
		}

private:
	// Con-/Destructors
	MapIterator(_map_t* Map, _size_t Position): m_It(&Map->m_Map, Position), m_Map(Map) {}

	// Common
	typename shared_map<_key_t, _value_t, _size_t, _group_size>::iterator m_It;
	Handle<_map_t> m_Map;
};

template <typename _key_t, typename _value_t, typename _size_t, WORD _group_size>
class ConstMapIterator: public Object
{
private:
	// Using
	using _map_t=Map<_key_t, _value_t, _size_t, _group_size>;

public:
	// Friends
	friend _map_t;

	// Using
	using FindFunction=find_func;

	// Access
	inline _key_t GetKey()const { return m_It->get_key(); }
	inline _value_t GetValue()const { return m_It->get_value(); }
	inline BOOL HasCurrent()const { return m_It.has_current(); }

	// Navigation
	inline BOOL Begin() { return m_It.begin(); }
	inline BOOL End() { return m_It.rbegin(); }
	inline BOOL Find(_key_t const& Key, FindFunction Function=FindFunction::equal) { return m_It.find(Key, Function); }
	inline _size_t GetPosition() { return m_It.get_position(); }
	inline BOOL MoveNext() { return m_It.move_next(); }
	inline BOOL MovePrevious() { return m_It.move_previous(); }
	
private:
	// Con-/Destructors
	ConstMapIterator(_map_t* Map, _size_t Position): m_It(&Map->m_Map, Position), m_Map(Map) {}

	// Common
	typename shared_map<_key_t, _value_t, _size_t, _group_size>::const_iterator m_It;
	Handle<_map_t> m_Map;
};

}