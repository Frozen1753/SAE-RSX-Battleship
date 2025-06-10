#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define DIM 11
const char* ALPHA[] = { "A","B","C","D","E","F","G","H","I","J" };

typedef struct {
    char symbole;
    int taille;
    int restants;
    int actif;
} Bateau;

void initialiserGrille(char grille[DIM][DIM][3]) {
    strcpy(grille[0][0], " ");
    for (int i = 1; i < DIM; i++) {
        snprintf(grille[0][i], 3, "%d", i);
        strcpy(grille[i][0], ALPHA[i - 1]);
    }
    for (int i = 1; i < DIM; i++) {
        for (int j = 1; j < DIM; j++) {
            strcpy(grille[i][j], "-");
        }
    }
}

void afficherGrille(char grille[DIM][DIM][3], int cacher) {
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            if (cacher && (strcmp(grille[i][j], "#") == 0 || strcmp(grille[i][j], "@") == 0 ||
                strcmp(grille[i][j], "%") == 0 || strcmp(grille[i][j], "&") == 0 ||
                strcmp(grille[i][j], "$") == 0))
                printf("- ");
            else
                printf("%s ", grille[i][j]);
        }
        printf("\n");
    }
}

int lettreVersIndice(char lettre) {
    lettre = toupper(lettre);
    return (lettre >= 'A' && lettre <= 'J') ? (lettre - 'A' + 1) : 0;
}

int valides(int i, int j) {
    return i >= 1 && i < DIM && j >= 1 && j < DIM;
}

int peutPlacer(int taille, int rot, int i, int j, char grille[DIM][DIM][3]) {
    for (int k = 0; k < taille; ++k) {
        int x = i + (rot == 3 ? k : rot == 1 ? -k : 0);
        int y = j + (rot == 2 ? k : rot == 4 ? -k : 0);
        if (!valides(x, y) || strcmp(grille[x][y], "-") != 0) return 0;
    }
    return 1;
}

void placerBateau(int taille, int rot, int i, int j, char symbole, char grille[DIM][DIM][3]) {
    for (int k = 0; k < taille; ++k) {
        int x = i + (rot == 3 ? k : rot == 1 ? -k : 0);
        int y = j + (rot == 2 ? k : rot == 4 ? -k : 0);
        grille[x][y][0] = symbole;
        grille[x][y][1] = '\0';
    }
}

void placement(char grille[DIM][DIM][3], int joueur, Bateau flotte[]) {
    printf("Placement des bateaux du Joueur %d\n", joueur);
    while (1) {
        int fini = 1;
        for (int k = 0; k < 5; k++) {
            if (flotte[k].actif) fini = 0;
        }
        if (fini) break;

        afficherGrille(grille, 0);
        printf("Choisissez un bateau (1-5) :\n");
        for (int i = 0; i < 5; i++) {
            if (flotte[i].actif)
                printf("%d. Bateau taille %d symbole '%c'\n", i + 1, flotte[i].taille, flotte[i].symbole);
        }

        int choix;
        scanf("%d", &choix);
        choix--;
        if (choix < 0 || choix >= 5 || !flotte[choix].actif) continue;

        printf("Lettre (A-J) : ");
        char lettre;
        scanf(" %c", &lettre);
        int i = lettreVersIndice(lettre);

        printf("Chiffre (1-10) : ");
        int j;
        scanf("%d", &j);

        printf("Rotation (1=nord, 2=est, 3=sud, 4=ouest) : ");
        int rot;
        scanf("%d", &rot);

        if (!peutPlacer(flotte[choix].taille, rot, i, j, grille)) {
            printf("Placement invalide.\n");
            continue;
        }

        placerBateau(flotte[choix].taille, rot, i, j, flotte[choix].symbole, grille);
        flotte[choix].actif = 0;
        system("cls || clear");
    }
}

int tirer(char grilleAdverse[DIM][DIM][3], char grilleTirs[DIM][DIM][3], int* vieBateaux) {
    printf("Lettre (A-J) : ");
    char lettre;
    scanf(" %c", &lettre);
    int i = lettreVersIndice(lettre);

    printf("Chiffre (1-10) : ");
    int j;
    scanf("%d", &j);

    if (!valides(i, j)) {
        printf("Coordonnées invalides.\n");
        return 0;
    }

    if (strcmp(grilleTirs[i][j], "-") != 0) {
        printf("Vous avez déjà tiré ici.\n");
        return 0;
    }

    if (strcmp(grilleAdverse[i][j], "-") != 0 && strcmp(grilleAdverse[i][j], "X") != 0) {
        printf("Touché !\n");
        grilleTirs[i][j][0] = 'X'; grilleTirs[i][j][1] = '\0';
        char symbole = grilleAdverse[i][j][0];
        grilleAdverse[i][j][0] = 'X'; grilleAdverse[i][j][1] = '\0';
        vieBateaux[symbole]--;
        if (vieBateaux[symbole] == 0) printf("Bateau coulé !\n");
        return 1;
    }
    else {
        printf("À l'eau.\n");
        grilleTirs[i][j][0] = 'O'; grilleTirs[i][j][1] = '\0';
        return 0;
    }
}

void jouer() {
    char grilleJ1[DIM][DIM][3], grilleJ2[DIM][DIM][3];
    char tirsJ1[DIM][DIM][3], tirsJ2[DIM][DIM][3];
    initialiserGrille(grilleJ1);
    initialiserGrille(grilleJ2);
    initialiserGrille(tirsJ1);
    initialiserGrille(tirsJ2);

    Bateau flotteJ1[5] = { {'#',5,5,1},{'@',4,4,1},{'%',3,3,1},{'&',3,3,1},{'$',2,2,1} };
    Bateau flotteJ2[5];
    memcpy(flotteJ2, flotteJ1, sizeof(flotteJ1));

    int vieJ1[128] = { 0 }, vieJ2[128] = { 0 };
    vieJ1['#'] = 5; vieJ1['@'] = 4; vieJ1['%'] = 3; vieJ1['&'] = 3; vieJ1['$'] = 2;
    memcpy(vieJ2, vieJ1, sizeof(vieJ1));

    placement(grilleJ1, 1, flotteJ1);
    placement(grilleJ2, 2, flotteJ2);

    int tour = 1;
    while (1) {
        system("cls || clear");
        printf("Tour du Joueur %d\n", tour);
        if (tour == 1) {
            printf("Votre grille de tirs :\n");
            afficherGrille(tirsJ1, 0);
            tirer(grilleJ2, tirsJ1, vieJ2);
            if (vieJ2['#'] == 0 && vieJ2['@'] == 0 && vieJ2['%'] == 0 && vieJ2['&'] == 0 && vieJ2['$'] == 0) {
                printf("Joueur 1 a gagné !\n");
                break;
            }
        }
        else {
            printf("Votre grille de tirs :\n");
            afficherGrille(tirsJ2, 0);
            tirer(grilleJ1, tirsJ2, vieJ1);
            if (vieJ1['#'] == 0 && vieJ1['@'] == 0 && vieJ1['%'] == 0 && vieJ1['&'] == 0 && vieJ1['$'] == 0) {
                printf("Joueur 2 a gagné !\n");
                break;
            }
        }
        tour = 3 - tour;
        printf("Appuyez sur Entrée pour continuer...\n");
        getchar(); getchar();
    }
}

int main() {
    srand(time(NULL));
    jouer();
    return 0;
}
