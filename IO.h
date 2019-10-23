/*
 * IO.h
 *
 *  Created on: Nov 20, 2018
 *      Author: mijaz
 */

#ifndef IO_H_
#define IO_H_

#include "Index.h"
#include "Database.h"

extern void writeDataFile(Database d, String filepath);
extern void writeDataFileHelper(INDEX index, FILE* fp);
extern Database readDataFile(String path);
extern int stringToInt(String s);
extern void printDatabaseToConsole(Database d);

#endif /* IO_H_ */
