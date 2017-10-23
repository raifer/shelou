#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jobs.h"

#define CMD_MAX 200

void free_elem(List *p_liste) ;
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
        old->previous = list;
}
return list;
}

void del_jobs(List **p_liste) {
if (! (*p_liste)->previous) { //premier elem de la liste
        List *list_return = (*p_liste)->next;
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

void create_job(pid_t pid, char *cmd, int id, List **jobs) {
        Job *j = malloc(sizeof(Job));
        j->pid = pid;
        j->id = id;
        strncpy(j->cmd, cmd, CMD_MAX);
        *jobs = list_prepend(*jobs, j); 
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
        free(p_liste->job);
        free(p_liste);
}


void print_jobs(List *jobs){
	if (jobs == NULL){
		printf("No job\n");
		return;}
	while(jobs != NULL) {
		printf("[%d] : %s\n", jobs->job->id, jobs->job->cmd);
		jobs = jobs->next;
	}
}
