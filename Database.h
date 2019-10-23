/*
 * Database.h
 *
 *  Created on: Nov 17, 2018
 *      Author: mijaz
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#include "Index.h"

typedef struct Database* Database;

extern Database new_Database();
extern void addRelation(Database d, INDEX i);
extern INDEX getRelation(Database d, char* name);

#endif /* DATABASE_H_ */
