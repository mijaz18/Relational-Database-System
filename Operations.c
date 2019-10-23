/*
 * Operations.c
 *
 *  Created on: Nov 22, 2018
 *      Author: mijaz
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Operations.h"
#define INT2VOIDP(i) (void*)(uintptr_t)(i)

struct INDEX {
	String name; //name of relation;
	LinkedList schema; //list of string attributes
	LinkedList keys; //list of lists of integers, the first of which is the primary key
	LinkedList tables; //a list of hash tables, size depends on how many indices we have
	int entries;
	int buckets;
};


INDEX projection_operation(INDEX idx, LinkedList projection) {

	//special case
	if(LinkedList_isEmpty(projection)) {
		printf("Requested empty projection is not valid\n");
		return NULL;
	}

	char* pi = "π[";
	char* proj_str = "";
	for (int i=0; i<LinkedList_size(projection); i++) {
		proj_str = concatenate(proj_str, LinkedList_elementAt(projection, i));
		proj_str = concatenate(proj_str, ",");
	}
	proj_str[strlen(proj_str)-1] = ']';
	pi = concatenate(pi, proj_str);
	pi = concatenate(pi, "(");
	pi = concatenate(pi, idx->name);
	pi = concatenate(pi, ")");

	LinkedList new_schema = projection;
	LinkedList old_schema = idx->schema;

	//make primary key for projection a combination of all attributes
	//this is to guarantee primary key is unique
	LinkedList new_keys = new_LinkedList();
	LinkedList primaryKey = new_LinkedList();
	for (int i=0; i<LinkedList_size(projection); i++) {
		LinkedList_add_at_end(primaryKey, INT2VOIDP(i));
	}
	LinkedList_add_at_end(new_keys, primaryKey);

	INDEX new_idx = new_INDEX(new_schema, new_keys, idx->buckets, pi);

	//add modified entries to projection
	LinkedList entries = allEntries(idx);


	for (int i=0; i<LinkedList_size(entries); i++) {

		LinkedList entry = LinkedList_elementAt(entries, i);
		LinkedList modified_entry = new_LinkedList();

		for (int j=0; j<LinkedList_size(new_schema); j++) {
			char* attr_name = LinkedList_elementAt(new_schema, j);

			bool foundMatch = false;
			for (int k=0; k<LinkedList_size(old_schema); k++) {
				if (!strcmp(LinkedList_elementAt(old_schema, k), attr_name)) {
					foundMatch = true;
					LinkedList_add_at_end(modified_entry, LinkedList_elementAt(entry, k));
					break;
				}
			}

			//this means requested attribute of projection was not found, so fail
			if (!foundMatch) {
				printf("Requested projection attribute \"%s\" is not a valid attribute of table %s\n", attr_name, idx->name);
				return NULL;
			}

		}

		insert(new_idx, modified_entry);

	}

	return new_idx;
}

INDEX selection_operation(INDEX idx, LinkedList parameters){

	char* sigma = "σ[";
	char* proj_str = (char*) calloc(10, sizeof(char));
	LinkedList_size(parameters);

	for (int i=0; i<LinkedList_size(parameters); i++) {
		if (strcmp(LinkedList_elementAt(parameters,i), "*")) {
			proj_str = concatenate(proj_str, LinkedList_elementAt(idx->schema, i));
			proj_str = concatenate(proj_str, "=\"");
			proj_str = concatenate(proj_str, LinkedList_elementAt(parameters,i));
			proj_str = concatenate(proj_str, "\",");
		}
	}

	if (strlen(proj_str)!=0){
		proj_str[strlen(proj_str)-1] = ']';
	} else {
		proj_str[strlen(proj_str)] = ']';
	}
	sigma = concatenate(sigma, proj_str);
	sigma = concatenate(sigma, "(");
	sigma = concatenate(sigma, idx->name);
	sigma = concatenate(sigma, ")");
	LinkedList tuples = lookUp(idx, parameters);
	INDEX selected = new_INDEX(idx->schema,idx->keys, idx->buckets,sigma);
	for(int q = 0; q < LinkedList_size(tuples); q++){
		//LinkedList_print_string_list(LinkedList_elementAt(tuples,q));
		insert(selected,LinkedList_elementAt(tuples,q));
	}
	return selected;
}

INDEX union_operation (INDEX idx1, INDEX idx2){
	/*
	 * Getting all entries of idx2 and inserting it to the first index
	 * b/c insert checks for duplicate we should be good.
	 */

	if (!LinkedList_equals_strings(idx1->schema, idx2->schema)) {
		printf("Schemas of tables don't match so union is invalid\n");
		return NULL;
	}

	char* union_name = "(";
	union_name = concatenate(union_name, idx1->name);
	union_name = concatenate(union_name, " U ");
	union_name = concatenate(union_name, idx2->name);
	union_name = concatenate(union_name, ")");

	INDEX temp = new_INDEX(idx1->schema, idx1->keys, idx1->buckets, union_name);
	LinkedList idx2Entries = allEntries(idx2);
	LinkedList idx1Entries = allEntries(idx1);

	for(int a = 0; a < LinkedList_size(idx2Entries); a++){
		insert(temp,LinkedList_elementAt(idx2Entries,a));
	}
	for(int a = 0; a < LinkedList_size(idx1Entries); a++){
		insert(temp,LinkedList_elementAt(idx1Entries,a));
	}
	return temp;
}

