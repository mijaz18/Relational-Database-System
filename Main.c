/*
 * Main.c
 *
 *  Created on: Nov 16, 2018
 *      Author: mijaz
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "Index.h"
#include "IO.h"
#include "Operations.h"
#include "Query.h"
#include "Parser.h"

#define INT2VOIDP(i) (void*)(uintptr_t)(i)

struct Node {
	Node next;
	LinkedList attr;
};

struct INDEX {
	String name; //name of relation;
	LinkedList schema; //list of string attributes
	LinkedList keys; //list of lists of integers, the first of which is the primary key
	LinkedList tables; //a list of hash tables, size depends on how many indices we have
	int entries;
	int buckets;
};

int main(int argc, char* argv[]) {

	Database d = readDataFile("databaseIN.txt");

	/*
	 * PART 1:
	 * Demo all three operations by performing operations shown in Example 8.2 (p. 409)
	 * Be sure that your program explains itself when run (using informative printed messages)
	 */

	printf("lookup((“CS101”, 12345, ∗), CSG)\n");
	LinkedList results1 = lookUp(getRelation(d, "CSG"), generateLinkedListFromString("CS101,12345,*"));
	for (int i = 0; i<LinkedList_size(results1); i++) {
		LinkedList_print_string_list(LinkedList_elementAt(results1, i));
	}
	printf("\n\n");

	printf("lookup((“CS205”, “CS120”), Course-Prerequisite)\n");
	results1 = lookUp(getRelation(d, "CP"), generateLinkedListFromString("CS205,CS120"));
	for (int i = 0; i < LinkedList_size(results1); i++) {
		LinkedList_print_string_list(LinkedList_elementAt(results1, i));
	}
	printf("\n\n");

	printf("delete((“CS101”,∗), Course-Room)\n");
	delete(getRelation(d, "CR"), generateLinkedListFromString("CS101,*"));
	printRelationContents(getRelation(d, "CR"), 0);
	printf("\n\n");

	printf("insert((“CS205”, “CS120”), Course-Prerequisite)\n");
	insert(getRelation(d, "CP"), generateLinkedListFromString("CS205,CS120"));
	printRelationContents(getRelation(d, "CP"), 0);
	printf("\n\n");

	/* The below command doesn't change the CP relation because identical tuple is already inserted */
	printf("insert((“CS205”, “CS101”), Course-Prerequisite)");
	insert(getRelation(d, "CP"), generateLinkedListFromString("CS205,CS101"));
	printRelationContents(getRelation(d, "CP"), 0);
	printf("\n\n");
	/*
	 * PART 2:
	 * Regenerate original unmodified database for part 2
	 * Optimized the Query by pushing down selections and projections
	 */
	d = readDataFile("databaseIN.txt");
	query_REPL(d);
	printf("\n\n");

	/*
	 * PART 3:
	 * Demo the operations on the database described in Examples 8.12 (Selection), 8.13 (Projection), 8.14 (Join), and 8.15 (all three)
	 */
	printf("σCourse=“CS101”(CSG)\n");
	printRelationContents(selection_operation(getRelation(d, "CSG"), generateLinkedListFromString("CS101,*,*")), 0);
	printf("\n\n");

	printf("πStudentId(σCourse=“CS101”(CSG))\n");
	printRelationContents(
			projection_operation(
					selection_operation(getRelation(d, "CSG"),
					generateLinkedListFromString("CS101,*,*")),
					generateLinkedListFromString("StudentId")),
					0);
	printf("\n\n");

	printf("CR [Course⋈Course] CDH\n");
	printRelationContents(join_operation(getRelation(d, "CR"), "Course", getRelation(d, "CDH"), "Course"),0);
	printf("\n\n");

	printf("πDay,Hour(σRoom=“Turing Aud.”(CR [Course⋈Course] CDH))\n");
	printRelationContents(projection_operation(selection_operation(join_operation(getRelation(d, "CR"), "Course", getRelation(d, "CDH"), "Course"), generateLinkedListFromString("*,Turing Aud.,*,*")), generateLinkedListFromString("Day,Hour")),0);
	printf("\n\n");

	writeDataFile(d, "databaseOUT.txt");

	/*
	 * EXTRA CREDIT:
	 * 1) Note that everything is completely generalized!
	 *    Even the quantity and values of secondary keys are generalized and up to the user's specification (see the databaseIN.txt file)
	 * 2) Added extra Union, Intersection, and Difference functions
	 * 3) Also built a parser that supports the following commands:
	 * 			CREATE TABLE <TABLENAME> (<column_1>, <column_2>,<column_3>, ..., <column_n>);
	 * 			//for CREATE TABLE command we assume primary key is combination of all attributes
	 *			SELECT * from <TABLENAME>
	 *			DELETE * from <TABLENAME>
	 *			INSERT INTO <TABLENAME> (<arg1>, <arg2>, ..., <argn>)
	 */

	sqlCommand(d, "CREATE TABLE DEMO (attr1, attr2, attr3)");
	sqlCommand(d, "INSERT INTO DEMO (dan, j, ro)");
	sqlCommand(d, "INSERT INTO DEMO (pra, m, sh)");
	sqlCommand(d, "INSERT INTO DEMO (mo, u, ij)");
	sqlCommand(d, "SELECT * from DEMO");
	sqlCommand(d, "DELETE * from DEMO");
	sqlCommand(d, "SELECT * from DEMO");

}
