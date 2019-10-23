/*
 * Index.h
 *
 *  Created on: Nov 18, 2018
 *      Author: mijaz
 */

#ifndef INDEX_H_
#define INDEX_H_

#include "LinkedList.h"

typedef char* String;
typedef struct INDEX* INDEX;
typedef struct Node* Node;

extern Node new_Node(LinkedList attr);
extern String concatenate(String s1, String s2);
extern void insert(INDEX index, LinkedList input);
extern LinkedList lookUp(INDEX index, LinkedList parameters);
extern INDEX new_INDEX(LinkedList attr, LinkedList keys, int buckets, String name);
extern void printRelationContents(INDEX index, int keyIdx);
extern LinkedList lookUp(INDEX index, LinkedList parameters);
extern void delete(INDEX index, LinkedList parameters);
extern LinkedList allEntries(INDEX idx);

#endif /* INDEX_H_ */
