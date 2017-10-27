/*
 * execute.h
 *
 *  Created on: 27 oct. 2017
 *      Author: raifer
 */

#ifndef SRC_EXECUTE_H_
#define SRC_EXECUTE_H_

#include "readcmd.h"
#include "jobs.h"

// Execute la comande sous forme cmdline
// return EXIT_FAILURE si erreur fatale
int execute_line(struct cmdline *l, List *jobs, int idBG);


#endif /* SRC_EXECUTE_H_ */
