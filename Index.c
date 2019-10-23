/*
 * Index.c
 *
 *  Created on: Nov 18, 2018
 *      Author: mijaz
 */

#include <stdio.h>
#include <stdlib.h>
#include "Index.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#define INT2VOIDP(i) (void*)(uintptr_t)(i)

struct Node {
	Node next;
	LinkedList attr;
};

Node new_Node(LinkedList attr) {
	Node this = (Node) malloc(sizeof(struct Node));
	this -> attr = attr;
	return this;
};

struct INDEX {
	String name; //name of relation;
	LinkedList schema; //list of string attributes
	LinkedList keys; //list of lists of integers, the first of which is the primary key
	LinkedList tables; //a list of hash tables, size depends on how many indices we have
	int entries;
	int buckets;
};

INDEX new_INDEX(LinkedList attr, LinkedList keys, int buckets, String name) {
	INDEX this = (INDEX) malloc(sizeof(struct INDEX));
	this -> name = name;
	this -> keys = keys;
	this -> schema = attr;
	this -> buckets = buckets;
	this -> entries = 0;
	this -> tables = new_LinkedList();
	for (int i=0; i<LinkedList_size(keys);i++) {
		Node* table = (Node*) calloc(this->buckets, sizeof(Node)); //IS THIS RIGHT?
		LinkedList_add_at_end(this -> tables, table);
	}
	return this;
}

//constructs a hash from a string
int hash_helper(char* key, int tableSize) {
    int hashVal = 0;
    int prime[20] = {1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123};
	for(int i=0; i< sizeof(key); i++)
	{
	    int ascii = key[i] * prime[i % 20];
	    hashVal =  hashVal+ascii;
	}
	return hashVal % tableSize;
}

//finds index of primary or secondary key which match query arguments
int getKeyIndexIfItExists(LinkedList search, LinkedList keys) {
	for (int i=0; i<LinkedList_size(keys); i++) {
		if (LinkedList_equals_integers(search, LinkedList_elementAt(keys, i))) {
			return i;
		}
	}
	return -1;
}

//this checks whether the specific indices defined by indices match
bool containsKeyHelper(LinkedList indices, LinkedList l1, LinkedList l2) {
	LinkedListIterator iter = keys(indices);
	while (LinkedListIterator_hasNext(iter)) {
		int idx = (int) (size_t) LinkedListIterator_next(iter);
		if (strcmp(LinkedList_elementAt(l1, idx), LinkedList_elementAt(l2, idx))) { //NOT SURE ABOUT THIS
			return false;
		}
	}
	return true;
}

//constructs a hash from a list of integer indices and list of strings
int hash(LinkedList primaryKeys, LinkedList input, int buckets) {
	char* hashStr = "";
	LinkedListIterator iter = keys(primaryKeys);
	while (LinkedListIterator_hasNext(iter)) {
		hashStr = concatenate(hashStr, (char*) LinkedList_elementAt(input, (int) (size_t) LinkedListIterator_next(iter)));
	}
	return hash_helper(hashStr, buckets);
}

//returns integer index of best viable key for query if it exists, else -1
int bestViableKeyForQuery(LinkedList keys, LinkedList query) {
	int temp = -1;
	for (int i=0; i<LinkedList_size(keys); i++) {
		if (LinkedList_isSubset(LinkedList_elementAt(keys,i), query)) {
			if (temp == -1) {
				temp = i;
			}
			else {
				if (LinkedList_size(LinkedList_elementAt(keys,temp))< (int)LinkedList_elementAt(keys,i)) {
					temp = i;
				}
			}
		}
	}
	return temp;
}

/*
 * Takes in an index and returns all the entries in the index as a LinkedList.
 */
LinkedList allEntries(INDEX idx)
{
    LinkedList allEntries = new_LinkedList();
    Node* primaryTable = LinkedList_elementAt(idx->tables, 0);
    for (int i=0; i<idx->buckets; i++) {
        Node curr = primaryTable[i];
        while(curr != NULL)
        {
            LinkedList tuple = curr->attr;
            LinkedList_add_at_end(allEntries, tuple);
            curr = curr->next;
        }
    }
    return allEntries;
}

