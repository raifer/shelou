/*
 * global.h
 *
 *  Created on: 5 nov. 2017
 *      Author: raifer
 */

#ifndef SRC_GLOBAL_H_
#define SRC_GLOBAL_H_

// Variables globales
// List chaînée des jobs.
        extern List *jobs_g;
// Mutex pour la liste des jobs
        extern pthread_mutex_t m_jobs;

#endif /* SRC_GLOBAL_H_ */
