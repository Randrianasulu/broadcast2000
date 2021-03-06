#ifndef LINKLIST_H
#define LINKLIST_H

template<class TYPE>
class ListItem;

template<class TYPE>
class List                        // inherited by lists
{
public:
	List();
	virtual ~List();

// the following don't affect *current
	void remove(TYPE *item);   // delete the item and the pointers to it
	void remove_pointer(ListItem<TYPE> *item);  // remove the pointers to the item only

// these must be used to add an item to a list
	TYPE *append();  // create new node and return pointer of it
	TYPE *append(TYPE *new_item);   // append the new pointer to the list
	TYPE *insert_before(TYPE *item);  // create new node and return pointer of it
	TYPE *insert_before(TYPE *item, TYPE *new_item);  // create new node and return pointer of it
	TYPE *insert_after(TYPE *item);
	TYPE *insert_after(TYPE *item, TYPE *new_item);
	void swap(TYPE *item1, TYPE *item2);

// query the list
	int total();     // total number of nodes

// convenience pointers for unthreaded derived classes
	TYPE *current;

// convenience macros
#define PREVIOUS current->previous
#define NEXT current->next

// references to list
	TYPE *first;
	TYPE *last;
};

template<class TYPE>
class ListItem                        // inherited by list items
{
public:
	ListItem();
	virtual ~ListItem();
	
	TYPE *previous;
	TYPE *next;
	List<TYPE> *owner;             // list that owns this item for deletion
};

template<class TYPE>
List<TYPE>::List()
{
	current = last = first = 0;
}

template<class TYPE>
List<TYPE>::~List()     // delete nodes
{
	while(last)
	{
		delete last;
	}
}

template<class TYPE>
int List<TYPE>::total()     // total number of nodes
{
	int total = 0;
	TYPE* current;
	
	for(current = first; current; current = NEXT)
	{
		total++;
	}
	return total;
}

template<class TYPE>
TYPE* List<TYPE>::append()
{
	TYPE* current_item;
	
	if(!last)        // add first node
	{
		current_item = last = first = current = new TYPE;
		current_item->previous = current_item->next = 0;
		current_item->owner = this;
		return current_item;
	}
	else
	{                // append node
		current_item = last->next = current = new TYPE;
		current_item->previous = last;
		current_item->next = 0;
		last = current_item;
		current_item->owner = this;
		return current_item;
	}
}

template<class TYPE>
TYPE* List<TYPE>::append(TYPE *new_item)
{
	TYPE* current_item;
	
	if(!last)        // add first node
	{
		current_item = last = first = current = new_item;
		current_item->previous = current_item->next = 0;
		current_item->owner = this;
		return current_item;
	}
	else
	{                // append node
		current_item = last->next = current = new_item;
		current_item->previous = last;
		current_item->next = 0;
		last = current_item;
		current_item->owner = this;
		return current_item;
	}
}

template<class TYPE>
TYPE* List<TYPE>::insert_before(TYPE *item)
{
	TYPE* new_item = new TYPE;
	return insert_before(item, new_item);
}

template<class TYPE>
TYPE* List<TYPE>::insert_before(TYPE *item, TYPE *new_item)
{
	if(!item) return append(new_item);      // if item is null, append

	TYPE* current_item = current = new_item;

	if(item == first) first = current_item;   // set *first

	current_item->previous = item->previous;       // set this node's pointers
	current_item->next = item;
	
	if(current_item->previous) current_item->previous->next = current_item;         // set previous node's pointers

	if(current_item->next) current_item->next->previous = current_item;        // set next node's pointers
	
	current_item->owner = this;
	return current_item;
}

template<class TYPE>
TYPE* List<TYPE>::insert_after(TYPE *item)
{
	TYPE *new_item = new TYPE;
	return insert_after(item, new_item);
}

template<class TYPE>
TYPE* List<TYPE>::insert_after(TYPE *item, TYPE *new_item)
{
	if(!item) return append(new_item);      // if item is null, append

	TYPE* current_item = current = new_item;

	if(item == last) last = current_item;   // set *last

	current_item->previous = item;       // set this node's pointers
	current_item->next = item->next;
	
	if(current_item->previous) current_item->previous->next = current_item;         // set previous node's pointers

	if(current_item->next) current_item->next->previous = current_item;        // set next node's pointers
	
	current_item->owner = this;
	return current_item;
}

template<class TYPE>
void List<TYPE>::swap(TYPE *item1, TYPE *item2)
{
	TYPE *new_item0, *new_item1, *new_item2, *new_item3;

// old == item0 item1 item2 item3
// new == item0 item2 item1 item3

	new_item0 = item1->previous;
	new_item1 = item2;
	new_item2 = item1;
	new_item3 = item2->next;

	if(new_item0) 
		new_item0->next = new_item1;
	else
		first = new_item1;

	if(new_item1) new_item1->next = new_item2;
	if(new_item2) new_item2->next = new_item3;
	if(new_item3)
		new_item3->previous = new_item2;
	else
		last = new_item2;

	if(new_item2) new_item2->previous = new_item1;
	if(new_item1) new_item1->previous = new_item0;
}

template<class TYPE>
void List<TYPE>::remove(TYPE *item)
{
	if(!item) return;
	delete item;                        // item calls back to remove pointers
}

template<class TYPE>
void List<TYPE>::remove_pointer(ListItem<TYPE> *item)
{
//printf("List<TYPE>::remove_pointer %x %x %x\n", item, last, first);
	if(!item) return;

	if(item == last && item == first)
	{
// last item
		last = first = 0;
		return;
	}

	if(item == last) last = item->previous;      // set *last and *first
	else
	if(item == first) first = item->next;

	if(item->previous) item->previous->next = item->next;         // set previous node's pointers

	if(item->next) item->next->previous = item->previous;       // set next node's pointers
}

template<class TYPE>
ListItem<TYPE>::ListItem()
{
// don't delete the pointer to this if it's not part of a list
	owner = 0;
}

template<class TYPE>
ListItem<TYPE>::~ListItem()
{
// don't delete the pointer to this if it's not part of a list
	if(owner) owner->remove_pointer(this);
}

#endif
