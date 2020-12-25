//=========
// slist.c
//=========

// C-Implementation of a sorted list.
// Items and can be inserted, removed and looked-up in real-time.

// Copyright 2020, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/clusters


//=======
// Using
//=======

#include <malloc.h>
#include "slist.h"


//=======
// Group
//=======

// Con-/Destructors
void slist_group_destroy(slist_group_t* group)
{
if(group->level==0)
	{
	free(group);
	return;
	}
return slist_parent_group_destroy((slist_parent_group_t*)group);
}


// Access

slist_item_t* slist_group_get_first_item(slist_group_t* group)
{
if(group->level==0)
	return slist_item_group_get_first_item((slist_item_group_t*)group);
slist_parent_group_t* pgroup=(slist_parent_group_t*)group;
return pgroup->first;
}

slist_item_t* slist_group_get_item(slist_group_t* group, slist_id_t id)
{
if(group->level==0)
	return slist_item_group_get_item((slist_item_group_t*)group, id);
return slist_parent_group_get_item((slist_parent_group_t*)group, id);
}

slist_item_t* slist_group_get_item_at(slist_group_t* group, size_t pos)
{
if(group->level==0)
	return slist_item_group_get_item_at((slist_item_group_t*)group, pos);
return slist_parent_group_get_item_at((slist_parent_group_t*)group, pos);
}

size_t slist_group_get_item_count(slist_group_t* group)
{
if(group->level==0)
	return group->child_count;
slist_parent_group_t* pgroup=(slist_parent_group_t*)group;
return pgroup->item_count;
}

slist_item_t* slist_group_get_last_item(slist_group_t* group)
{
if(group->level==0)
	return slist_item_group_get_last_item((slist_item_group_t*)group);
slist_parent_group_t* pgroup=(slist_parent_group_t*)group;
return pgroup->last;
}


// Modification

bool slist_group_add_item(slist_group_t* group, slist_id_t id, slist_value_t value, bool again)
{
if(group->level==0)
	return slist_item_group_add_item((slist_item_group_t*)group, id, value);
return slist_parent_group_add_item((slist_parent_group_t*)group, id, value, again);
}

bool slist_group_remove_item(slist_group_t* group, slist_id_t id)
{
if(group->level==0)
	return slist_item_group_remove_item((slist_item_group_t*)group, id);
return slist_parent_group_remove_item((slist_parent_group_t*)group, id);
}

bool slist_group_remove_item_at(slist_group_t* group, size_t pos)
{
if(group->level==0)
	return slist_item_group_remove_item_at((slist_item_group_t*)group, pos);
return slist_parent_group_remove_item_at((slist_parent_group_t*)group, pos);
}

bool slist_group_set_item(slist_group_t* group, slist_id_t id, slist_value_t value, bool again, bool* exists)
{
if(group->level==0)
	return slist_item_group_set_item((slist_item_group_t*)group, id, value, exists);
return slist_parent_group_set_item((slist_parent_group_t*)group, id, value, again, exists);
}


//============
// Item-group
//============

// Con-/Destructors

slist_item_group_t* slist_item_group_create()
{
slist_item_group_t* group=(slist_item_group_t*)malloc(sizeof(slist_item_group_t));
if(group==NULL)
	return NULL;
group->level=0;
group->child_count=0;
return group;
}


// Access

slist_item_t* slist_item_group_get_first_item(slist_item_group_t* group)
{
if(group->child_count==0)
	return NULL;
return group->items;
}

uint16_t slist_item_group_get_insert_pos(slist_item_group_t* group, slist_id_t id, bool* exists)
{
uint16_t start=0;
uint16_t end=group->child_count;
while(start<end)
	{
	uint16_t pos=start+(end-start)/2;
	if(group->items[pos].id>id)
		{
		end=pos;
		continue;
		}
	if(group->items[pos].id<id)
		{
		start=pos+1;
		continue;
		}
	*exists=true;
	return pos;
	}
return end;
}

slist_item_t* slist_item_group_get_item(slist_item_group_t* group, slist_id_t id)
{
int16_t pos=slist_item_group_get_item_pos(group, id);
if(pos<0)
	return NULL;
return &group->items[pos];
}

slist_item_t* slist_item_group_get_item_at(slist_item_group_t* group, size_t pos)
{
if(pos>=group->child_count)
	return NULL;
return &group->items[pos];
}

