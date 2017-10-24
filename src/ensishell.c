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

#include "variante.h"
#include "readcmd.h"
#include "jobs.h"

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
int execute(struct cmdline *l) {

	// Variables pour récupérer le status et le pid du fils qui relache le wait() du père.
	int status;
	pid_t pidEnd;

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
		// Si demande de background on attend pas la fin du fils
		if (l->bg) return pidChild;
		pidEnd = waitpid(pidChild,&status,0);
		if (pidEnd == -1) {
			perror("Wait error :");
			return EXIT_FAILURE;
		} else {
			if (WIFEXITED(status)) printf("Le fils %d c'est terminé avec success avec comme code de retour %d\n", pidEnd, WEXITSTATUS(status));
			else printf("Le fils %d a rencontrer une erreur\n", pidEnd);

			if (WIFSIGNALED(status)) printf("Le fils %d c'est terminé à cause du signal n° %d : %s\n", pidEnd, WTERMSIG(status), strsignal(WTERMSIG(status)));
			// printf("Wait à  terminé avec la fin du fils %d\n", pid_end);
			return EXIT_SUCCESS;
		}
	}
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
	int nombreBG = 1;

	while (1) {
		struct cmdline *l;
		char *line=0;
		char *prompt = "shelou>";

		// pid du fils dans le cas d'un jobs
		pid_t pid;

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);

		// Si erreur ou mots clef exit
		if (line == 0 || ! strncmp(line,"exit", 4)) {
			terminate(line);
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
			print_jobs(jobs); 
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
			terminate(0);
		}



		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			//free(cmd);
			continue;
		}

		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);

		pid = execute(l);
		if(l->bg && pid > 0) {
            char cmd[200] ; 
            for (int i = 0; l->seq[i]!=0; i++) {
                char **cat = l->seq[i];
                for( int j = 0; cat[j]!=0; j++) {
                    if ( j == 0){
                         strcpy(cmd, cat[j]); //on copie le premier mot
                    }
                    else {
                        strcat(cmd," "); //on ajoute un espace pour separ
                                         //les mots 
                        strcat(cmd, cat[j]);//on complete la contenation
                    }
                }
                strcat(cmd," &"); //par defaut le caractere n'apparait
                                  //pour une representation fidele, ajout
                                  //a la main du caracte &
			    printf("[%d], %s\n",nombreBG, cmd);
            }
			create_job(pid, cmd, nombreBG, &jobs);
			nombreBG++;
        } //else free(cmd); // on a pas besoin de conserver cmd.
	} // end while
	free_list(jobs);
}
