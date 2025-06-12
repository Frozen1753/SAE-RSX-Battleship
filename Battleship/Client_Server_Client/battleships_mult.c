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
    for (int i = 0; i < DIM; i++)
        for (int j = 0; j < DIM; j++)
            grille[i][j] = '-';
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

        int choix = -1;
        char line[32];
        printf("Votre choix : ");
        if (!fgets(line, sizeof(line), stdin) || sscanf(line, "%d", &choix) != 1) {
            printf("Entrée invalide. Merci de saisir un chiffre.\n");
            continue;
        }
        choix--;
        if (choix < 0 || choix >= 5 || !flotte[choix].actif) {
            printf("Choix de bateau invalide.\n");
            continue;
        }

        char lettre;
        int i = -1, j = -1, rot = -1;

        printf("Lettre (A-J) : ");
        if (!fgets(line, sizeof(line), stdin) || sscanf(line, " %c", &lettre) != 1) {
            printf("Entrée invalide pour la lettre.\n");
            continue;
        }
        lettre = toupper(lettre);
        if (lettre < 'A' || lettre > 'J') {
            printf("Lettre hors bornes (A-J).\n");
            continue;
        }
        i = lettreVersIndice(lettre);

        printf("Chiffre (0-9) : ");
        if (!fgets(line, sizeof(line), stdin) || sscanf(line, "%d", &j) != 1) {
            printf("Entrée invalide pour le chiffre.\n");
            continue;
        }
        if (j < 0 || j >= DIM) {
            printf("Chiffre hors bornes (0-9).\n");
            continue;
        }

        printf("Rotation (1=nord, 2=est, 3=sud, 4=ouest) : ");
        if (!fgets(line, sizeof(line), stdin) || sscanf(line, "%d", &rot) != 1) {
            printf("Entrée invalide pour la rotation.\n");
            continue;
        }
        if (rot < 1 || rot > 4) {
            printf("Rotation hors bornes (1-4).\n");
            continue;
        }

        if (!peutPlacer(flotte[choix].taille, rot, i, j, grille)) {
            printf("Placement invalide (emplacement ou superposition).\n");
            continue;
        }

        placerBateau(flotte[choix].taille, rot, i, j, flotte[choix].symbole, grille);
        flotte[choix].actif = 0;
        system("clear");
    }
}

void sauvegarderGrille(char* buffer, char grille[DIM][DIM]) {
    int pos = 0;
    for (int i = 0; i < DIM; i++)
        for (int j = 0; j < DIM; j++)
            buffer[pos++] = grille[i][j];
    buffer[pos] = '\0';
}

void chargerGrille(const char* buffer, char grille[DIM][DIM]) {
    int pos = 0;
    for (int i = 0; i < DIM; i++)
        for (int j = 0; j < DIM; j++)
            grille[i][j] = buffer[pos++];
}