int16_t slist_item_group_get_item_pos(slist_item_group_t* group, slist_id_t id)
{
uint16_t start=0;
uint16_t end=group->child_count;
uint16_t pos=0;
slist_item_t* item=NULL;
while(start<end)
	{
	pos=start+(end-start)/2;
	item=&group->items[pos];
	if(item->id>id)
		{
		end=pos;
		continue;
		}
	if(item->id<id)
		{
		start=pos+1;
		continue;
		}
	return pos;
	}
return -(int16_t)pos-1;
}

slist_item_t* slist_item_group_get_last_item(slist_item_group_t* group)
{
uint16_t count=group->child_count;
if(count==0)
	return NULL;
return &group->items[count-1];
}


// Modification

bool slist_item_group_add_item(slist_item_group_t* group, slist_id_t id, slist_value_t value)
{
bool exists=false;
uint16_t pos=slist_item_group_get_insert_pos(group, id, &exists);
return slist_item_group_add_item_internal(group, id, value, pos);
}

bool slist_item_group_add_item_internal(slist_item_group_t* group, slist_id_t id, slist_value_t value, uint16_t pos)
{
uint16_t child_count=group->child_count;
if(child_count==SLIST_GROUP_SIZE)
	return false;
for(uint16_t u=child_count; u>pos; u--)
	group->items[u]=group->items[u-1];
group->items[pos].id=id;
group->items[pos].value=value;
group->child_count++;
return true;
}

void slist_item_group_append_items(slist_item_group_t* group, slist_item_t const* items, uint16_t count)
{
uint16_t child_count=group->child_count;
for(uint16_t u=0; u<count; u++)
	group->items[child_count+u]=items[u];
group->child_count+=count;
}

void slist_item_group_insert_items(slist_item_group_t* group, uint16_t pos, slist_item_t const* items, uint16_t count)
{
uint16_t child_count=group->child_count;
for(uint16_t u=child_count+count-1; u>=pos+count; u--)
	group->items[u]=group->items[u-count];
for(uint16_t u=0; u<count; u++)
	group->items[pos+u]=items[u];
group->child_count+=count;
}

bool slist_item_group_remove_item(slist_item_group_t* group, slist_id_t id)
{
int16_t pos=slist_item_group_get_item_pos(group, id);
if(pos<0)
	return false;
return slist_item_group_remove_item_at(group, pos);
}

bool slist_item_group_remove_item_at(slist_item_group_t* group, size_t pos)
{
uint16_t child_count=group->child_count;
if(pos>=child_count)
	return false;
for(size_t u=pos; u+1<child_count; u++)
	group->items[u]=group->items[u+1];
group->child_count--;
return true;
}

void slist_item_group_remove_items(slist_item_group_t* group, uint16_t pos, uint16_t count)
{
uint16_t child_count=group->child_count;
for(uint16_t u=pos; u+count<child_count; u++)
	group->items[u]=group->items[u+count];
group->child_count-=count;
}

bool slist_item_group_set_item(slist_item_group_t* group, slist_id_t id, slist_value_t value, bool *exists)
{
uint16_t pos=slist_item_group_get_insert_pos(group, id, exists);
if(*exists)
	{
	group->items[pos].value=value;
	return true;
	}
return slist_item_group_add_item_internal(group, id, value, pos);
}


//==============
// Parent-group
//==============

// Con-/Destructors

slist_parent_group_t* slist_parent_group_create(uint16_t level)
{
slist_parent_group_t* group=(slist_parent_group_t*)malloc(sizeof(slist_parent_group_t));
if(group==NULL)
	return NULL;
group->level=level;
group->child_count=0;
group->first=NULL;
group->last=NULL;
group->item_count=0;
return group;
}

slist_parent_group_t* slist_parent_group_create_with_child(slist_group_t* child)
{
slist_parent_group_t* group=(slist_parent_group_t*)malloc(sizeof(slist_parent_group_t));
if(group==NULL)
	return NULL;
group->level=child->level+1;
group->child_count=1;
group->first=slist_group_get_first_item(child);
group->last=slist_group_get_last_item(child);
group->item_count=slist_group_get_item_count(child);
group->children[0]=child;
return group;
}

void slist_parent_group_destroy(slist_parent_group_t* group)
{
uint16_t child_count=group->child_count;
for(uint16_t u=0; u<child_count; u++)
	slist_group_destroy(group->children[u]);
free(group);
}


