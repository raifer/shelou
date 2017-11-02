/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "variante.h"
#include "readcmd.h"
#include "jobs.h"
#include "execute.h"

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
       // char *prompt = "guile";
        struct cmdline *l = parsecmd(&line);
        List *jobs = NULL;
        execute(l->seq[0],0,1,0);
        /* Remove this line when using parsecmd as it will free it */
        //free(line);

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
        printf("exit\n");
        exit(0);
}

int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
        scm_init_guile();
        /* register "executer" function in scheme */
        scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

        // List chaînée des jobs.
        List *jobs = NULL;
        // nb d'appels en arrière-plan (pour affichage)
        int idJob = 1;

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
                //if (line[0] == 'j') { 
                //La commande ci -dessous est plus correcte, car sinon on affiche
                //quand-meme la commande jobs en tapant seulement j
                if (! strncmp(line, "jobs", 4)){ 
                        print_jobs(&jobs);
                        free(line);
                        continue;
                }
                printf("line[] = %c\n",line[0]);

#if USE_GUILE == 1
                /* The line is a scheme command */
                if (line[0] == '(') {
                	printf("Guile détecté\n");
                        char catchligne[strlen(line) + 256];
                        sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
                        printf("catchligne : '%s'\n", catchligne);
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

                execute_line(l, &jobs, idJob++);

        } // end while
}
