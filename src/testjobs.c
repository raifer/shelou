
/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include<stdint.h>
#include "jobs.h"
#include "variante.h"
#include "readcmd.h"
#ifndef VARIANTE
#error "Variante non défini !!"
#endif

/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */

#if USE_GUILE == 1
#include <libguile.h>

CM executer_wrapper(SCM x)
{
	return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif


void terminate(char *line) {
#if USE_GNU_READLINE == 1
	/* rl_clear_history() does not exist yet in centOS 6 */
	clear_history();
#endif
	if (line)
		free(line);
	printf("exit\n");
	exit(0);
}
/*Fonction qui exécute la commande passé en parametre
 * Retourne -1 si erreur*/


pid_t execute(struct cmdline *l) {

	// Variables pour récupérer le status et le pid du fils qui relache le wait() du père.
	int status;
	pid_t pidEnd;
        
        if ( !l->bg ) { // Si la champs background n'a pas été modifié
                        // on doit attendre la mort du processus fils
	        pid_t pidChild = fork();

	        switch (pidChild) {
	        case -1 :
	        	perror("Erreur lors de la création du processus fils :");
	        	return EXIT_FAILURE;
	        	break;

	        	// Child
	        case 0:
	        	// execvp : v pour tableau de variable et p pour chercher dans le path.
	        	execvp(l->seq[0][0], l->seq[0]);
	        	perror("Erreur d'exec");
	        	abort (); // Envoit le signal SIGABRT au père.
	        	break;

	        	// Father
	        default:
	        	//printf("\nLe pid du fils est %d\n", pidChild);
	        	pidEnd = waitpid(pidChild,&status,0);
	        	if (pidEnd == -1) {
	        		perror("Wait error :");
	        		return EXIT_FAILURE;
	        	} else {
	        		if (WIFEXITED(status)) printf("Le fils %d c'est terminé avec success avec comme code de retour %d\n", pidEnd, WEXITSTATUS(status));
	        		else printf("Le fils %d a rencontrer une erreur\n", pidEnd);

	        		if (WIFSIGNALED(status)) printf("Le fils %d c'est terminé à cause du signal n° %d : %s\n", pidEnd, WTERMSIG(status), strsignal(WTERMSIG(status)));
		// printf("Wait à  terminé avec la fin du fils %d\n", pid_end);
        	                return pidChild;
                	}
        	}
        }
        else { // on lance une tache de fond
                pid_t pidChild = fork();

	        switch (pidChild) {
	        case -1 :
	        	perror("Erreur lors de la création du processus fils :");
	        	return EXIT_FAILURE;
	        	break;

	        	// Child
	        case 0:
	        	// execvp : v pour tableau de variable et p pour chercher dans le path.
	        	execvp(l->seq[0][0], l->seq[0]);
	        	perror("Erreur d'exec");
	        	abort (); // Envoit le signal SIGABRT au père.
	        	break;

	        	// Father
	        default: 
	        	printf(" %d\n", pidChild);
                        break;
        	}
        	return EXIT_SUCCESS;         
        }
}

int main() {
        struct cmdline *l;
	char *tab[] = {"sleep 1 &", "sleep 2 &", "sleep 3 &"};
	pid_t result ;
        List *jobs = NULL;
        for ( int j = 0; j < 3; j++) {  
	        l = parsecmd( &tab[j]);
	        result = execute(l);
                if(l->bg && result > 0) {
                        create_job(result, tab[j], j, &jobs);  ;
                }
        }
        free_list(jobs);
}
