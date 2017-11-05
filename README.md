# Spécifications de l'implémentation du code

## Lancement et enchaînement des commandes

Création de la fonction execute dans le fichier execute.c
Fonction qui exécute la commande passée en paramètre et retourne -1 si erreur.

Méthode de résolution:
- création d'un pid child qui récupère la valeur du fork:
        - Si on est dans le fils, on exécute la commande (fonction execvp)
        - Si on est dans le père, on attend (ou non si commande se finit par &). 
- Vérification des signaux envoyés par le fils

## Lancement en tâche de fond

La fonction execute permet de considérer une commande lancée en tâche de fond, on teste la valeur du champs bg de la structure cmdline passée en paramère. 

## Commmande interne jobs

### Idée de résolution:
Création d'une liste chaînée (toutes les fonctions nécessaires se trouvent dans le fichier jobs.c) qui enregistre le pid, le numéro d'appel de jobs, le nom de la commande.

### Explications de la méthode employée:
- Implémentation de toutes les fonctions nécessaires à une liste chaînée
        - création d'une liste
        - ajout d'éléments
        - suppression d'éléments
        - destruction de la liste
 
- Création de la fonction printJobs
Boucle while qui lit tous les jobs créés.
Une variable enregistre l'état du pid du job (waitpid)
        - si le pid n'existe plus, on affiche un message puis on supprime le job en question de la liste chainée
        - si le pid n'a pas changé depuis le dernier appel à la fonction, on affiche l'état Running puis parcourt de la liste
        - sinon le fils vient de se terminer et on parcourt les autres jobs.

## Les pipes

Création d'une fonction executeLine qui lance l'exécution de la commande fournie au prompte (C'est cette fonction qui est appelée dans le main).

Idée: 
On compte le nombre de pipes pour créer un tableau de pipes de la dimension à 2 fois le nombre de pipes. Puis, par la redirection des entrées sorties, on exécute chaque commande (séparée par | ).


ensimag-shell
=============

Squelette pour le TP shell

Tous les fichiers sont sous licence GPLv3+

Introduction
----------

Ces fichiers servent de base à un TP sur le shell de commande à la Unix.

Spécificités de ce squelette:
- plusieurs variantes (libre choix par les étudiants)
- jeux de tests fournis aux étudiants
- utilisation de Gnu Readline
- Scheme (interpréteur Guile; Javascript possible)

Compilation et lancement des tests
----------

cd ensimag-shell
cd build
cmake ..
make
make test



Autres
------

Les premières versions imposaient la variante par un modulo sur le
sha512 sur de la liste triée des logins des étudiants. Cela peut être
réalisé complètement en CMake (>= 2.8).
