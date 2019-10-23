/*
 * Parser.c
 *
 *  Created on: Nov 22, 2018
 *      Author: mijaz
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "Parser.h"
#include "LinkedList.h"
#include "Index.h"
#include "Database.h"
#include "IO.h"
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
void removeSpaces(char *str)
{
    /*
     * geeks for geeks
     */
    int count = 0;
    for (int i = 0; str[i]; i++)
        if (str[i] != ' ')
            str[count++] = str[i];
    str[count] = '\0';
}
void sqlCommand(Database d, char* command){

    int length = strlen(command);
    int i = 0;

    while (i<length) {
        int titleStart = i;
        int titleEnd = i+1;
        while (command[titleEnd]!=' ') {
            titleEnd++;
        }
        char* operation = (char*) calloc(10, sizeof(char));
        strncpy(operation, command + titleStart, titleEnd - titleStart);
        operation[titleEnd-titleStart] = ' ';

        removeSpaces(operation);


        if(!strcasecmp(operation, "CREATE")){
            //study after " into "

            i = titleEnd+7;

            int tableStart = i;
            int tableEnd = i+1;

            while (command[tableEnd]!='(') {
                tableEnd++;
            }
            char* tableName = (char*) calloc(10, sizeof(char));
            strncpy(tableName, command + tableStart, tableEnd - tableStart);
            tableName[tableEnd-tableStart] = ' ';
            removeSpaces(tableName);



            /*
             * datatype can only be Strings
             */
            /*
             * reading after " ("
             */
            i = i+6;
            //read in schema
            LinkedList schema = new_LinkedList();
            while(command[i-1]!=')') {
                int startSchema = i;
                int endSchema = i+1;
                while (command[endSchema]!=',' && command[endSchema]!=')') {
                    endSchema++;
                }
                char* attr = (char*) calloc(10, sizeof(char));
                strncpy(attr, command + startSchema, endSchema - startSchema);

                removeSpaces(attr);
                attr[endSchema-startSchema] = '\0';
                LinkedList_add_at_end(schema, attr);
                i=endSchema+1;
            }
            //LinkedList_print_string_list(schema);
            LinkedList keys = new_LinkedList();
            LinkedList subKeys = new_LinkedList();
            for (int a = 0;a< LinkedList_size(schema); a++) {
                LinkedList_add_at_end(subKeys, INT2VOIDP(a));
            }

            LinkedList_add_at_end(keys, subKeys);

            INDEX make = new_INDEX(schema,keys, 100, tableName);
            printRelationContents(make,0);

            addRelation(d, make);
            writeDataFile(d, "databaseOUT.txt");
            return;


        }


        else if(!strcasecmp(operation, "INSERT")){



            i = titleEnd+6;

            int tableStart = i;
            int tableEnd = i+1;

            while (command[tableEnd]!='(') {
                tableEnd++;
            }
            char* tableName = (char*) calloc(10, sizeof(char));
            strncpy(tableName, command + tableStart, tableEnd - tableStart);
            tableName[tableEnd-tableStart] = ' ';
            //printf("asdasd");
            //printf("\n\n%s\n", tableName);
            removeSpaces(tableName);
            INDEX temp_Index = getRelation(d, tableName);

            i = i+titleEnd;

            LinkedList tuple = new_LinkedList();
            while(command[i-1]!=')') {
                int startSchema = i;
                int endSchema = i+1;
                while (command[endSchema]!=',' && command[endSchema]!=')') {
                    endSchema++;
                }
                char* attr = (char*) calloc(10, sizeof(char));
                strncpy(attr, command + startSchema, endSchema - startSchema);

                removeSpaces(attr);
                attr[endSchema-startSchema] = '\0';

                LinkedList_add_at_end(tuple, attr);

                i=endSchema+1;
            }
            //LinkedList_print_string_list(tuple);
            insert(temp_Index, tuple);
            printRelationContents(temp_Index,0);
            addRelation(d,temp_Index);
            writeDataFile(d, "databaseOUT.txt");
            return;

        }

        else if(!strcasecmp(operation, "DELETE")) {


            i = titleEnd + 1;
            int tableStart = i;
            int tableEnd = i + 1;
            if (command[i] == '*') {
                /*
                 * getting rid of from
                 */
                tableStart = tableStart + 7;


                while (command[tableEnd] != '\n') {
                    tableEnd++;
                }
                char *tableName = (char *) calloc(10, sizeof(char));
                strncpy(tableName, command + tableStart, tableEnd - tableStart);
                tableName[tableEnd - tableStart] = ' ';
                //printf("\n\n\n\n\n%s\n\n\n\n", tableName);
                removeSpaces(tableName);
                INDEX temp_Index = getRelation(d, tableName);
                LinkedList param = new_LinkedList();
                for (int a = 0; a < LinkedList_size(temp_Index->schema); a++) {
                    LinkedList_add_at_end(param, "*");
                }
                delete(temp_Index, param);
                addRelation(d, temp_Index);
                writeDataFile(d, "databaseOUT.txt");
                return;
            } else {
                return;
            }
        }

            else if(!strcasecmp(operation, "SELECT")) {


            i = titleEnd + 1;
            int tableStart = i;
            int tableEnd = i + 1;
            if (command[i] == '*') {
                /*
                 * getting rid of from
                 */
                tableStart = tableStart + 7;


                while (command[tableEnd] != '\n') {
                    tableEnd++;
                }
                char *tableName = (char *) calloc(10, sizeof(char));
                strncpy(tableName, command + tableStart, tableEnd - tableStart);
                tableName[tableEnd - tableStart] = ' ';
                //printf("\n\n\n\n\n%s\n\n\n\n", tableName);
                removeSpaces(tableName);
                INDEX temp_Index = getRelation(d, tableName);
                LinkedList param = new_LinkedList();
                for (int a = 0; a < LinkedList_size(temp_Index->schema); a++) {
                    LinkedList_add_at_end(param, LinkedList_elementAt(temp_Index->schema,a));
                }

                printRelationContents(projection_operation(temp_Index, param), 0);
                return;
                //LinkedList_add_at_end(d, temp_Index);
                //writeDataFile(d, "databaseOUT.txt");
            } else {
                return;
            }
        }
    }
}
