#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
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
    for (int i = 1; i < DIM; i++)
        for (int j = 1; j < DIM; j++)
            strcpy(grille[i][j], "-");
}

void afficherGrille(char grille[DIM][DIM][3]) {
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
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

void placementManuel(char grille[DIM][DIM][3], Bateau flotte[]) {
    printf("Placement de vos bateaux :\n");
    for (int b = 0; b < 5; b++) {
        int ok = 0;
        while (!ok) {
            afficherGrille(grille);
            printf("Bateau %d (taille %d, symbole %c)\n", b+1, flotte[b].taille, flotte[b].symbole);
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
            if (peutPlacer(flotte[b].taille, rot, i, j, grille)) {
                placerBateau(flotte[b].taille, rot, i, j, flotte[b].symbole, grille);
                ok = 1;
            } else {
                printf("Placement invalide, recommencez.\n");
            }
        }
    }
}

int traiterTir(char grille[DIM][DIM][3], int* vieBateaux, int i, int j, char* reponse) {
    if (!valides(i, j)) {
        strcpy(reponse, "invalide");
        return 0;
    }
    if (strcmp(grille[i][j], "-") != 0 && strcmp(grille[i][j], "X") != 0 && strcmp(grille[i][j], "O") != 0) {
        char symbole = grille[i][j][0];
        grille[i][j][0] = 'X'; grille[i][j][1] = '\0';
        vieBateaux[symbole]--;
        if (vieBateaux[symbole] == 0)
            strcpy(reponse, "coule");
        else
            strcpy(reponse, "touche");
        return 1;
    } else if (strcmp(grille[i][j], "-") == 0) {
        strcpy(grille[i][j], "O");
        strcpy(reponse, "eau");
        return 0;
    } else {
        strcpy(reponse, "eau");
        return 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <adresse_ip_serveur>\n", argv[0]);
        return 1;
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(12345);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connexion échouée");
        return 1;
    }
    printf("Connecté au serveur !\n");

    char grilleJoueur[DIM][DIM][3], grilleTirs[DIM][DIM][3];
    initialiserGrille(grilleJoueur);
    initialiserGrille(grilleTirs);
    Bateau flotteJoueur[5] = { {'#',5,5,1},{'@',4,4,1},{'%',3,3,1},{'&',3,3,1},{'$',2,2,1} };
    int vieJoueur[128] = {0};
    vieJoueur['#'] = 5; vieJoueur['@'] = 4; vieJoueur['%'] = 3; vieJoueur['&'] = 3; vieJoueur['$'] = 2;

    placementManuel(grilleJoueur, flotteJoueur);

    send(sockfd, "pret", 4, 0);

    char syncbuf[16];
    int syncn = recv(sockfd, syncbuf, sizeof(syncbuf)-1, 0);
    if (syncn <= 0 || strncmp(syncbuf, "pret", 4) != 0) {
        printf("Erreur de synchronisation avec le serveur.\n");
        close(sockfd);
        return 1;
    }

    char buffer[32], reponse[16];
    char coup[16];
    char lettre;
    int chiffre;
    int n;

    while (1) {
        int tir_valide = 0;
        while (!tir_valide) {
            afficherGrille(grilleTirs);
            printf("A vous de tirer !\n");
            printf("Entrez votre coup (ex: B5) : ");
            scanf("%s", coup);

            char lettreTir;
            int chiffreTir;
            if (sscanf(coup, " %c%d", &lettreTir, &chiffreTir) != 2) {
                printf("Coup invalide, réessayez.\n");
                continue;
            }
            int iTir = lettreVersIndice(lettreTir);
            int jTir = chiffreTir;
            if (!valides(iTir, jTir)) {
                printf("Coup hors grille, réessayez.\n");
                continue;
            }
            if (strcmp(grilleTirs[iTir][jTir], "X") == 0 || strcmp(grilleTirs[iTir][jTir], "O") == 0) {
                printf("Vous avez déjà tiré ici, réessayez.\n");
                continue;
            }

            send(sockfd, coup, strlen(coup), 0);

            n = recv(sockfd, reponse, sizeof(reponse)-1, 0);
            if (n <= 0) {
                printf("Déconnexion du serveur.\n");
                exit(1);
            }
            reponse[n] = 0;
            printf("Réponse du serveur : %s\n", reponse);

            if (strcmp(reponse, "invalide") == 0) {
                printf("Coup refusé par le serveur, recommencez.\n");
                continue;
            }

            tir_valide = 1;
        }

        if (sscanf(coup, " %c%d", &lettre, &chiffre) == 2) {
            int ti = lettreVersIndice(lettre);
            int tj = chiffre;
            if (strncmp(reponse, "touche", 6) == 0 || strncmp(reponse, "coule", 5) == 0)
                strcpy(grilleTirs[ti][tj], "X");
            else if (strncmp(reponse, "eau", 3) == 0)
                strcpy(grilleTirs[ti][tj], "O");
        }

        if (strcmp(reponse, "victoire") == 0) {
            printf("\n=== FIN DE PARTIE ===\n");
            printf("%s\n", reponse + 9); // Affiche le message après "victoire: "
            printf("=====================\n");
            break;
        }

        n = recv(sockfd, buffer, sizeof(buffer)-1, 0);
        if (n <= 0) break;
        buffer[n] = 0;
        if (sscanf(buffer, " %c%d", &lettre, &chiffre) != 2) {
            send(sockfd, "invalide", 8, 0);
            continue;
        }
        int i = lettreVersIndice(lettre);
        int j = chiffre;
        traiterTir(grilleJoueur, vieJoueur, i, j, reponse);
        send(sockfd, reponse, strlen(reponse), 0);

        printf("Le serveur a tiré sur %c%d : %s\n", lettre, chiffre, reponse);
        afficherGrille(grilleJoueur);

        if (vieJoueur['#'] == 0 && vieJoueur['@'] == 0 && vieJoueur['%'] == 0 && vieJoueur['&'] == 0 && vieJoueur['$'] == 0) {
            send(sockfd, "victoire", 8, 0);
            printf("Le serveur a gagné !\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}