// Access

int16_t slist_parent_group_get_group(slist_parent_group_t* group, size_t* pos)
{
uint16_t child_count=group->child_count;
for(uint16_t u=0; u<child_count; u++)
	{
	size_t item_count=slist_group_get_item_count(group->children[u]);
	if(*pos<item_count)
		return u;
	*pos-=item_count;
	}
return -1;
}

uint16_t slist_parent_group_get_insert_pos(slist_parent_group_t* group, slist_id_t id, uint16_t* insert_pos)
{
uint16_t child_count=group->child_count;
if(!child_count)
	return 0;
uint16_t start=0;
uint16_t end=child_count;
slist_item_t* first=NULL;
slist_item_t* last=NULL;
while(start<end)
	{
	uint16_t pos=start+(end-start)/2;
	first=slist_group_get_first_item(group->children[pos]);
	last=slist_group_get_last_item(group->children[pos]);
	if(first->id>id)
		{
		end=pos;
		continue;
		}
	if(last->id<id)
		{
		start=pos+1;
		continue;
		}
	start=pos;
	break;
	}
if(start>child_count-1)
	start=child_count-1;
*insert_pos=start;
if(start>0)
	{
	first=slist_group_get_first_item(group->children[start]);
	if(first->id>=id)
		{
		*insert_pos=start-1;
		return 2;
		}
	}
if(start+1<child_count)
	{
	last=slist_group_get_last_item(group->children[start]);
	if(last->id<=id)
		return 2;
	}
return 1;
}

slist_item_t* slist_parent_group_get_item(slist_parent_group_t* group, slist_id_t id)
{
int16_t child=slist_parent_group_get_item_pos(group, id);
if(child<0)
	return NULL;
return slist_group_get_item(group->children[child], id);
}

slist_item_t* slist_parent_group_get_item_at(slist_parent_group_t* group, size_t pos)
{
int16_t child=slist_parent_group_get_group(group, &pos);
if(child<0)
	return NULL;
return slist_group_get_item_at(group->children[child], pos);
}

int16_t slist_parent_group_get_item_pos(slist_parent_group_t* group, slist_id_t id)
{
uint16_t start=0;
uint16_t end=group->child_count;
uint16_t pos=0;
slist_item_t* first=NULL;
slist_item_t* last=NULL;
while(start<end)
	{
	pos=start+(end-start)/2;
	first=slist_group_get_first_item(group->children[pos]);
	if(first->id>id)
		{
		end=pos;
		continue;
		}
	last=slist_group_get_last_item(group->children[pos]);
	if(last->id<id)
		{
		start=pos+1;
		continue;
		}
	return pos;
	}
return -(int16_t)pos-1;
}

int16_t slist_parent_group_get_nearest_space(slist_parent_group_t* group, int16_t pos)
{
int16_t child_count=(int16_t)group->child_count;
int16_t before=pos-1;
int16_t after=pos+1;
while(before>=0||after<child_count)
	{
	if(before>=0)
		{
		uint16_t count=group->children[before]->child_count;
		if(count<SLIST_GROUP_SIZE)
			return before;
		before--;
		}
	if(after<child_count)
		{
		uint16_t count=group->children[after]->child_count;
		if(count<SLIST_GROUP_SIZE)
			return after;
		after++;
		}
	}
return -1;
}


// Modification

bool slist_parent_group_add_item(slist_parent_group_t* group, slist_id_t id, slist_value_t value, bool again)
{
if(slist_parent_group_add_item_internal(group, id, value, again))
	{
	group->item_count++;
	slist_parent_group_update_bounds(group);
	return true;
	}
return false;
}

bool slist_parent_group_add_item_internal(slist_parent_group_t* group, slist_id_t id, slist_value_t value, bool again)
{
uint16_t pos=0;
uint16_t count=slist_parent_group_get_insert_pos(group, id, &pos);
if(!again)
	{
	for(uint16_t u=0; u<count; u++)
		{
		if(slist_group_add_item(group->children[pos+u], id, value, false))
			return true;
		}
	if(slist_parent_group_shift_children(group, pos, count))
		{
		count=slist_parent_group_get_insert_pos(group, id, &pos);
		for(uint16_t u=0; u<count; u++)
			{
			if(slist_group_add_item(group->children[pos+u], id, value, false))
				return true;
			}
		}
	}
if(!slist_parent_group_split_child(group, pos))
	return false;
count=slist_parent_group_get_insert_pos(group, id, &pos);
for(uint16_t u=0; u<count; u++)
	{
	if(slist_group_add_item(group->children[pos+u], id, value, true))
		return true;
	}
return false;
}

