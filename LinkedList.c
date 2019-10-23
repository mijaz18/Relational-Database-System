

#include <stdio.h>
#include <stdlib.h>
#include "LinkedList.h"
#include <string.h>

/**
 * Structure for each element of a doubly-linked LinkedList.
 */
typedef struct Node* Node;
struct Node {
	void *data;
	Node next;
	Node prev;
};

/**
 * Linked list with first and last (head and tail) pointers.
 */
struct LinkedList {
	Node first;
	Node last;
};


/**
 * Allocate, initialize and return a new (empty) LinkedList.
 */
LinkedList new_LinkedList() {
	LinkedList this = (LinkedList)malloc(sizeof(struct LinkedList));
	this->first = this->last = NULL;
	return this;
}

LinkedList generateLinkedListFromString(char* s) {
	LinkedList this = new_LinkedList();
	int i=0;
	while (i < strlen(s)) {
		int startSchema = i;
		int endSchema = i + 1;
		while (s[endSchema] != ',' && endSchema != strlen(s)) {
			endSchema++;
		}
		char* attr = (char*) calloc(10, sizeof(char));
		strncpy(attr, s + startSchema, endSchema - startSchema);
		attr[endSchema - startSchema] = '\0';
		LinkedList_add_at_end(this, attr);
		i = endSchema + 1;
	}
	return this;
}


LinkedList LinkedList_union(LinkedList l1, LinkedList l2){
   LinkedList unionList = new_LinkedList();
    for (int i=0; i<LinkedList_size(l1); i++) {
        LinkedList_add_at_end(unionList, LinkedList_elementAt(l1,i));
    }
    for (int i=0; i<LinkedList_size(l2); i++) {
        LinkedList_add_at_end(unionList, LinkedList_elementAt(l2,i));
    }
    return unionList;
}

static Node new_Node(void *data) {
	Node this = (Node)malloc(sizeof(struct Node));
	this->data = data;
	this->next = this->prev = NULL;
	return this;
}

/**
 * Free the memory used for the given LinkedList.
 * If boolean free_data_also is true, also free the data associated with
 * each element.
 */
void LinkedList_free(LinkedList this, bool free_data_also) {
	if (this == NULL) {
		return;
	}
	// Free the elements
	Node node = this->first;
	while (node != NULL) {
		Node next = node->next;
		if (free_data_also && node->data != NULL) {
			free(node->data);
		}
		free(node);
		node = next;
	}
	// Free the list itself
	free(this);
}

/**
 * Return true if the given LinkedList is empty.
 */
bool LinkedList_isEmpty(const LinkedList this) {
	return this->first == NULL;
}

/**
 * Add the given void* value at the front of the given LinkedList.
 */
void LinkedList_add_at_front(LinkedList this, void *data) {
	Node node = new_Node(data);
	node->next = this->first;
	if (this->first != NULL) {
		this->first->prev = node;
	}
	this->first = node;
	if (this->last == NULL) {
		this->last = node;
	}
}

/**
 * Add the given void* value at the end of the given LinkedList.
 */
void LinkedList_add_at_end(LinkedList this, void *data) {
	Node node = new_Node(data);
	node->prev = this->last;
	if (this->last != NULL) {
		this->last->next = node;
	}
	this->last = node;
	if (this->first == NULL) {
		this->first = node;
	}
}

/**
 * Return true if then given LinkedList contains given void* value.
 * Note this doesn't any kind of equals function, just plain ``==''.
 */
bool LinkedList_contains(const LinkedList this, void *data) {
	for (Node node=this->first; node != NULL; node=node->next) {
		if (node->data == data) {
			return true;
		}
	}
	return false;
}

/**
 * Remove the given void* value from the given LinkedList if it is there.
 * This function uses ``=='' to test for the element.
 * Note that this does not free the data associated with the element.
 */
void LinkedList_remove(LinkedList this, void *data) {
	for (Node node=this->first; node != NULL; node=node->next) {
		if (node->data == data) {
			if (node == this->first) {
				this->first = node->next;
			}
			if (node == this->last) {
				this->last = node->prev;
			}
			if (node->prev != NULL) {
				node->prev->next = node->next;
			}
			if (node->next != NULL) {
				node->next->prev = node->prev;
			}
			free(node);
			return;
		}
	}
}    

/**
 * Return the void* value at the given index in the given LinkedList, or
 * NULL if there is no such.
 * Note that this means you can't store NULL in a LinkedList. C'est la vie.
 */
void* LinkedList_elementAt(LinkedList this, int index) {
	int i = 0;
	for (Node node=this->first; node != NULL; node=node->next) {
		if (i == index) {
			return node->data;
		}
		i += 1;
	}
	return NULL;
}

/**
 * Remove and return the first element from the given LinkedList.
 * Returns NULL if the list is empty.
 */
void* LinkedList_pop(LinkedList this) {
	void *data = LinkedList_elementAt(this, 0);
	if (data != NULL) {
		LinkedList_remove(this, data); // Removes first occurrence
	}
	return data;
}

/**
 * Call the given function on each element of given LinkedList, passing the
 * void* value to the function.
 */
void LinkedList_iterate(const LinkedList this, void (*func)(void *)) {
	for (Node node=this->first; node != NULL; node=node->next) {
		func(node->data);
	}
}

