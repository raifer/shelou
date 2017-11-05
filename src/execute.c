/*
 * execute.c
 *
 *  Created on: 27 oct. 2017
 *      Author: raifer
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "variante.h"
#include "readcmd.h"
#include "jobs.h"
#include "execute.h"

// Mutex extern pour gérer les jobs
extern pthread_mutex_t m_jobs;

// Proto
int execute(char **seq, int in, int out, int bg);

/**
 * Renvoie le nombres de pipes utilisés dans la commande
 */
uint8_t get_nb_pipes(struct cmdline *l) {
	uint8_t n = 0;
	while(l->seq[n++]!=0);
	return n-2;
}

/**
 * Lance l'ex"exécution de la commande fournie au prompte
 */
int execute_line(struct cmdline *l, List **p_jobs, int idJob) {
	// Descripteurs des fichier
	int infd, outfd;

	// Pid du fils dans le cas d'un jobs
	pid_t pid;

	// Poiteur vers le tableau de pipes
	int *pipes = NULL;

	// Attribution de stdin et stdout
	if(l->out == NULL) {
		// Pas de redirection, on prend stdout
		outfd = 1;
	} else {
		/* Redirection vers un fichier
		 * On ouvre le fichier en écriture,
		 * Si il n'existe pas, on le créer,
		 * si il existe, on le tronque
		 */
		outfd = open(l->out, O_WRONLY | O_TRUNC | O_CREAT);
		if (outfd == -1) {
			perror("Erreur lors de l'ouverture");
			fprintf(stderr, "Fichier : '%s'\n", l->in);
			return EXIT_FAILURE;
		}
	}

	// In
	if(l->in == NULL) {
		// Pas de redirection, on prend stdin
			infd = 0;
		} else {
			/* On redirige l'entré standar depuis un fichier
			 * On ouvre le fichier en lecture seul,
			 * Si il existe pas : erreur
			 */
			infd = open(l->in, O_RDONLY);
			if (infd == -1) {
				fprintf(stderr, "Fichier : '%s'\n", l->in);
				perror("");
				return EXIT_FAILURE;
			}
		}


	uint8_t nb_pipes = get_nb_pipes(l);
	// Si commande simple sans pipe
	if(nb_pipes == 0){
		pid = execute(l->seq[0], infd, outfd, l->bg);
		if (pid == EXIT_FAILURE) return EXIT_FAILURE;
	}

	// Execution de plusieurs programmes liés par des pipes
	else {
		// Création des pipes
		pipes = malloc(sizeof(int)*nb_pipes*2);
		if (pipes == NULL) {
			perror("Error lors du malloc du tableau de pipes");
			return EXIT_FAILURE;
		}
		for (int i = 0; i < nb_pipes; i++) {
			if( pipe(&(pipes[i*2])) == -1) {
				perror("Erreur lors de la creation du pipe");
				free(pipes);
				return EXIT_FAILURE;
			}

			// On switch les l'entré sortie pour avoir l'entré en premier dans le tableau
			int sav = pipes[i*2];
			pipes[i*2] = pipes[i*2+1];
			pipes[i*2+1] = sav;
		}

		// Programme 0
		pid = execute(l->seq[0], infd, pipes[0], 1);
		if (pid == EXIT_FAILURE) {
			free(pipes);
			return EXIT_FAILURE;
		}

		// Programme de 1 à n-2
		for(int i = 1; i < nb_pipes; i++){
			pid = execute(l->seq[i], pipes[i*2-1], pipes[i*2], 1);
			if (pid == EXIT_FAILURE) {
				free(pipes);
				return EXIT_FAILURE;
			}
		}
		// Programme n-1
		pid = execute(l->seq[nb_pipes], pipes[nb_pipes*2-1], outfd, l->bg);
		if (pid == EXIT_FAILURE) {
			free(pipes);
			return EXIT_FAILURE;
		}
	}

	// Si demande de traitement en arrière plan, on créer un jobs
	if(l->bg && pid > 0) {
		char *cmd = get_cmd_line(l);
		// On prend le mutex sur la liste des jobs
		pthread_mutex_lock(&m_jobs);
		add_job(pid, cmd, idJob, p_jobs, pipes);
		pthread_mutex_unlock(&m_jobs);
	}
	else {
		// Les pgs sont finis, on free les pipes.
		if (pipes != NULL) free(pipes);
	}
	return EXIT_SUCCESS;
}


/*Fonction qui exécute la commande passé en parametre
 * Retourne -1 si erreur*/
int execute(char **seq, int in, int out, int bg) {

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
		/* Modification des I/O
		 * dup2 close new avant de copier
		 * dup2 ne ferme pas si les deux params sont identiques.
		 **/
		if( -1 == dup2(in, STDIN_FILENO) ) perror("dup2(in, stdin) error");
		if( -1 == dup2(out, STDOUT_FILENO) ) perror("dup2( out, stdout) perror");

		/* Remplacement du programme
		 * execvp : v pour tableau de variable et p pour chercher dans le path.
		 */
		execvp(seq[0], seq);
		perror("Erreur d'exec");
		abort (); // Envoit le signal SIGABRT au père.
		break;

		// Father
	default:
		//printf("\nLe pid du fils est %d\n", pidChild);
		// fermetture des pipes
		if (in != STDIN_FILENO) close(in);
		if (out != STDOUT_FILENO) close(out);

		// Si demande de background on attend pas la fin du fils
		if (bg) return pidChild;

		pidEnd = waitpid(pidChild,&status,0);
		if (pidEnd == -1) {
			perror("Wait error :");
			return EXIT_FAILURE;
		}
		if (WIFEXITED(status)) {
				//printf("Le fils %d c'est terminé avec success avec comme code de retour %d\n", pidEnd, WEXITSTATUS(status));
			} else {
				if (WIFSIGNALED(status)) printf("Le fils %d c'est terminé avec une erreur, signal reçu n° %d : %s\n", pidEnd, WTERMSIG(status), strsignal(WTERMSIG(status)));
			}
			return EXIT_SUCCESS;
	} // end switch
} // end execute