void slist_parent_group_append_groups(slist_parent_group_t* group, slist_group_t* const* groups, uint16_t count)
{
uint16_t child_count=group->child_count;
for(uint16_t u=0; u<count; u++)
	{
	group->children[child_count+u]=groups[u];
	group->item_count+=slist_group_get_item_count(groups[u]);
	}
group->child_count+=count;
slist_parent_group_update_bounds(group);
}

bool slist_parent_group_combine_children(slist_parent_group_t* group, uint16_t pos)
{
uint16_t count=group->children[pos]->child_count;
if(count==0)
	{
	slist_parent_group_remove_group(group, pos);
	return true;
	}
if(pos>0)
	{
	uint16_t before=group->children[pos-1]->child_count;
	if(count+before<=SLIST_GROUP_SIZE)
		{
		slist_parent_group_move_children(group, pos, pos-1, count);
		slist_parent_group_remove_group(group, pos);
		return true;
		}
	}
uint16_t child_count=group->child_count;
if(pos+1<child_count)
	{
	uint16_t after=group->children[pos+1]->child_count;
	if(count+after<=SLIST_GROUP_SIZE)
		{
		slist_parent_group_move_children(group, pos+1, pos, count);
		slist_parent_group_remove_group(group, pos+1);
		return true;
		}
	}
return false;
}

void slist_parent_group_insert_groups(slist_parent_group_t* group, uint16_t pos, slist_group_t* const* groups, uint16_t count)
{
uint16_t child_count=group->child_count;
for(uint16_t u=child_count+count-1; u>=pos+count; u--)
	group->children[u]=group->children[u-count];
for(uint16_t u=0; u<count; u++)
	{
	group->children[pos+u]=groups[u];
	group->item_count+=slist_group_get_item_count(groups[u]);
	}
group->child_count+=count;
slist_parent_group_update_bounds(group);
}

void slist_parent_group_move_children(slist_parent_group_t* group, uint16_t from, uint16_t to, uint16_t count)
{
uint16_t level=group->level;
if(level>1)
	{
	slist_parent_group_t* src=(slist_parent_group_t*)group->children[from];
	slist_parent_group_t* dst=(slist_parent_group_t*)group->children[to];
	if(from>to)
		{
		slist_parent_group_append_groups(dst, src->children, count);
		slist_parent_group_remove_groups(src, 0, count);
		}
	else
		{
		uint16_t src_count=src->child_count;
		slist_parent_group_insert_groups(dst, 0, &src->children[src_count-count], count);
		slist_parent_group_remove_groups(src, src_count-count, count);
		}
	}
else
	{
	slist_item_group_t* src=(slist_item_group_t*)group->children[from];
	slist_item_group_t* dst=(slist_item_group_t*)group->children[to];
	if(from>to)
		{
		slist_item_group_append_items(dst, src->items, count);
		slist_item_group_remove_items(src, 0, count);
		}
	else
		{
		uint16_t src_count=src->child_count;
		slist_item_group_insert_items(dst, 0, &src->items[src_count-count], count);
		slist_item_group_remove_items(src, src_count-count, count);
		}
	}
}

void slist_parent_group_move_space(slist_parent_group_t* group, uint16_t from, uint16_t to)
{
if(from<to)
	{
	for(uint16_t u=from; u<to; u++)
		slist_parent_group_move_children(group, u+1, u, 1);
	}
else
	{
	for(uint16_t u=from; u>to; u--)
		slist_parent_group_move_children(group, u-1, u, 1);
	}
}

void slist_parent_group_remove_group(slist_parent_group_t* group, uint16_t pos)
{
uint16_t child_count=group->child_count;
slist_group_t* child=group->children[pos];
for(uint16_t u=pos; u+1<child_count; u++)
	group->children[u]=group->children[u+1];
group->child_count--;
free(child);
}

