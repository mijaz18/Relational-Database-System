/*
 * Query.h
 *
 *  Created on: Nov 22, 2018
 *      Author: mijaz
 */

#ifndef QUERY_H_
#define QUERY_H_

#include "Database.h"
#include "Index.h"

typedef char* String;

extern INDEX whatGrade(Database d);
extern INDEX whereIs(Database d);
extern void query_REPL(Database d);

#endif /* QUERY_H_ */
