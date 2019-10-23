/*
 * IO.c
 *
 *  Created on: Nov 20, 2018
 *      Author: mijaz
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "IO.h"
#define INT2VOIDP(i) (void*)(uintptr_t)(i)

struct Node {
	Node next;
	LinkedList attr;
};

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

void printDatabaseToConsole(Database d) {
	LinkedList relations = d->relations;
	for (int i=0; i<LinkedList_size(relations); i++) {
		printRelationContents(LinkedList_elementAt(relations, i), 0);
	}
}

void writeDataFile(Database d, String filepath) {
	LinkedList relations = d->relations;
	FILE *fp;
	fp = fopen(filepath, "w+");
	for (int i=0; i<LinkedList_size(relations); i++) {
		writeDataFileHelper(LinkedList_elementAt(relations, i), fp);
		fprintf(fp, "\n\n");
	}
	fclose(fp);
}

void writeDataFileHelper(INDEX index, FILE* fp) {
	fprintf(fp, "[%s]\n", index->name);
	LinkedList schema = index->schema;
	LinkedListIterator schemaIter = keys(schema);
	char* schemaStr = "[";
	while (LinkedListIterator_hasNext(schemaIter)) {
		schemaStr = concatenate(schemaStr, (char*) LinkedListIterator_next(schemaIter));
		schemaStr = concatenate(schemaStr, ",");
	}
	schemaStr[strlen(schemaStr)-1] = 0;
	fprintf(fp, "%s]",schemaStr);
	fprintf(fp, "\n");

	for (int a = 0; a < LinkedList_size(index->keys); a++) {
		LinkedList key = LinkedList_elementAt(index->keys, a);
		LinkedListIterator keyIter = keys(key);
		char* entry = "[";
		while (LinkedListIterator_hasNext(keyIter)) {
			char str[5];
			sprintf(str, "%d", (int) LinkedListIterator_next(keyIter));
			entry = concatenate(entry, str);
			entry = concatenate(entry, ",");
		}
		entry[strlen(entry)-1] = 0;
		fprintf(fp, "%s]", entry);

	}
	fprintf(fp, "\n");
	fprintf(fp, "[%d]\n", index->buckets);
	Node* primaryTable = LinkedList_elementAt(index->tables, 0);
	for (int i = 0; i < index->buckets; i++) {
		Node curr = primaryTable[i];
		while (curr != NULL) {
			LinkedList attr = curr->attr;
			LinkedListIterator attrIter = keys(attr);
			char* entry = "[";
			while (LinkedListIterator_hasNext(attrIter)) {

				entry = concatenate(entry, (char*) LinkedListIterator_next(attrIter));
				entry = concatenate(entry, ",");
			}
			entry[strlen(entry)-1] = 0;
			fprintf(fp, "%s]\n",entry);
			curr = curr->next;
		}
	}
}

//Used Stack Overflow for this: https://stackoverflow.com/questions/174531/how-to-read-the-content-of-a-file-to-a-string-in-c
char* readFileAsString(String path) {
	char * buffer = 0;
	long length;
	FILE * f = fopen (path, "rb");

	if (f)
	{
	  fseek (f, 0, SEEK_END);
	  length = ftell (f);
	  fseek (f, 0, SEEK_SET);
	  buffer = malloc (length);
	  if (buffer)
	  {
	    fread (buffer, 1, length, f);
	  }
	  fclose (f);
	}

	if (buffer)
	{
	  // start to process your data / extract strings here...
		return buffer;
	}
	return NULL;
}

int stringToInt(String s) {
	int d;
	sscanf(s, "%d", &d);
	return d;
}

Database readDataFile(String path){

	Database d = new_Database();
	char* file = readFileAsString(path);
	int len = strlen(file);
	int i = 0;

	while (i<len) {

		//read in title of relation
		int titleStart = i+1;
		int titleEnd = i+2;
		while (file[titleEnd]!=']') {
			titleEnd++;
		}
		char* name = (char*) calloc(10, sizeof(char));
		strncpy(name, file + titleStart, titleEnd - titleStart);
		name[titleEnd-titleStart] = '\0';
		i = titleEnd+3;
		//printf("%s\n", name);

		//read in schema
		LinkedList schema = new_LinkedList();
		while(file[i-1]!=']') {
			int startSchema = i;
			int endSchema = i+1;
			while (file[endSchema]!=',' && file[endSchema]!=']') {
				endSchema++;
			}
			char* attr = (char*) calloc(10, sizeof(char));
			strncpy(attr, file + startSchema, endSchema - startSchema);
			//printf("%s\n", attr);
			attr[endSchema-startSchema] = '\0';
			LinkedList_add_at_end(schema, attr);
			i=endSchema+1;
		}
		i+=1;
		//LinkedList_print_string_list(schema);

		//read in keys
		LinkedList keys = new_LinkedList();
		while(file[i]!='\n') {

			LinkedList key = new_LinkedList();
			int start = i+1;
			int end = i+2;
			while (file[start-1]!=']') {
				while (file[end]!=',' && file[end]!=']') {
					end++;
				}

				char* unit = (char*) calloc(10, sizeof(char));
				strncpy(unit, file + start, end - start);
				unit[end-start] = '\0';
				int val = stringToInt(unit);
				//printf("%i\n",val);
				LinkedList_add_at_end(key, INT2VOIDP(val));
				start = end+1;
				end = start+1;
			}
			//LinkedList_print_integer_list(key);
			//printf("next\n");
			LinkedList_add_at_end(keys, key);
			i = start;
		}
		i++;

		//read in number of buckets
		int startbucket = i+1;
		int endbucket = i+2;
		while(file[endbucket]!=']') {
			endbucket++;
		}
		char* bucketStr = (char*) calloc(10, sizeof(char));
		strncpy(bucketStr, file + startbucket, endbucket - startbucket);
		bucketStr[endbucket-startbucket] = '\0';
		int buckets = stringToInt(bucketStr);
		//printf("%i\n", buckets);
		i = endbucket+2;

		//initialize Relation
		INDEX idx = new_INDEX(schema, keys, buckets, name);

		//read in table entries
		while (!(file[i]=='\n' && file[i-1]=='\n')) {

			LinkedList tuple = new_LinkedList();

			int start = i+1;
			int end = i+2;
			while (file[start-1]!=']') {
				while (file[end]!=',' && file[end]!=']') {
					end++;
				}

				char* unit = (char*) calloc(50, sizeof(char));
				strncpy(unit, file + start, end - start);
				unit[end-start] = '\0';

				LinkedList_add_at_end(tuple, unit);
				start = end+1;
				end = start+1;
			}
			//LinkedList_print_string_list(tuple);

			//printf("%i, %i", len, i);
			insert(idx, tuple);
			i = start+1;
			//printf("%c\n",file[i]);
			//break;
		}
		i+=2;
		//printf("%c\n", file[i]);

		//printf("%i, %i", len, i);
		//break;
		addRelation(d, idx);
	}

	//printDatabaseToConsole(d);

	return d;

}