void slist_parent_group_remove_groups(slist_parent_group_t* group, uint16_t pos, uint16_t count)
{
for(uint16_t u=0; u<count; u++)
	group->item_count-=slist_group_get_item_count(group->children[pos+u]);
uint16_t child_count=group->child_count;
for(uint16_t u=pos; u+count<child_count; u++)
	group->children[u]=group->children[u+count];
group->child_count-=count;
slist_parent_group_update_bounds(group);
}

bool slist_parent_group_remove_item(slist_parent_group_t* group, slist_id_t id)
{
int16_t pos=slist_parent_group_get_item_pos(group, id);
if(pos<0)
	return false;
if(!slist_group_remove_item(group->children[pos], id))
	return false;
group->item_count--;
slist_parent_group_combine_children(group, pos);
slist_parent_group_update_bounds(group);
return true;
}

bool slist_parent_group_remove_item_at(slist_parent_group_t* group, size_t pos)
{
if(pos>=group->item_count)
	return false;
int16_t child=slist_parent_group_get_group(group, &pos);
if(child<0)
	return false;
slist_group_remove_item_at(group->children[child], pos);
group->item_count--;
slist_parent_group_combine_children(group, child);
slist_parent_group_update_bounds(group);
return true;
}

bool slist_parent_group_set_item(slist_parent_group_t* group, slist_id_t id, slist_value_t value, bool again, bool* exists)
{
if(slist_parent_group_set_item_internal(group, id, value, again, exists))
	{
	if(!(*exists))
		{
		group->item_count++;
		slist_parent_group_update_bounds(group);
		}
	return true;
	}
return false;
}

bool slist_parent_group_set_item_internal(slist_parent_group_t* group, slist_id_t id, slist_value_t value, bool again, bool* exists)
{
int16_t pos=slist_parent_group_get_item_pos(group, id);
if(pos>=0)
	{
	if(slist_group_set_item(group->children[pos], id, value, again, exists))
		return true;
	}
return slist_parent_group_add_item_internal(group, id, value, again);
}

bool slist_parent_group_shift_children(slist_parent_group_t* group, uint16_t pos, uint16_t count)
{
int16_t space=slist_parent_group_get_nearest_space(group, pos);
if(space<0)
	return false;
if(count>1&&space>pos)
	pos++;
slist_parent_group_move_space(group, space, pos);
return true;
}

bool slist_parent_group_split_child(slist_parent_group_t* group, uint16_t pos)
{
uint16_t child_count=group->child_count;
if(child_count==SLIST_GROUP_SIZE)
	return false;
slist_group_t* child=NULL;
uint16_t level=group->level;
if(level>1)
	{
	child=(slist_group_t*)slist_parent_group_create(level-1);
	}
else
	{
	child=(slist_group_t*)slist_item_group_create();
	}
if(!child)
	return false;
for(uint16_t u=child_count; u>pos+1; u--)
	group->children[u]=group->children[u-1];
group->children[pos+1]=child;
group->child_count++;
slist_parent_group_move_children(group, pos, pos+1, 1);
return true;
}

void slist_parent_group_update_bounds(slist_parent_group_t* group)
{
uint16_t child_count=group->child_count;
if(child_count==0)
	{
	group->first=NULL;
	group->last=NULL;
	return;
	}
group->first=slist_group_get_first_item(group->children[0]);
group->last=slist_group_get_last_item(group->children[child_count-1]);
}


//======
// List
//======

// Con-/Destructors

void slist_destroy(slist_t* list)
{
slist_group_t* root=list->root;
if(!root)
	return;
list->root=NULL;
slist_group_destroy(root);
}

void slist_init(slist_t* list)
{
list->root=NULL;
}


// Access

slist_item_t* slist_get_item(slist_t* list, slist_id_t id)
{
if(!list->root)
	return NULL;
return slist_group_get_item(list->root, id);
}

slist_item_t* slist_get_item_at(slist_t* list, size_t pos)
{
if(!list->root)
	return NULL;
return slist_group_get_item_at(list->root, pos);
}

size_t slist_get_item_count(slist_t* list)
{
if(!list->root)
	return 0;
return slist_group_get_item_count(list->root);
}


// Modification

bool slist_add_item(slist_t* list, slist_id_t id, slist_value_t value)
{
if(!list->root)
	{
	list->root=(slist_group_t*)slist_item_group_create();
	if(!list->root)
		return false;
	}
if(slist_group_add_item(list->root, id, value, false))
	return true;
slist_parent_group_t* root=slist_parent_group_create_with_child(list->root);
if(!root)
	return false;
list->root=(slist_group_t*)root;
if(slist_parent_group_add_item(root, id, value, true))
	return true;
return false;
}

