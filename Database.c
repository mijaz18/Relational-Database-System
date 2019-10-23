/*
 * Database.c
 *
 *  Created on: Nov 17, 2018
 *      Author: mijaz
 */

#include "Database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Database {
	LinkedList relations;
};

struct INDEX {
	String name; //name of relation;
	LinkedList schema; //list of string attributes
	LinkedList keys; //list of lists of integers, the first of which is the primary key
	LinkedList tables; //a list of hash tables, size depends on how many indices we have
	int entries;
	int buckets;
};


Database new_Database() {
	Database this = (Database) malloc(sizeof(struct Database));
	this->relations = new_LinkedList();
	return this;
}

void addRelation(Database d, INDEX i) {
	LinkedList_add_at_end(d->relations, i);
}

INDEX getRelation(Database d, char* name) {
	LinkedList relations = d->relations;
	for (int i=0; i<LinkedList_size(relations); i++) {
		INDEX temp = LinkedList_elementAt(relations,i);
		if (!strcmp(name, temp->name)) {
			return temp;
		}
	}
	return NULL;
}