//"*" parameter indicates N/A
LinkedList lookUp(INDEX index, LinkedList parameters) {
	LinkedList corresponding = new_LinkedList();

	for (int i=0; i<LinkedList_size(parameters); i++) {
		if (!strcmp((char*)LinkedList_elementAt(parameters,i), "*")) {
			continue;
		}
		LinkedList_add_at_end(corresponding, INT2VOIDP(i));
	}

	int key = getKeyIndexIfItExists(corresponding, index->keys);

	LinkedList toBeReturned = new_LinkedList();
	//this is the case where we have an index for the given parameters
	if (key!=-1) {

		Node* table = LinkedList_elementAt(index->tables,key);
		int hashi = hash (LinkedList_elementAt(index->keys, key), parameters, index->buckets);
		Node bucket = table[hashi];
		while (bucket!=NULL) {

			if (containsKeyHelper(LinkedList_elementAt(index->keys, key), parameters, bucket->attr)) {
				LinkedList_add_at_end(toBeReturned, bucket->attr);
			}

			bucket = bucket -> next;
		}
	}
	//this is the case where we don't have an index exactly matching the given parameters
	else {
		int bestViable = bestViableKeyForQuery(index->keys, corresponding);
		if (bestViable == -1) {

			//brute force it because there is no index we can optimize with
			LinkedList all = allEntries(index);
			LinkedListIterator iter = keys(all);
			while (LinkedListIterator_hasNext(iter)) {
				void* node = LinkedListIterator_next(iter);

				if (!containsKeyHelper(corresponding, parameters, node)) {
					LinkedList_remove(all, node);
				}
			}
			toBeReturned = all;
		}
		else {

			//use the index to optimize
			int hashi = hash (LinkedList_elementAt(index->keys, bestViable), parameters, index->buckets);
			Node x = ((Node*)LinkedList_elementAt(index->tables, bestViable))[hashi];

			//check all entries in bucket
			while (x!=NULL) {
				//if matches on all defined parameters, add entry to list
				if (containsKeyHelper(corresponding, parameters, x->attr)) {
					LinkedList_add_at_end(toBeReturned, x->attr);
				}
				x = x->next;
			}

		}
	}

	return toBeReturned;
}

//returns true if table already contains entry with same primary key
bool containsPrimaryKey(LinkedList primaryKey, LinkedList toBeAdded, Node table[], int hash) {

	Node x = table[hash];

	while (x!=NULL) {
		if (containsKeyHelper(primaryKey, toBeAdded, x->attr)) {
			return true;
		}
		x = x->next;
	}

	return false;
}

String concatenate(String s1, String s2) {
	char* str = (char*) calloc(180, sizeof(char));
	strcpy(str, s1);
	strcat(str, s2);
	return str;
}

void insert_table_helper(Node table[], int hash, Node new_Node)
{
	new_Node->next = table[hash];
	table[hash] = new_Node;
}

void insert(INDEX index, LinkedList input) {
	LinkedList primaryKeys = LinkedList_elementAt(index->keys,0);
	int hashIdx = hash(primaryKeys, input, index->buckets);

	//if already contained, don't add it
	if (containsPrimaryKey(primaryKeys, input, LinkedList_elementAt(index->tables, 0), hashIdx)) {
		//printf("Tuple already exists in Relation %s\n", index->name);
		return;
	}

	index->entries+=1;
	for(int i = 0; i < LinkedList_size(index -> keys); i++)
	{
		Node newNode = new_Node(input);
		int hashi = hash (LinkedList_elementAt(index->keys,i), input, index->buckets);
		insert_table_helper(LinkedList_elementAt((index->tables),i), hashi, newNode);
	}
	//printf("Tuple successfully inserted into Relation %s\n", index->name);
}


void printRelationContents(INDEX index, int keyIdx) {
	printf("Table %s\n", index->name);
	LinkedList schema = index->schema;
	LinkedListIterator schemaIter = keys(schema);
	printf("%s\t", "bucket");
	while (LinkedListIterator_hasNext(schemaIter)) {
		printf("%s\t", (char*) LinkedListIterator_next(schemaIter));
	}
	printf("\n");
	Node* primaryTable = LinkedList_elementAt(index->tables, keyIdx);
	for (int i=0; i<index->buckets; i++) {
		Node curr = primaryTable[i];

		while (curr!=NULL) {
			LinkedList attr = curr->attr;

			LinkedListIterator attrIter = keys(attr);
			printf("%i\t", i);
			while (LinkedListIterator_hasNext(attrIter)) {
				printf("%s\t", (char*) LinkedListIterator_next(attrIter));
			}
			printf("\n");
			curr = curr -> next;

		}

	}
}

void delete_helper(Node* buckets, int hash, LinkedList keyIndices, LinkedList parameters) {

	Node start = NULL;
	Node end = NULL;
	Node curr = buckets[hash];

	while (curr != NULL) {

		if (!containsKeyHelper(keyIndices, parameters, curr->attr)) {
			if (start == NULL) {
				start = curr;
				end = curr;
			}
			else {
				end -> next = curr;
				end = curr;
			}
		}
		else {
			if (end != NULL) {
				end -> next = NULL;
			}
		}

		curr = curr -> next;
	}

	buckets[hash] = start;
}

//worst case time complexity: O(n+kt) where n=entries, k=keys, t=tuples to be deleted
void delete(INDEX index, LinkedList parameters) {
	LinkedList tuples = lookUp(index, parameters);

	LinkedList keyIndices = new_LinkedList();
	for (int i=0; i<LinkedList_size(parameters); i++) {
		if (!strcmp((char*)LinkedList_elementAt(parameters,i), "*")) {
			continue;
		}
		LinkedList_add_at_end(keyIndices, INT2VOIDP(i));
	}

	//LinkedList_print_integer_list(keyIndices);

	for (int i=0; i<LinkedList_size(tuples); i++) {
		LinkedList tuple = LinkedList_elementAt(tuples, i);

		//need to delete from all the primary and secondary key tables
		LinkedList tables = index->tables;
		for (int j=0; j<LinkedList_size(tables); j++) {
			int hashi = hash(LinkedList_elementAt(index->keys, j), tuple, index->buckets);
			delete_helper(LinkedList_elementAt(tables, j), hashi, keyIndices, parameters);
		}
	}

}
