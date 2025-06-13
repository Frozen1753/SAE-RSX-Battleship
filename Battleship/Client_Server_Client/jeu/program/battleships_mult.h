#ifndef BATTLESHIPS_MULT_H
#define BATTLESHIPS_MULT_H

#define DIM 10

typedef struct {
    char symbole;
    int taille;
    int restants;
    int actif;
} Bateau;

void initialiserGrille(char grille[DIM][DIM]);
void afficherGrille(char grille[DIM][DIM], int cacher);
int lettreVersIndice(char lettre);
int valides(int i, int j);
int peutPlacer(int taille, int rot, int i, int j, char grille[DIM][DIM]);
void placerBateau(int taille, int rot, int i, int j, char symbole, char grille[DIM][DIM]);
void placement(char grille[DIM][DIM], int joueur, Bateau flotte[]);
void sauvegarderGrille(char* buffer, char grille[DIM][DIM]);
void chargerGrille(const char* buffer, char grille[DIM][DIM]);

#endif