INDEX intersection_operation(INDEX idx1, INDEX idx2){

	if (!LinkedList_equals_strings(idx1->schema, idx2->schema)) {
		printf("Schemas of tables don't match so intersection is invalid\n");
		return NULL;
	}

	char* intersection_name = "(";
	intersection_name = concatenate(intersection_name, idx1->name);
	intersection_name = concatenate(intersection_name, " ∩ ");
	intersection_name = concatenate(intersection_name, idx2->name);
	intersection_name = concatenate(intersection_name, ")");

	INDEX temp = new_INDEX(idx1->schema, idx1->keys, idx1->buckets, intersection_name);

    LinkedList idx1Entries = allEntries(idx1);

    for(int a = 0; a<LinkedList_size(idx1Entries); a++){
        LinkedList lookUpHelp = lookUp(idx2, LinkedList_elementAt(idx1Entries, a));
        if(LinkedList_size(lookUpHelp) != 0){
            /*
             * If entry is in both tables, the lookup function will return list of size > 0
             */
            insert(temp, LinkedList_elementAt(lookUpHelp, 0));
        }
    }
    return temp;
}

INDEX difference_operation(INDEX idx1, INDEX idx2){
	if (!LinkedList_equals_strings(idx1->schema, idx2->schema)) {
		printf("Schemas of tables don't match so difference is invalid\n");
		return NULL;
	}

	char* difference_name = "(";
	difference_name = concatenate(difference_name, idx1->name);
	difference_name = concatenate(difference_name, " ∖ ");
	difference_name = concatenate(difference_name, idx2->name);
	difference_name = concatenate(difference_name, ")");

	INDEX new_table = new_INDEX(idx1->schema, idx1->keys, idx1->buckets, difference_name);
	LinkedList idx1Entries = allEntries(idx1);
	for (int i = 0; i < LinkedList_size(idx1Entries); i++) {
		insert(new_table, LinkedList_elementAt(idx1Entries, i));
	}

	LinkedList idx2Entries = allEntries(idx2);
	for (int i=0; i<LinkedList_size(idx2Entries); i++) {
		delete(new_table, LinkedList_elementAt(idx2Entries, i));
	}

	return new_table;
}