bool slist_remove_item(slist_t* list, slist_id_t id)
{
if(!list->root)
	return false;
if(slist_group_remove_item(list->root, id))
	{
	slist_update_root(list);
	return true;
	}
return false;
}

bool slist_remove_item_at(slist_t* list, size_t pos)
{
if(!list->root)
	return false;
if(slist_group_remove_item_at(list->root, pos))
	{
	slist_update_root(list);
	return true;
	}
return false;
}

bool slist_set_item(slist_t* list, slist_id_t id, slist_value_t value)
{
if(!list->root)
	{
	list->root=(slist_group_t*)slist_item_group_create();
	if(!list->root)
		return false;
	}
bool exists=false;
if(slist_group_set_item(list->root, id, value, false, &exists))
	return true;
slist_parent_group_t* root=slist_parent_group_create_with_child(list->root);
if(!root)
	return false;
list->root=(slist_group_t*)root;
return slist_parent_group_set_item(root, id, value, true, &exists);
}

void slist_update_root(slist_t* list)
{
slist_group_t* root=list->root;
uint16_t level=root->level;
uint16_t child_count=root->child_count;
if(level==0)
	{
	if(child_count==0)
		{
		list->root=NULL;
		free(root);
		}
	return;
	}
if(child_count>1)
	return;
slist_parent_group_t* proot=(slist_parent_group_t*)root;
list->root=proot->children[0];
root->child_count=0;
free(root);
}


//==========
// Iterator
//==========

// Con-/Destructors

void slist_it_init(slist_it_t* it, slist_t* list)
{
it->current=NULL;
it->list=list;
it->level_count=0;
it->pointers=NULL;
}

void slist_it_destroy(slist_it_t* it)
{
if(it->pointers)
	{
	free(it->pointers);
	it->pointers=NULL;
	}
it->current=NULL;
it->list=NULL;
it->level_count=0;
}


// Access

int16_t slist_it_get_child_pos(slist_group_t* group, size_t* pos)
{
uint16_t child_count=group->child_count;
uint16_t level=group->level;
if(level==0)
	{
	if(*pos>=child_count)
		return -1;
	return *pos;
	}
slist_parent_group_t* pgroup=(slist_parent_group_t*)group;
for(uint16_t u=0; u<child_count; u++)
	{
	slist_group_t* child=pgroup->children[u];
	size_t item_count=slist_group_get_item_count(child);
	if(*pos<item_count)
		return u;
	*pos-=item_count;
	}
return -1;
}

size_t slist_it_get_position(slist_it_t* it)
{
if(!it->current)
	return -1;
uint16_t level_count=it->level_count;
size_t pos=0;
for(uint16_t level=0; level<level_count-1; level++)
	{
	slist_parent_group_t* group=(slist_parent_group_t*)it->pointers[level].group;
	uint16_t group_pos=it->pointers[level].pos;
	for(uint16_t child=0; child<group_pos; child++)
		pos+=slist_group_get_item_count(group->children[child]);
	}
pos+=it->pointers[level_count-1].pos;
return pos;
}


// Modification

bool slist_it_find(slist_it_t* it, slist_id_t id)
{
it->current=NULL;
bool found=true;
slist_group_t* group=it->list->root;
if(!group)
	return false;
uint16_t level_count=group->level+1;
if(!slist_it_set_level_count(it, level_count))
	return false;
for(uint16_t level=0; level<level_count-1; level++)
	{
	slist_parent_group_t* pgroup=(slist_parent_group_t*)group;
	int16_t child=slist_parent_group_get_item_pos(pgroup, id);
	if(child<0)
		{
		found=false;
		child++;
		child*=-1;
		}
	uint16_t pos=(uint16_t)child;
	it->pointers[level].group=group;
	it->pointers[level].pos=pos;
	group=pgroup->children[pos];
	}
slist_item_group_t* igroup=(slist_item_group_t*)group;
int16_t child=slist_item_group_get_item_pos(igroup, id);
if(child<0)
	{
	found=false;
	child++;
	child*=-1;
	}
uint16_t pos=(uint16_t)child;
it->pointers[level_count-1].group=group;
it->pointers[level_count-1].pos=pos;
it->current=slist_item_group_get_item_at(igroup, pos);
return found;
}