/**
 * A LinkedListIterator is simply a pointer to a node. It is initialized
 * to this->first and increments following next pointers until NULL.
 */
struct LinkedListIterator {
	Node next;
};

/**
 * Return an LinkedListIterator for the given LinkedList.
 * Don't forget to free() this when you're done iterating.
 */
LinkedListIterator keys(const LinkedList this) {
	LinkedListIterator iterator = (LinkedListIterator)malloc(sizeof(struct LinkedListIterator));
	iterator->next = this->first;
	return iterator;
}

/**
 * Return true if the given LinkedListIterator will return another element
 * if LinkedListIterator_next() is called.
 */
bool LinkedListIterator_hasNext(const LinkedListIterator this) {
	return this->next != NULL;
}

int LinkedList_size(LinkedList ll) {
	int count = 0;
	LinkedListIterator iter = keys(ll);
	while (LinkedListIterator_hasNext(iter)) {
		count++;
		LinkedListIterator_next(iter);
	}
	return count;
}

/**
 * Return the next value from the given LinkedListIterator and increment it
 * to point to the next element.
 * Will return NULL if there is no such element.
 * This means that you can't store NULL in a LinkedList. C'est la vie.
 * It would be easy to allow it and signal `no such element' some other way...
 */
void* LinkedListIterator_next(LinkedListIterator this) {
	if (this->next == NULL) {
		return NULL;
	} else {
		void *data = this->next->data;
		this->next = this->next->next;
		return data;
	}
}

bool LinkedList_equals_strings(LinkedList l1, LinkedList l2) {
	if (LinkedList_size(l1)!=LinkedList_size(l2)) {
			return false;
		}
		for (int i=0; i<LinkedList_size(l1); i++) {
			if (strcmp(LinkedList_elementAt(l1, i),LinkedList_elementAt(l2, i))) {
				return false;
			}
		}
		return true;
}

bool LinkedList_equals_integers(LinkedList l1, LinkedList l2) {
	if (LinkedList_size(l1)!=LinkedList_size(l2)) {
		return false;
	}
	for (int i=0; i<LinkedList_size(l1); i++) {
		if (LinkedList_elementAt(l1, i) != LinkedList_elementAt(l2, i)) {
			return false;
		}
	}
	return true;
}

/**
 * Print the given LinkedList to stdout, assuming that the values are
 * all null-terminated strings.
 * Also prints a newline (because why not).
 */
void LinkedList_print_string_list(LinkedList this) {
	printf("[");
	for (Node node=this->first; node != NULL; node=node->next) {
		printf("%s", (char*)(node->data));
		if (node->next != NULL) {
			printf(" ");
		}
	}
	printf("]\n");
}

void LinkedList_print_integer_list(LinkedList this) {
	printf("[");
	for (Node node=this->first; node != NULL; node=node->next) {
		printf("%i", (int)(node->data));
		if (node->next != NULL) {
			printf(" ");
		}
	}
	printf("]\n");
}

bool LinkedList_isSubset(LinkedList l1, LinkedList l2) {
	for (int i=0; i<LinkedList_size(l1); i++) {
		if (!LinkedList_contains(l2, LinkedList_elementAt(l1, i))) {
			return false;
		}
	}
	return true;
}

void LinkedList_removeIndex(LinkedList this, int index){
	 Node temp = this->first;
	 if (index == 0) {
		this->first = temp->next;
		free(temp);
		return;
	}
	for (int i=0; temp!=NULL && i<index-1; i++)
		temp = temp->next;
	if (temp == NULL || temp->next == NULL)
		return;
	Node next = temp->next->next;
	free(temp->next);
	temp->next = next;
}

#ifdef MAIN



int main(int argc, char **argv) {
	LinkedList list = new_LinkedList();
	printf("new list =");
	LinkedList_print_string_list(list);

	printf("adding three elements: ");
	LinkedList_add_at_end(list, "foo");
	LinkedList_add_at_end(list, "bar");
	LinkedList_add_at_end(list, "baz");
	LinkedList_print_string_list(list);
	printf("adding Ted at front: ");
	LinkedList_add_at_front(list, "Ted");
	LinkedList_print_string_list(list);

	printf("iterating over list:\n");
	LinkedListIterator iterator = keys(list);
	while (LinkedListIterator_hasNext(iterator)) {
		void *data = LinkedListIterator_next(iterator);
		char *str = (char*)data;
		printf("%s\n", str);
	}
	free(iterator);

	// Test remove in middle
	printf("removing bar from middle: ");
	LinkedList_remove(list, "bar");
	LinkedList_print_string_list(list);
	// Test remove at head
	printf("removing Ted from start: ");
	LinkedList_remove(list, "Ted");
	LinkedList_print_string_list(list);
	// Test remove in end
	printf("removing baz from end: ");
	LinkedList_remove(list, "baz");
	LinkedList_print_string_list(list);

	// Test remove only element
	void *elt = LinkedList_elementAt(list, 0);
	printf("element 0 is \"%s\"\n", (char*)elt);
	printf("removing only remaining element: ");
	LinkedList_remove(list, elt);
	LinkedList_print_string_list(list);

	printf("list isEmpty: %d\n", LinkedList_isEmpty(list));

	printf("freeing list\n");
	LinkedList_free(list, false);
}

#endif
