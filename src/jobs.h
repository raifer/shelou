#ifndef _JOBS_H
#define _JOBS_H

#include <sys/types.h>
#include <stdint.h>
/* Structure to save commands, used by intern command jobs */
typedef struct S_List List;
typedef struct S_job Job;
struct S_job {
        char *cmd;	/* If not null : name of command written */
        uint16_t id;       /* Save the number of the call-system in background */ 
	pid_t pid;	/* Save the pid of process*/
} ;
struct S_list{
	List *next;
        List *previous;
        Job *jobs;
} ;

/*fonction pour la liste*/
void free_list(List *liste);
void create_job(pid_t pid, char *cmd, int id, List **jobs);
//void print_jobs(struct cmdjobs j) ;

#endif