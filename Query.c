/*
 * Query.c
 *
 *  Created on: Nov 22, 2018
 *      Author: mijaz
 */

#include "Query.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Operations.h"

struct INDEX {
	String name; //name of relation;
	LinkedList schema; //list of string attributes
	LinkedList keys; //list of lists of integers, the first of which is the primary key
	LinkedList tables; //a list of hash tables, size depends on how many indices we have
	int entries;
	int buckets;
};

INDEX whatGrade(Database d) {

	printf("Enter student name\n");
	char studentName[128];
	fgets(studentName, 100, stdin);
	studentName[strlen(studentName) - 1] = '\0';

	printf("Enter course name\n");
	char courseName[128];
	fgets(courseName, 100, stdin);
	courseName[strlen(courseName) - 1] = '\0';

	INDEX snap = getRelation(d, "SNAP");
	//printRelationContents(snap, 0);
	INDEX csg = getRelation(d, "CSG");
	//printRelationContents(csg, 0);

	//params are ordered assuming join of SNAP and CSG results below schema:
	//SNAPCG
	LinkedList params = new_LinkedList();
	LinkedList_add_at_end(params, "*");
	LinkedList_add_at_end(params, studentName);
	LinkedList_add_at_end(params, "*");
	LinkedList_add_at_end(params, "*");
	LinkedList_add_at_end(params, courseName); //course name
	LinkedList_add_at_end(params, "*");

	LinkedList project = new_LinkedList();
	LinkedList_add_at_end(project, "Grade");

	return
			projection_operation(
			selection_operation(
					join_operation(snap, "StudentId", csg, "StudentId"),
					params),
					project);

}

INDEX whereIs(Database d) {

	printf("Enter student name\n");
	char studentName[128];
	fgets(studentName, 100, stdin);
	studentName[strlen(studentName) - 1] = '\0';

	printf("Enter time\n");
	char time[128];
	fgets(time, 100, stdin);
	time[strlen(time) - 1] = '\0';

	printf("Enter day\n");
	char day[128];
	fgets(day, 100, stdin);
	day[strlen(day) - 1] = '\0';

	INDEX snap = getRelation(d, "SNAP");
	INDEX csg = getRelation(d, "CSG");
	INDEX cdh = getRelation(d, "CDH");
	INDEX cr = getRelation(d, "CR");

	//project on course
	LinkedList projectOnCourse = new_LinkedList();
	LinkedList_add_at_end(projectOnCourse, "Course");

	//project on course and studentid
	LinkedList projectOnCourseAndStudentId = new_LinkedList();
	LinkedList_add_at_end(projectOnCourseAndStudentId, "Course");
	LinkedList_add_at_end(projectOnCourseAndStudentId, "StudentId");

	//project on studentid
	LinkedList projectOnStudentId = new_LinkedList();
	LinkedList_add_at_end(projectOnStudentId, "StudentId");

	//project on room
	LinkedList projectOnRoom = new_LinkedList();
	LinkedList_add_at_end(projectOnRoom, "Room");

	//select on day and hour from CDH
	LinkedList selectOnDayAndHour = new_LinkedList();
	LinkedList_add_at_end(selectOnDayAndHour, "*");
	LinkedList_add_at_end(selectOnDayAndHour, day);
	LinkedList_add_at_end(selectOnDayAndHour, time);

	//select on name from SNAP
	LinkedList selectOnName = new_LinkedList();
	LinkedList_add_at_end(selectOnName,"*");
	LinkedList_add_at_end(selectOnName, studentName);
	LinkedList_add_at_end(selectOnName, "*");
	LinkedList_add_at_end(selectOnName, "*");

	return

			projection_operation(
			join_operation(
					join_operation(
							projection_operation(
									join_operation(
											projection_operation(csg, projectOnCourseAndStudentId), "StudentId",
											projection_operation(
													selection_operation(snap, selectOnName), projectOnStudentId), "StudentId"), projectOnCourse), "Course",
							projection_operation(
									selection_operation(cdh, selectOnDayAndHour), projectOnCourse)
									, "Course")
									, "Course",
					cr, "Course"), projectOnRoom);

}

void query_REPL(Database d) {
	while(1) {

		printf("\n----------------------------------\n");

		printf(
				"Enter \"Y\" to figure out what grade student x received in course y or enter quit to go to next query:\n");
		char input[128];
		fgets(input, 100, stdin);
		input[strlen(input) - 1] = '\0';
		if (!strcmp(input, "quit")) {
			break;
		}

		printRelationContents(whatGrade(d), 0);

	}

	while(1) {

		printf("\n----------------------------------\n");

		printf(
				"Enter \"Y\" to figure out where student x is on day y and time z or enter quit to exit:\n");
		char input[128];
		fgets(input, 100, stdin);
		input[strlen(input) - 1] = '\0';
		if (!strcmp(input, "quit")) {
			break;
		}

		printRelationContents(whereIs(d), 0);

	}
}
