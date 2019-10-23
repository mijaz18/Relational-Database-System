/*
 * Operations.h
 *
 *  Created on: Nov 22, 2018
 *      Author: mijaz
 */

#include "Index.h"

#ifndef OPERATIONS_H_
#define OPERATIONS_H_

extern INDEX selection_operation(INDEX idx, LinkedList parameters);
extern INDEX join_operation(INDEX idx1, String attr1, INDEX idx2, String attr2);
extern INDEX projection_operation(INDEX idx, LinkedList projection);
extern INDEX union_operation (INDEX idx1, INDEX idx2);
extern INDEX intersection_operation(INDEX idx1, INDEX idx2);
extern INDEX difference_operation(INDEX idx1, INDEX idx2);

#endif /* OPERATIONS_H_ */
