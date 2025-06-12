#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>

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

void afficherDeuxGrilles(char grillePerso[DIM][DIM][3], char grilleEnnemie[DIM][DIM][3]) {
    printf("Votre grille :%*sGrille ennemie (vos tirs) :\n", 20, "");
    for (int i = 0; i < DIM; i++) {
        // Grille perso
        for (int j = 0; j < DIM; j++) {
            printf("%s ", grillePerso[i][j]);
        }
        printf("%*s", 6, ""); // Espace entre les deux grilles
        // Grille ennemie
        for (int j = 0; j < DIM; j++) {
            if (i == 0 || j == 0) {
                printf("%s ", grilleEnnemie[i][j]);
            } else if (strcmp(grilleEnnemie[i][j], "X") == 0 || strcmp(grilleEnnemie[i][j], "O") == 0) {
                printf("%s ", grilleEnnemie[i][j]);
            } else {
                printf("· ");
            }
        }
        printf("\n");
    }
}

void placementManuel(char grille[DIM][DIM][3], Bateau flotte[]) {
    printf("=== Placement manuel des bateaux (Client) ===\n");
    for (int b = 0; b < 5; b++) {
        int ok = 0;
        while (!ok) {
            afficherDeuxGrilles(grille, NULL);
            printf("Placer le bateau %c (taille %d)\n", flotte[b].symbole, flotte[b].taille);
            char ligne[8];
            char lettre;
            int chiffre, rot;
            printf("Entrer la case de départ (ex: B5) : ");
            scanf("%s", ligne);
            if (sscanf(ligne, " %c%d", &lettre, &chiffre) != 2) {
                printf("Entrée invalide.\n");
                continue;
            }
            printf("Orientation (1=haut, 2=droite, 3=bas, 4=gauche) : ");
            scanf("%d", &rot);
            int i = lettreVersIndice(lettre);
            int j = chiffre;
            if (peutPlacer(flotte[b].taille, rot, i, j, grille)) {
                placerBateau(flotte[b].taille, rot, i, j, flotte[b].symbole, grille);
                ok = 1;
            } else {
                printf("Placement impossible, réessayez.\n");
            }
        }
    }
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Erreur de connexion au serveur\n");
        return 1;
    }

    // Placement manuel des bateaux du client
    char grilleJoueur[DIM][DIM][3];
    initialiserGrille(grilleJoueur);
    Bateau flotteJoueur[5] = { {'#',5,5,1},{'@',4,4,1},{'%',3,3,1},{'&',3,3,1},{'$',2,2,1} };
    int vieJoueur[128] = {0};
    vieJoueur['#'] = 5; vieJoueur['@'] = 4; vieJoueur['%'] = 3; vieJoueur['&'] = 3; vieJoueur['$'] = 2;
    placementManuel(grilleJoueur, flotteJoueur);

    // Synchronisation
    send(sockfd, "pret", 4, 0);
    char syncbuf[16];
    int syncn = recv(sockfd, syncbuf, sizeof(syncbuf)-1, 0);
    if (syncn <= 0 || strncmp(syncbuf, "pret", 4) != 0) {
        printf("Erreur de synchronisation avec le serveur.\n");
        close(sockfd);
        return 1;
    }

    // Boucle de jeu
    char buffer[32], reponse[16];
    char grilleEnnemie[DIM][DIM][3];
    initialiserGrille(grilleEnnemie);
    while (1) {
        // 1. Le client (humain) tire sur le serveur
        afficherDeuxGrilles(grilleJoueur, grilleEnnemie);
        printf("À vous de tirer !\nEntrez votre coup (ex: B5) : ");
        scanf("%s", buffer);
        send(sockfd, buffer, strlen(buffer), 0);

        // 2. Recevoir la réponse du serveur
        int n = recv(sockfd, reponse, sizeof(reponse)-1, 0);
        if (n <= 0) break;
        reponse[n] = 0;
        printf("Réponse du serveur à %s : %s\n", buffer, reponse);

        // Mettre à jour la grille ennemie
        char lettre;
        int chiffre;
        if (sscanf(buffer, " %c%d", &lettre, &chiffre) == 2) {
            int i = lettreVersIndice(lettre);
            int j = chiffre;
            if (strcmp(reponse, "touche") == 0 || strcmp(reponse, "coule") == 0) {
                strcpy(grilleEnnemie[i][j], "X");
            } else if (strcmp(reponse, "eau") == 0) {
                strcpy(grilleEnnemie[i][j], "O");
            }
        }
        afficherDeuxGrilles(grilleJoueur, grilleEnnemie);

        // Vérifier victoire du client
        if (strcmp(reponse, "victoire") == 0) {
            printf("Vous avez gagné !\n");
            break;
        }

        // 3. Le serveur tire sur le client
        n = recv(sockfd, buffer, sizeof(buffer)-1, 0);
        if (n <= 0) break;
        buffer[n] = 0;
        printf("Le serveur tire sur %s\n", buffer);

        // 4. Traiter le tir du serveur
        if (sscanf(buffer, " %c%d", &lettre, &chiffre) != 2) {
            send(sockfd, "invalide", 8, 0);
            continue;
        }
        int i = lettreVersIndice(lettre);
        int j = chiffre;
        char rep[16];
        if (!valides(i, j)) {
            strcpy(rep, "invalide");
        } else if (strcmp(grilleJoueur[i][j], "-") != 0 && strcmp(grilleJoueur[i][j], "X") != 0 && strcmp(grilleJoueur[i][j], "O") != 0) {
            char symbole = grilleJoueur[i][j][0];
            grilleJoueur[i][j][0] = 'X'; grilleJoueur[i][j][1] = '\0';
            if (vieJoueur[(int)symbole] > 0) vieJoueur[(int)symbole]--;
            if (vieJoueur[(int)symbole] == 0)
                strcpy(rep, "coule");
            else
                strcpy(rep, "touche");
        } else if (strcmp(grilleJoueur[i][j], "-") == 0) {
            strcpy(grilleJoueur[i][j], "O");
            strcpy(rep, "eau");
        } else if (strcmp(grilleJoueur[i][j], "X") == 0 || strcmp(grilleJoueur[i][j], "O") == 0) {
            strcpy(rep, "deja");
        } else {
            strcpy(rep, "eau");
        }
        send(sockfd, rep, strlen(rep), 0);

        // Vérifier victoire du serveur
        if (vieJoueur['#'] == 0 && vieJoueur['@'] == 0 && vieJoueur['%'] == 0 && vieJoueur['&'] == 0 && vieJoueur['$'] == 0) {
            send(sockfd, "victoire", 8, 0);
            printf("Le serveur a gagné !\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}