LinkedList joinHelper(LinkedList idx1, String attr1, LinkedList idx2, String attr2){
	LinkedList indexList = new_LinkedList();
    //LinkedList_print_string_list(idx1);
    //LinkedList_print_string_list(idx2);
	//printf("%i\n",LinkedList_size(idx1));
	//printf("%i\n",LinkedList_size(idx2));
	for(int a = 0; a < LinkedList_size(idx1); a++){
		//printf("\n%s\n", LinkedList_elementAt(idx1, a));
        if(!strcasecmp(LinkedList_elementAt(idx1,a),attr1)) {
           //printf("\n%s\n", LinkedList_elementAt(idx1, a));
            LinkedList_add_at_end(indexList, INT2VOIDP(a));
        }
    }
    for(int b = 0; b < LinkedList_size(idx2); b++){
    	//printf("\n%s\n", LinkedList_elementAt(idx2, b));
        if(!strcasecmp(LinkedList_elementAt(idx2,b), attr2)) {
            LinkedList_add_at_end(indexList, INT2VOIDP(b));
        }
    }
    return indexList;
}

INDEX join_operation(INDEX idx1, String attr1, INDEX idx2, String attr2) {
	char* join_name = "(";

	join_name = concatenate(join_name, idx1->name);
	join_name = concatenate(join_name, " [");
	join_name = concatenate(join_name, attr1);
	join_name = concatenate(join_name, "⋈");
	join_name = concatenate(join_name, attr2);
	join_name = concatenate(join_name, "] ");
	join_name = concatenate(join_name, idx2->name);
	join_name = concatenate(join_name, ")");


	LinkedList secondSchema  = new_LinkedList();
	for (int a = 0; a < LinkedList_size(idx2->schema); a++) {
		LinkedList_add_at_end(secondSchema,
				LinkedList_elementAt(idx2->schema, a));
	}

	LinkedList joinIndexList = joinHelper(idx1->schema, attr1, idx2->schema,
			attr2);


	int a = (int) (size_t) (LinkedList_elementAt(joinIndexList, 0));
	int b = (int) (size_t) (LinkedList_elementAt(joinIndexList, 1));

	LinkedList_remove(secondSchema, LinkedList_elementAt(secondSchema, b));

	//LinkedList_print_string_list(secondSchema);


	LinkedList keys = new_LinkedList();
	LinkedList subKeys = new_LinkedList();
	for (int z= 0;z< LinkedList_size(idx1->schema)+ LinkedList_size(idx2->schema) - 1; z++) {
		LinkedList_add_at_end(subKeys, INT2VOIDP(z));
	}

	LinkedList_add_at_end(keys, subKeys);
	INDEX join = new_INDEX(LinkedList_union(idx1->schema, secondSchema), keys,
			idx1->buckets, join_name);


	LinkedList idx1Entries = allEntries(idx1);
	for (int q = 0; q < LinkedList_size(idx1Entries); q++) {
		LinkedList toBeAdded = new_LinkedList();
		LinkedList params = new_LinkedList();

		LinkedList tuple = LinkedList_elementAt(idx1Entries, q);

		for (int p = 0; p < LinkedList_size(idx2->schema); p++) {

			if (p == b) {

				LinkedList_add_at_end(params, LinkedList_elementAt(tuple, a));
			} else
				LinkedList_add_at_end(params, "*");
		}

		LinkedList look = lookUp(idx2, params);
		for (int l = 0; l < LinkedList_size(look); l++) {


			LinkedList temp = new_LinkedList();
			for (int z =0; z<LinkedList_size(LinkedList_elementAt(look, l)); z++) {
				LinkedList_add_at_end(temp, LinkedList_elementAt(LinkedList_elementAt(look, l), z));
			}

			LinkedList_removeIndex(temp, b);

			toBeAdded = LinkedList_union(LinkedList_elementAt(idx1Entries, q),
					temp);
			//LinkedList_removeIndex(toBeAdded, LinkedList_size(idx1->schema) + b);
			//LinkedList_print_string_list(toBeAdded);
			insert(join, toBeAdded);
		}
	}

	return join;

}
