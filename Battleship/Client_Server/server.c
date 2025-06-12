#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
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

int traiterTir(char grille[DIM][DIM][3], int* vieBateaux, int i, int j, char* reponse) {
    if (!valides(i, j)) {
        strcpy(reponse, "invalide");
        return 0;
    }
    if (strcmp(grille[i][j], "-") != 0 && strcmp(grille[i][j], "X") != 0 && strcmp(grille[i][j], "O") != 0) {
        char symbole = grille[i][j][0];
        grille[i][j][0] = 'X'; grille[i][j][1] = '\0';
        if (vieBateaux[(int)symbole] > 0) vieBateaux[(int)symbole]--;
        if (vieBateaux[(int)symbole] == 0)
            strcpy(reponse, "coule");
        else
            strcpy(reponse, "touche");
        return 1;
    } else if (strcmp(grille[i][j], "-") == 0) {
        strcpy(grille[i][j], "O");
        strcpy(reponse, "eau");
        return 0;
    } else if (strcmp(grille[i][j], "X") == 0 || strcmp(grille[i][j], "O") == 0) {
        strcpy(reponse, "deja");
        return 0;
    } else {
        strcpy(reponse, "eau");
        return 0;
    }
}

void demanderTir(char* coup) {
    printf("À vous de tirer !\n");
    printf("Entrez votre coup (ex: B5) : ");
    scanf("%s", coup);
}

void placementManuel(char grille[DIM][DIM][3], Bateau flotte[]) {
    printf("=== Placement manuel des bateaux (Serveur) ===\n");
    for (int b = 0; b < 5; b++) {
        int ok = 0;
        while (!ok) {
            afficherDeuxGrilles(grille, NULL); // NULL pour pas d'affichage ennemi
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
    srand(time(NULL));
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(12345);
    bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(sockfd, 1);
    printf("En attente d'un client...\n");
    int client_sock = accept(sockfd, NULL, NULL);
    printf("Client connecté !\n");

    // Synchronisation
    char syncbuf[16];
    int syncn = recv(client_sock, syncbuf, sizeof(syncbuf)-1, 0);
    if (syncn <= 0 || strncmp(syncbuf, "pret", 4) != 0) {
        printf("Erreur de synchronisation avec le client.\n");
        close(client_sock);
        close(sockfd);
        return 1;
    }

    // Placement manuel des bateaux du serveur
    char grilleServeur[DIM][DIM][3];
    initialiserGrille(grilleServeur);
    Bateau flotteServeur[5] = { {'#',5,5,1},{'@',4,4,1},{'%',3,3,1},{'&',3,3,1},{'$',2,2,1} };
    int vieServeur[128] = {0};
    vieServeur['#'] = 5; vieServeur['@'] = 4; vieServeur['%'] = 3; vieServeur['&'] = 3; vieServeur['$'] = 2;
    placementManuel(grilleServeur, flotteServeur);

    // Répondre au client qu'on est prêt
    send(client_sock, "pret", 4, 0);

    // Boucle de jeu
    char buffer[32], reponse[16];
    char grilleEnnemie[DIM][DIM][3];
    initialiserGrille(grilleEnnemie);
    while (1) {
        // 1. Le client tire sur le serveur
        int n = recv(client_sock, buffer, sizeof(buffer)-1, 0);
        if (n <= 0) break;
        buffer[n] = 0;
        char lettre;
        int chiffre;
        if (sscanf(buffer, " %c%d", &lettre, &chiffre) != 2) {
            send(client_sock, "invalide", 8, 0);
            continue;
        }
        int i = lettreVersIndice(lettre);
        int j = chiffre;
        traiterTir(grilleServeur, vieServeur, i, j, reponse);
        send(client_sock, reponse, strlen(reponse), 0);

        printf("Le client a tiré sur %c%d : %s\n", lettre, chiffre, reponse);
        afficherDeuxGrilles(grilleServeur, grilleEnnemie);

        // Vérifier victoire du client
        if (vieServeur['#'] == 0 && vieServeur['@'] == 0 && vieServeur['%'] == 0 && vieServeur['&'] == 0 && vieServeur['$'] == 0) {
            send(client_sock, "victoire", 8, 0);
            printf("Le client a gagné !\n");
            break;
        }

        // 2. Le serveur (humain) tire sur le client
        char coup[16];
        demanderTir(coup);
        send(client_sock, coup, strlen(coup), 0);

        // 3. Recevoir la réponse du client
        n = recv(client_sock, reponse, sizeof(reponse)-1, 0);
        if (n <= 0) break;
        reponse[n] = 0;
        printf("Réponse du client à %s : %s\n", coup, reponse);

        // Mettre à jour la grille ennemie
        if (sscanf(coup, " %c%d", &lettre, &chiffre) == 2) {
            int i = lettreVersIndice(lettre);
            int j = chiffre;
            if (strcmp(reponse, "touche") == 0 || strcmp(reponse, "coule") == 0) {
                strcpy(grilleEnnemie[i][j], "X");
            } else if (strcmp(reponse, "eau") == 0) {
                strcpy(grilleEnnemie[i][j], "O");
            }
        }
        afficherDeuxGrilles(grilleServeur, grilleEnnemie);

        // Vérifier victoire du serveur
        if (strcmp(reponse, "victoire") == 0) {
            printf("Le serveur a gagné !\n");
            break;
        }
    }

    close(client_sock);
    close(sockfd);
    return 0;
}