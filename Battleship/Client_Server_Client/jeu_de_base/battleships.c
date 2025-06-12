#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define DIM 10
const char ALPHA[] = "ABCDEFGHIJ";

typedef struct {
    char symbole;
    int taille;
    int restants;
    int actif;
} Bateau;

void initialiserGrille(char grille[DIM][DIM]) {
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            grille[i][j] = '-';
        }
    }
}

void afficherGrille(char grille[DIM][DIM], int cacher) {
    printf("  ");
    for (int i = 0; i < DIM; i++) printf("%d ", i);
    printf("\n");

    for (int i = 0; i < DIM; i++) {
        printf("%c ", ALPHA[i]);
        for (int j = 0; j < DIM; j++) {
            if (cacher && strchr("#@%&$", grille[i][j]))
                printf("- ");
            else
                printf("%c ", grille[i][j]);
        }
        printf("\n");
    }
}

int lettreVersIndice(char lettre) {
    lettre = toupper(lettre);
    return (lettre >= 'A' && lettre <= 'J') ? (lettre - 'A') : -1;
}

int valides(int i, int j) {
    return i >= 0 && i < DIM && j >= 0 && j < DIM;
}

int peutPlacer(int taille, int rot, int i, int j, char grille[DIM][DIM]) {
    for (int k = 0; k < taille; ++k) {
        int x = i + (rot == 3 ? k : rot == 1 ? -k : 0);
        int y = j + (rot == 2 ? k : rot == 4 ? -k : 0);
        if (!valides(x, y) || grille[x][y] != '-') return 0;
    }
    return 1;
}

void placerBateau(int taille, int rot, int i, int j, char symbole, char grille[DIM][DIM]) {
    for (int k = 0; k < taille; ++k) {
        int x = i + (rot == 3 ? k : rot == 1 ? -k : 0);
        int y = j + (rot == 2 ? k : rot == 4 ? -k : 0);
        grille[x][y] = symbole;
    }
}

void placement(char grille[DIM][DIM], int joueur, Bateau flotte[]) {
    printf("Placement des bateaux du Joueur %d\n", joueur);
    while (1) {
        int fini = 1;
        for (int k = 0; k < 5; k++) if (flotte[k].actif) fini = 0;
        if (fini) break;

        afficherGrille(grille, 0);
        printf("Choisissez un bateau (1-5) :\n");
        for (int i = 0; i < 5; i++)
            if (flotte[i].actif)
                printf("%d. Bateau taille %d symbole '%c'\n", i + 1, flotte[i].taille, flotte[i].symbole);

        int choix;
        scanf("%d", &choix);
        choix--;
        if (choix < 0 || choix >= 5 || !flotte[choix].actif) continue;

        char lettre;
        int j, rot;
        printf("Lettre (A-J) : ");
        scanf(" %c", &lettre);
        int i = lettreVersIndice(lettre);

        printf("Chiffre (0-9) : ");
        scanf("%d", &j);

        printf("Rotation (1=nord, 2=est, 3=sud, 4=ouest) : ");
        scanf("%d", &rot);

        if (!peutPlacer(flotte[choix].taille, rot, i, j, grille)) {
            printf("Placement invalide.\n");
            continue;
        }

        placerBateau(flotte[choix].taille, rot, i, j, flotte[choix].symbole, grille);
        flotte[choix].actif = 0;
        system("clear");
    }
}

void jouer() {
    char grilleJ1[DIM][DIM], grilleJ2[DIM][DIM], tirsJ1[DIM][DIM], tirsJ2[DIM][DIM];
    initialiserGrille(grilleJ1);
    initialiserGrille(grilleJ2);
    initialiserGrille(tirsJ1);
    initialiserGrille(tirsJ2);

    Bateau flotteJ1[5] = { {'#',5,5,1},{'@',4,4,1},{'%',3,3,1},{'&',3,3,1},{'$',2,2,1} };
    Bateau flotteJ2[5];
    memcpy(flotteJ2, flotteJ1, sizeof(flotteJ1));

    placement(grilleJ1, 1, flotteJ1);
    placement(grilleJ2, 2, flotteJ2);

    printf("Les deux joueurs ont placé leurs bateaux. La partie commence !\n");
}

int main() {
    srand(time(NULL));
    jouer();
    return 0;
}
