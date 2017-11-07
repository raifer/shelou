#ifndef _JOBS_H
#define _JOBS_H

#include <sys/types.h>
#include <stdint.h>

/* Structure to save commands, used by intern command jobs */
typedef struct S_job Job;
struct S_job {
    char *cmd;	/* If not null : name of command written */
    uint16_t id;       /* Save the number of the call-system in background */
    pid_t pid;	/* Save the pid of process*/
    int status; // Permet de sauvegarder le status renvoyer par waitpid pour un affichage par la commande jobs
    int *pipes; // Poiteur vers le tableau des pipes qui permet de free lorssque le job est terminé.
} ;

typedef struct S_List List;
struct S_List{
    List *next;
    List *previous;
    Job *job;
} ;



/*fonction pour la liste*/
void add_job(pid_t pid, char *cmd, uint16_t idJob, List **p_jobs, int *pipes);
void free_list(List *liste);
void print_jobs(List **jobs);
void *asynchronous_print_thread(void* arg);

#endif
