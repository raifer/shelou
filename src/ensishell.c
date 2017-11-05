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
#include <pthread.h>

#include "variante.h"
#include "readcmd.h"
#include "jobs.h"
#include "execute.h"

// Variables globales
// Mutex pour la liste des jobs
       pthread_mutex_t m_jobs;
// Variable global limitée à ce fichier
       // pointeur sur la liste des jobs
               List *jobs = NULL;

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

int question6_executer(char *line)
{
        /* Question 6: Insert your code to execute the command line
         * identically to the standard execution scheme:
         * parsecmd, then fork+execvp, for a single command.
         * pipe and i/o redirection are not required.
         */
        printf("Not implemented yet: can not execute %s\n", line);

        /* Remove this line when using parsecmd as it will free it */
        free(line);

        return 0;
}

SCM executer_wrapper(SCM x)
{
        return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif


void terminate(char *line, List *jobs) {
#if USE_GNU_READLINE == 1
        /* rl_clear_history() does not exist yet in centOS 6 */
        clear_history();
#endif
        if (jobs)
        	free_list(jobs);
        if (line)
                free(line);
        // Destruction du mutex
        pthread_mutex_destroy(&m_jobs);
        printf("exit\n");
        exit(0);
}

void handle_sigchld(int sig) {
	pthread_t thread1;
	if(pthread_create(&thread1, NULL, asynchronous_print_thread, jobs) == -1) {
		perror("pthread_create");
	}
	return;
}

int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
        scm_init_guile();
        /* register "executer" function in scheme */
        scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

        // nb d'appels en arrière-plan (pour affichage)
        int idJob = 1;

        // Création du handle qui va gérer la fin des jobs, question 7.4
        signal(SIGCHLD, handle_sigchld);
        // Initialisation du mutex jobs
        // Bug avec la ligne suivante, je ne sais pas pourquoi.
//        m_jobs = PTHREAD_MUTEX_INITIALIZER;

        while (1) {
                struct cmdline *l;
                char *line=NULL;
                char *prompt = "shelou>";

                // pid du fils dans le cas d'un jobs

                /* Readline use some internal memory structure that
                   can not be cleaned at the end of the program. Thus
                   one memory leak per command seems unavoidable yet */
                line = readline(prompt);

                // Si erreur ou mots clef exit
                if (line == 0 || ! strncmp(line,"exit", 4)) {
                        terminate(line, jobs);
                }

                // Si aucune commande a été entrée.
                if (line[0] == 0){
                        //printf("pas de commande\n");
                        free(line);
                        continue;
                }

#if USE_GNU_READLINE == 1
                add_history(line);
#endif


                // Si la commande jobs est appelé
                if (! strncmp(line, "jobs", 4)){
                	// On prend le mutex m_jobs
                	pthread_mutex_lock(&m_jobs);
                        print_jobs(&jobs);
                        pthread_mutex_unlock(&m_jobs);
                        free(line);
                        continue;
                }

#if USE_GUILE == 1
                /* The line is a scheme command */
                if (line[0] == '(') {
                        char catchligne[strlen(line) + 256];
                        sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
                        scm_eval_string(scm_from_locale_string(catchligne));
                        free(line);
                        continue;
                }
#endif

                // Copy de la commande pour ne pas la perdre dans parse
                //char *cmd = NULL ; 
                // cmd = strcpy(cmd, line);
                //char *cmd = "cmd generic";

                /* parsecmd free line and set it up to 0 */
                l = parsecmd( & line);

                /* If input stream closed, normal termination */
                if (!l) {
                        //free(cmd);
                        terminate(0, jobs);
                }



                if (l->err) {
                        /* Syntax error, read another command */
                        printf("error: %s\n", l->err);
                        continue;
                }

                execute_line(l, &jobs, idJob);

                // Si c'était une commande en arrière plan, on incrémente le numéro du job.
if (l->bg) idJob++;
        } // end while
}
