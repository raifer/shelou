#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>

#include "jobs.h"

// Récupération de le mutex global pour gérer la liste des jobs
extern pthread_mutex_t m_jobs;

// Constante indiquant le nombre max de caractère pour une commande, utilisée pour des questions de sécurité.
#define CMD_MAX 200

// Prototype
void free_elem(List *p_liste) ;

/**
 * Créer une liste d'un seul élément
 * lors de la création, le job passé en paramètre est placé dans l'élément
 */
List *create_liste(Job *job) {
    List *list = malloc(sizeof(list));
    if (list && job) {
        list->previous = NULL;
        list->next = NULL;
        list->job = job;
    }
    return list;   
}


/**
 * Ajoute un élément en tête de liste
 * dans ce noubelle élément, la référence du job sera enregistrée,
 * le pointeur vers la liste sera mis à jour.
 */
List *list_prepend(List *old, Job *jobs){
    List *list = create_liste(jobs);
    if (list){
        list->next = old;
        list->previous = NULL;
        if (old != NULL) old->previous = list;
    }
    return list;
}

/**
 * Supprime et libère un élément de la liste,
 * la liste est reconstitué,
 * le si on sup le 1er élément, le pointeur de liste est donc mis à jour.
 */
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

/**
 * Créer un job et l'ajoute en tête de liste,
 *
 *@param pid_t pid : Le pid du job;
 *@param char *cmd : La commande demandé par l'utilisateur;
 *@param int uint16_t idJobs : L'id du job;
 *@param List **p_jobs : Un poiteur vers la liste des jobs, qui pourra être mis à jour;
 *@param int *pipes : Poiteur vers le tableau contenant les pipes du job, NULL si pas de pipe.
 */
void add_job(pid_t pid, char *cmd, uint16_t idJob, List **p_jobs, int *pipes) {
    printf("[%d], %s\n",idJob, cmd);
    Job *j = malloc(sizeof(Job));
    j->pid = pid;
    j->id = idJob;
    j->pipes = pipes;
    j->cmd = cmd;
    *p_jobs = list_prepend(*p_jobs, j);
}

/**
 * Libère toute la liste passée en paramètre,
 * ainsi que son contenu.
 */
void free_list(List *liste) {
    for(List *cur = liste; cur != NULL; ){
        List *next = cur->next;
        free_elem(cur);
        cur=next;
    }
}

/**
 * Libère un élément de la liste ainsi que :,
 * job->cmd;
 * job->pipes;
 * job.
 */
void free_elem(List *p_liste) {
    free(p_liste->job->cmd);
    if (p_liste->job->pipes != NULL)
        free(p_liste->job->pipes);
    free(p_liste->job);
    free(p_liste);
}

/**
 * Affiche les jobs en cours,
 *
 * Si pas de jobs, on affiche "No job".
 *
 * Si un job est en cours :
 * 	- On indique son état.
 *
 * Si un job est terminé :
 * 	- On indique qu'il a terminé;
 * 	- on renseigne sur son code de retour;
 * 	- on le supprime de la liste des jobs.
 */
void print_jobs(List **p_jobs){
	// Si pas de job
    if (*p_jobs == NULL){
        printf("No job\n");
        return;}

    // Parcour des jobs
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

/**
 * Thread lancé à la réception du signal SIGCHLD
 * récupération du pid du job qui c'est terminé
 * parrcour de la liste des jobs pour trouver les infos complémentaires
 * affiche des infos sur le job se terminant.
 * enregistre le status de retour du job dans la liste des jobs.
 */
void *asynchronous_print_thread(void* jobs) {
	printf("thread\n");
	fflush(stdout);
    pid_t pid = 0;
    int status = 0;

    // On regarde si un fils à belle et bien finit
//    pid = waitpid(0, &status, WNOHANG);

    // Si plus de fils vivant ou aucun fils n'a transmis de signal
    if (pid > 0) {
        // On a été appelé par un fils qui était en arrière plan et quie a terminé.
        List *j = jobs;
        // On prend le mutex pour parccourir la liste chaînée des jobs
        if (pthread_mutex_lock(&m_jobs) == -1) perror("Asynchrone_print_thread, eErreur lors de la prise du mutex m_jobs");
        while (j != NULL) {
            if (pid == j->job->pid)
                printf("\nLe job %d : '%s', vient  de se terminer", j->job->id, j->job->cmd);
            // On enregistre le statu de retour dans le job pour affichage ultérieur
            j->job->status = status;
            j = j->next;
        }
        // Relachement du mutex
        if (pthread_mutex_unlock(&m_jobs) == -1) perror("Asynchrone_print_thread, erreur lors du relachement du mutex m_jobs");

        if (WIFEXITED(status)) {
            printf(" correctement avec comme code de retour %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            printf(" avec une erreur, signal reçu n° %d : %s\n", WTERMSIG(status), strsignal(WTERMSIG(status)));
        }
        printf("> ");
        fflush(stdout);
    }
    pthread_exit(NULL);
}

