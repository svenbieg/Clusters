//=========
// slist.h
//=========

// C-Implementation of a sorted list.
// Items and can be inserted, removed and looked-up in real-time.

// Copyright 2020, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


#ifndef _SLIST_H
#define _SLIST_H


//=======
// Using
//=======

#include <stdbool.h>
#include <stdint.h>


//===========
// Namespace
//===========

#ifdef __cplusplus
extern "C" {
#endif


//==========
// Settings
//==========

#define SLIST_GROUP_SIZE 8

typedef size_t slist_id_t;
typedef size_t slist_value_t;


//======
// Item
//======

typedef struct
{
slist_id_t id;
slist_value_t value;
}slist_item_t;


//=======
// Group
//=======

typedef struct
{
int16_t level;
int16_t child_count;
}slist_group_t;

// Con-/Destructors
void slist_group_destroy(slist_group_t* group);

// Access
slist_item_t* slist_group_get_first_item(slist_group_t* group);
slist_item_t* slist_group_get_item(slist_group_t* group, slist_id_t id);
slist_item_t* slist_group_get_item_at(slist_group_t* group, size_t pos);
size_t slist_group_get_item_count(slist_group_t* group);
slist_item_t* slist_group_get_last_item(slist_group_t* group);

// Modification
bool slist_group_add_item(slist_group_t* group, slist_id_t id, slist_value_t value, bool again);
bool slist_group_remove_item(slist_group_t* group, slist_id_t id);
bool slist_group_remove_item_at(slist_group_t* group, size_t pos);
bool slist_group_set_item(slist_group_t* group, slist_id_t id, slist_value_t value, bool again, bool* exists);


//============
// Item-group
//============

typedef struct slist_item_group_t
{
int16_t level;
int16_t child_count;
slist_item_t items[SLIST_GROUP_SIZE];
}slist_item_group_t;

// Con-/Destructors
slist_item_group_t* slist_item_group_create();

// Access
slist_item_t* slist_item_group_get_first_item(slist_item_group_t* group);
uint16_t slist_item_group_get_insert_pos(slist_item_group_t* group, slist_id_t id, bool* exists);
slist_item_t* slist_item_group_get_item(slist_item_group_t* group, slist_id_t id);
slist_item_t* slist_item_group_get_item_at(slist_item_group_t* group, size_t pos);
int16_t slist_item_group_get_item_pos(slist_item_group_t* group, slist_id_t id);
slist_item_t* slist_item_group_get_last_item(slist_item_group_t* group);

// Modification
bool slist_item_group_add_item(slist_item_group_t* group, slist_id_t id, slist_value_t value);
bool slist_item_group_add_item_internal(slist_item_group_t* group, slist_id_t id, slist_value_t value, uint16_t pos);
void slist_item_group_append_items(slist_item_group_t* group, slist_item_t const* items, uint16_t count);
void slist_item_group_insert_items(slist_item_group_t* group, uint16_t pos, slist_item_t const* items, uint16_t count);
bool slist_item_group_remove_item(slist_item_group_t* group, slist_id_t id);
bool slist_item_group_remove_item_at(slist_item_group_t* group, size_t pos);
void slist_item_group_remove_items(slist_item_group_t* group, uint16_t pos, uint16_t count);
bool slist_item_group_set_item(slist_item_group_t* group, slist_id_t id, slist_value_t value, bool* exists);


//==============
// Parent-group
//==============

typedef struct
{
int16_t level;
int16_t child_count;
slist_item_t* first;
slist_item_t* last;
size_t item_count;
slist_group_t* children[SLIST_GROUP_SIZE];
}slist_parent_group_t;

// Con-/Destructors
slist_parent_group_t* slist_parent_group_create(uint16_t level);
slist_parent_group_t* slist_parent_group_create_with_child(slist_group_t* child);
void slist_parent_group_destroy(slist_parent_group_t* group);

// Access
int16_t slist_parent_group_get_group(slist_parent_group_t* group, size_t* pos);
uint16_t slist_parent_group_get_insert_pos(slist_parent_group_t* group, slist_id_t id, uint16_t* insert_pos);
slist_item_t* slist_parent_group_get_item(slist_parent_group_t* group, slist_id_t id);
slist_item_t* slist_parent_group_get_item_at(slist_parent_group_t* group, size_t pos);
int16_t slist_parent_group_get_item_pos(slist_parent_group_t* group, slist_id_t id);
int16_t slist_parent_group_get_nearest_space(slist_parent_group_t* group, int16_t pos);

// Modification
bool slist_parent_group_add_item(slist_parent_group_t* group, slist_id_t id, slist_value_t value, bool again);
bool slist_parent_group_add_item_internal(slist_parent_group_t* group, slist_id_t id, slist_value_t value, bool again);
void slist_parent_group_append_groups(slist_parent_group_t* group, slist_group_t* const* groups, uint16_t count);
bool slist_parent_group_combine_children(slist_parent_group_t* group, uint16_t pos);
void slist_parent_group_insert_groups(slist_parent_group_t* group, uint16_t pos, slist_group_t* const* groups, uint16_t count);
void slist_parent_group_move_children(slist_parent_group_t* group, uint16_t from, uint16_t to, uint16_t count);
void slist_parent_group_move_space(slist_parent_group_t* group, uint16_t from, uint16_t to);
void slist_parent_group_remove_group(slist_parent_group_t* group, uint16_t pos);
void slist_parent_group_remove_groups(slist_parent_group_t* group, uint16_t pos, uint16_t count);
bool slist_parent_group_remove_item(slist_parent_group_t* group, slist_id_t id);
bool slist_parent_group_remove_item_at(slist_parent_group_t* group, size_t pos);
bool slist_parent_group_set_item(slist_parent_group_t* group, slist_id_t id, slist_value_t value, bool again, bool* exists);
bool slist_parent_group_set_item_internal(slist_parent_group_t* group, slist_id_t id, slist_value_t value, bool again, bool* exists);
bool slist_parent_group_shift_children(slist_parent_group_t* group, uint16_t pos, uint16_t count);
bool slist_parent_group_split_child(slist_parent_group_t* group, uint16_t pos);
void slist_parent_group_update_bounds(slist_parent_group_t* group);


//======
// List
//======

typedef struct
{
slist_group_t* root;
}slist_t;

// Con-/Destructors
void slist_destroy(slist_t* list);
void slist_init(slist_t* list);

// Access
slist_item_t* slist_get_item(slist_t* list, slist_id_t id);
slist_item_t* slist_get_item_at(slist_t* list, size_t pos);
size_t slist_get_item_count(slist_t* list);

// Modification
bool slist_add_item(slist_t* list, slist_id_t id, slist_value_t value);
bool slist_remove_item(slist_t* list, slist_id_t id);
bool slist_remove_item_at(slist_t* list, size_t pos);
bool slist_set_item(slist_t* list, slist_id_t id, slist_value_t value);
void slist_update_root(slist_t* list);


//==========
// Iterator
//==========

typedef struct
{
slist_group_t* group;
uint16_t pos;
}slist_it_ptr_t;

typedef struct
{
slist_item_t* current;
slist_t* list;
uint16_t level_count;
slist_it_ptr_t* pointers;
}slist_it_t;


// Con-/Destructors
void slist_it_init(slist_it_t* it, slist_t* list);
void slist_it_destroy(slist_it_t* it);

// Access
int16_t slist_it_get_child_pos(slist_group_t* group, size_t* pos);
size_t slist_it_get_position(slist_it_t* it);

// Modification
bool slist_it_find(slist_it_t* it, slist_id_t id);
bool slist_it_move_next(slist_it_t* it);
bool slist_it_move_previous(slist_it_t* it);
bool slist_it_remove_current(slist_it_t* it);
bool slist_it_set_level_count(slist_it_t* it, uint16_t level_count);
bool slist_it_set_position(slist_it_t* it, size_t pos);

#ifdef __cplusplus
}
#endif

#endif // _SLIST_H