bool slist_it_move_next(slist_it_t* it)
{
if(!it->current)
	return false;
uint16_t level_count=it->level_count;
slist_it_ptr_t* ptr=&it->pointers[level_count-1];
slist_item_group_t* igroup=(slist_item_group_t*)ptr->group;
uint16_t child_count=ptr->group->child_count;
if(ptr->pos+1<child_count)
	{
	ptr->pos++;
	it->current=slist_item_group_get_item_at(igroup, ptr->pos);
	return true;
	}
for(uint16_t level=level_count-1; level>0; level--)
	{
	ptr=&it->pointers[level-1];
	slist_parent_group_t* pgroup=(slist_parent_group_t*)ptr->group;
	child_count=ptr->group->child_count;
	if(ptr->pos+1>=child_count)
		continue;
	ptr->pos++;
	slist_group_t* group=ptr->group;
	for(; level<level_count; level++)
		{
		pgroup=(slist_parent_group_t*)group;
		group=pgroup->children[ptr->pos];
		ptr=&it->pointers[level];
		ptr->group=group;
		ptr->pos=0;
		}
	igroup=(slist_item_group_t*)group;
	it->current=slist_item_group_get_item_at(igroup, 0);
	return true;
	}
it->current=NULL;
return false;
}

bool slist_it_move_previous(slist_it_t* it)
{
if(!it->current)
	return false;
uint16_t level_count=it->level_count;
slist_it_ptr_t* ptr=&it->pointers[level_count-1];
slist_item_group_t* igroup=(slist_item_group_t*)ptr->group;
if(ptr->pos>0)
	{
	ptr->pos--;
	it->current=slist_item_group_get_item_at(igroup, ptr->pos);
	return true;
	}
for(uint16_t level=level_count-1; level>0; level--)
	{
	ptr=&it->pointers[level-1];
	slist_parent_group_t* pgroup=(slist_parent_group_t*)ptr->group;
	if(ptr->pos==0)
		continue;
	ptr->pos--;
	slist_group_t* group=ptr->group;
	uint16_t pos=0;
	for(; level<level_count; level++)
		{
		pgroup=(slist_parent_group_t*)group;
		group=pgroup->children[ptr->pos];
		pos=group->child_count-1;
		ptr=&it->pointers[level];
		ptr->group=group;
		ptr->pos=pos;
		}
	igroup=(slist_item_group_t*)group;
	it->current=slist_item_group_get_item_at(igroup, pos);
	return true;
	}
it->current=NULL;
return false;
}

bool slist_it_remove_current(slist_it_t* it)
{
if(!it->current)
	return false;
size_t pos=slist_it_get_position(it);
if(!slist_remove_item_at(it->list, pos))
	return false;
slist_it_set_position(it, pos);
return true;
}

bool slist_it_set_level_count(slist_it_t* it, uint16_t level_count)
{
it->current=NULL;
if(!level_count)
	return false;
if(it->level_count==level_count)
	return true;
if(it->pointers)
	free(it->pointers);
it->pointers=(slist_it_ptr_t*)malloc(level_count*sizeof(slist_it_ptr_t));
if(it->pointers)
	{
	it->level_count=level_count;
	return true;
	}
it->level_count=0;
return false;
}

bool slist_it_set_position(slist_it_t* it, size_t pos)
{
it->current=NULL;
slist_group_t* group=it->list->root;
if(!group)
	return false;
uint16_t level_count=group->level+1;
if(!slist_it_set_level_count(it, level_count))
	return false;
int16_t child=slist_it_get_child_pos(group, &pos);
if(child<0)
	return false;
it->pointers[0].group=group;
it->pointers[0].pos=child;
for(uint16_t level=0; level<level_count-1; level++)
	{
	slist_parent_group_t* pgroup=(slist_parent_group_t*)it->pointers[level].group;
	group=pgroup->children[child];
	child=slist_it_get_child_pos(group, &pos);
	if(child<0)
		return false;
	it->pointers[level+1].group=group;
	it->pointers[level+1].pos=child;
	}
if((uint16_t)child<group->child_count)
	{
	slist_item_group_t* igroup=(slist_item_group_t*)group;
	it->current=slist_item_group_get_item_at(igroup, child);
	return true;
	}
return false;
}
