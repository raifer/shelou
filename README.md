% Projet Shell : Shelou
% Gautier Fidry, Mathieu Barbe
% mardi 7 novembre 2017

# Informations

- Nous avons choisi la variante 6 du sujet;
- Utilisation de Git pour la révision de code.

# Spécifications de l'implémentation du code

## Lancement et enchaînement des commandes

L'ensemble des fonctions gérant l'exécution sont regroupés dans le fichier execute.c.
Deux fonctions vont réaliser l'exécutions des commandes :

* execute_line :   Fonction qui exécute la commande passée en paramètre et retourne -1 si erreur;
* execute : Fonction qui lance un programme dans un fils.

### Particularité de notre implémentation

* Gestion fine des erreurs qui se propage dans l'appelant;
* Gestion des signaux de retour des fils pour voir comment c'est terminé le programme;

## Jobs

Afin de gérer efficacement la liste des jobs, nous avons réfléchi à la meilleur structure de donné pour les stoker.
Une liste doublements chaînée a été choisie.
L'ensemble de ces fonctions sont regroupées dans jobs.c.

### Structure jobs

struct S_job {
        char *cmd;	/* If not null : name of command written */
        uint16_t id;       /* Save the number of the call-system in background */
	pid_t pid;	/* Save the pid of process*/
	int status; // Permet de sauvegarder le status renvoyer par waitpid pour un affichage par la commande jobs
	int *pipes; // Poiteur vers le tableau des pipes qui permet de free lorsque le job est terminé.
} ;


### Détail sur la fonction printJobs

- Boucle while qui lit tous les jobs créés.
- Une variable enregistre l'état du pid du job (waitpid en mode non bloquant)
        - si le pid n'existe plus, on affiche un message puis on supprime le job en question de la liste chainée
        - si le pid n'a pas changé depuis le dernier appel à la fonction, on affiche l'état Running puis parcourt de la liste
        - sinon le fils vient de se terminer et on parcourt les autres jobs.

## Les pipes

- La gestion des pipes est faite dans la fonction execute_line du fichier execute.c;
- A partir du nombre de pipes nécessaires à l'éxécution de la commande, un tableau des pipes est généré;
- Le premier programme a donc stdin en entré et le pipe[0] en sortie;
- Le programme i, avec i entre 2 jusqu'à n-2 à pour entrer pipe{i-1] et en sortie pipe{i];
- Le programme n à en entrer pipe{n-1] et en sortie stdout 

## Terminaisons asynchrone

Pour gérer les terminaisons des jobs de manière asynchrone une fonction traitante est appelée à l'arrivé d'un signal SIGCHLD.
Celle-ci lance un thread pour gérer l'affichage de l'information.
Dans ce thread (localisé dans jobs.c), on récupère le pid qui nous a envoyé le signal et on parcourt la liste des jobs pour trouver des informations complémentaires à afficher.
Un mutex est utilisé afin d protéger d'un accès concurrent de la liste, car on ne peut prédire quand le traitant sera appelé.


# Remarque sur le sujet

Pour les tests, il est nécessaire de rajouter les droits en exécutions sur les fichiers ruby.
Peut-être changer la méthode de lancement des tests, avec : ruby path plutôt que ./test.
