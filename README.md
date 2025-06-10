# Bataille Navale (Battleship)

Ce projet est une implémentation en C du jeu de la bataille navale en mode console, pour deux joueurs en local.

## Fonctionnement

- Chaque joueur place ses bateaux sur sa grille.
- Les joueurs jouent chacun leur tour pour tirer sur la grille adverse.
- Le jeu affiche les grilles et indique les touches, les ratés et les bateaux coulés.
- Le premier joueur à couler tous les bateaux adverses gagne la partie.

## Compilation

Assurez-vous d'avoir `gcc` installé.

Dans le dossier du projet, compilez avec :
gcc -o bataille_navale bataille_navale.c

## Utilisation

Lancez le jeu avec : ./bataille_navale
Suivez les instructions à l'écran pour placer vos bateaux et jouer.

## Remarques

- Ce projet fonctionne en local, en mode texte.
- Pour une version réseau, il faudrait ajouter un serveur et des clients utilisant les sockets.

