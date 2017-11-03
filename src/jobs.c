#include "jobs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define CMD_MAX 200

void free_elem(List *p_liste) ;

List *create_liste(Job *job) {
        List *list = malloc(sizeof(list));
        if (list && job) {
                list->previous = NULL;
                list->next = NULL;
                list->job = job;
        }
        return list;   
}

List *list_prepend(List *old, Job *jobs){
        List *list = create_liste(jobs);
        if (list){
                list->next = old;
                list->previous = NULL;
                if (old != NULL) old->previous = list;
        }
        return list;
}
// Supprime un élément de la liste et recole les morceaux.
void del_elem(List **p_liste) {
        //si premier elem de la liste
        if ((*p_liste)->previous == NULL) {
                List *list_return = (*p_liste)->next;
                if (list_return != NULL) list_return->previous = NULL;
                free_elem(*p_liste);
                *p_liste = list_return ;
        }
        else {
                (*p_liste)->previous->next = (*p_liste)->next ;
                if ((*p_liste)->next)
                        (*p_liste)->next->previous = (*p_liste)->previous;
                free_elem(*p_liste);
        }
}

void add_job(pid_t pid, char *cmd, uint16_t idJob, List **p_jobs, int *pipes) {


	                printf("[%d], %s\n",idJob, cmd);
	                        Job *j = malloc(sizeof(Job));
	                        j->pid = pid;
	                        j->id = idJob;
	                        j->pipes = pipes;
	                        j->cmd = cmd;
	                        *p_jobs = list_prepend(*p_jobs, j);
	                }

void free_list(List *liste) {
        for(List *cur = liste; cur != NULL; ){
                List *next = cur->next;
                free_elem(cur);
                cur=next; //HYPER IMPORTANT !! PAS CUR->NEXT!!!!!
                // Lancer Valgrind ./listecgainee
        }
}

void free_elem(List *p_liste) {
        free(p_liste->job->cmd);
        if (p_liste->job->pipes != NULL)
        free(p_liste->job->pipes);
        free(p_liste->job);
        free(p_liste);
}


void print_jobs(List **p_jobs){
        if (*p_jobs == NULL){
                printf("No job\n");
                return;}
        List *jobs = *p_jobs;
        while(jobs != NULL) {
                int status = 0;

                // Fetch Status of child
                pid_t pid = waitpid(jobs->job->pid, &status, WNOHANG);

                switch (pid) {
                        case -1 :
                                /* le fils n'éxiste plus et il n'y a pas de signal en attentes.
                                 * On affiche ça terminaison puis on le  supprime de la liste des jobs
                                 */

                        	// Afichage du job terminé
                        	status = jobs->job->status;
                        	// si le fils s'est terminé normalement,
                        	                                if (WIFEXITED(status)) {
                        	                                        printf("[%d] pid %d | %s | Finish with code %d\n",
                        	                                               jobs->job->id,
                        	                                               jobs->job->pid,
                        	                                               jobs->job->cmd,
                        	                                               WEXITSTATUS(status));
                        	                                }
                        	                                // si le fils s'est terminé à cause d'un signal.
                        	                                if (WIFSIGNALED(status)) {
                        	                                        printf("[%d] pid %d | %s | Finish with error - signal n°%d: %s\n",
                        	                                               jobs->job->id,
                        	                                               jobs->job->pid,
                        	                                               jobs->job->cmd,
                        	                                               WTERMSIG(status),
                        	                                               strsignal(WTERMSIG(status)));
                        	                                }

                        	// Suppression du job
                                // Si on est sur le premier elem, il faudra mettre à jour le pointeur vers la liste
                                if (*p_jobs == jobs){
                                        del_elem(p_jobs);
                                        jobs = *p_jobs;
                                } else // Sinon pas besoin de mettre à jour le pointeur
                                	del_elem(&jobs);
                                break;

                        case 0 :
                                /* Le pid n'a pas changé d'état depuis la dernière fois.
                                 * On le considère actif.
                                 */
                                printf("[%d] pid %d | %s | Running\n", jobs->job->id, jobs->job->pid, jobs->job->cmd);
                                jobs = jobs->next;
                                break;

                        default :
                                /* Le fils c'est terminé depuis la dernière fois.
                                 * comme la gestion asynchrone est utilisé, on ne peut capter le signal ici,
                                 * waitpid aura toujours 0 ou -1 comme code de retour
                                 * on laisse pour restter compatible si on sup l'asynchrone.
                                 */

                                // si le fils s'est terminé normalement,
                                if (WIFEXITED(status)) {
                                        printf("[%d] pid %d | %s | Finish with code %d\n",
                                               jobs->job->id,
                                               jobs->job->pid,
                                               jobs->job->cmd,
                                               WEXITSTATUS(status));
                                }
                                // si le fils s'est terminé à cause d'un signal.
                                if (WIFSIGNALED(status)) {
                                        printf("[%d] pid %d | %s | Finish with error - signal n°%d: %s\n",
                                               jobs->job->id,
                                               jobs->job->pid,
                                               jobs->job->cmd,
                                               WTERMSIG(status),
                                               strsignal(WTERMSIG(status)));
                                }
                                // Passage au job suivant
                                jobs = jobs->next;
                }
        }
}
