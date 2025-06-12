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

void placementIA(char grille[DIM][DIM][3], Bateau flotte[]) {
    for (int b = 0; b < 5; b++) {
        int ok = 0;
        while (!ok) {
            int rot = rand() % 4 + 1;
            int i = rand() % 10 + 1;
            int j = rand() % 10 + 1;
            if (peutPlacer(flotte[b].taille, rot, i, j, grille)) {
                placerBateau(flotte[b].taille, rot, i, j, flotte[b].symbole, grille);
                ok = 1;
            }
        }
    }
}

void afficherGrille(char grille[DIM][DIM][3]) {
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            printf("%s ", grille[i][j]);
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
        vieBateaux[symbole]--;
        if (vieBateaux[symbole] == 0)
            strcpy(reponse, "coule");
        else
            strcpy(reponse, "touche");
        return 1;
    } else if (strcmp(grille[i][j], "-") == 0) {
        strcpy(grille[i][j], "O"); // Ajout : marque l'eau
        strcpy(reponse, "eau");
        return 0;
    } else {
        strcpy(reponse, "eau");
        return 0;
    }
}

int main() {
    srand(time(NULL));
    // --- Initialisation réseau ---
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

    // --- Synchronisation : attendre que le client soit prêt ---
    char syncbuf[16];
    int syncn = recv(client_sock, syncbuf, sizeof(syncbuf)-1, 0);
    if (syncn <= 0 || strncmp(syncbuf, "pret", 4) != 0) {
        printf("Erreur de synchronisation avec le client.\n");
        close(client_sock);
        close(sockfd);
        return 1;
    }

    // --- Initialisation jeu IA (APRÈS réception du "pret") ---
    char grilleIA[DIM][DIM][3];
    initialiserGrille(grilleIA);
    Bateau flotteIA[5] = { {'#',5,5,1},{'@',4,4,1},{'%',3,3,1},{'&',3,3,1},{'$',2,2,1} };
    int vieIA[128] = {0};
    vieIA['#'] = 5; vieIA['@'] = 4; vieIA['%'] = 3; vieIA['&'] = 3; vieIA['$'] = 2;
    placementIA(grilleIA, flotteIA);

    // Répondre au client qu'on est prêt
    send(client_sock, "pret", 4, 0);

    // --- Boucle de jeu ---
    char buffer[32];
    while (1) {
        int n = recv(client_sock, buffer, sizeof(buffer)-1, 0);
        if (n <= 0) break;
        buffer[n] = 0;
        // Format attendu : "B5"
        char lettre;
        int chiffre;
        if (sscanf(buffer, " %c%d", &lettre, &chiffre) != 2) {
            send(client_sock, "invalide", 8, 0);
            continue;
        }
        int i = lettreVersIndice(lettre);
        int j = chiffre;
        char reponse[16];
        traiterTir(grilleIA, vieIA, i, j, reponse);
        send(client_sock, reponse, strlen(reponse), 0);

        // Optionnel : afficher la grille côté serveur
        printf("Tir reçu : %c%d -> %s\n", lettre, chiffre, reponse);
        afficherGrille(grilleIA);

        // Vérifier victoire
        if (vieIA['#'] == 0 && vieIA['@'] == 0 && vieIA['%'] == 0 && vieIA['&'] == 0 && vieIA['$'] == 0) {
            send(client_sock, "victoire", 8, 0);
            printf("Le client a gagné !\n");
            break;
        }
    }

    close(client_sock);
    close(sockfd);
    return 0;
}