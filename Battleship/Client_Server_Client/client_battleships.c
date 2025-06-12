#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "battleships_mult.h"

#define PORT 12345
#define BUFFER_SIZE 1024
#define DIM 10

extern void initialiserGrille(char grille[DIM][DIM]);
extern void sauvegarderGrille(char* buffer, char grille[DIM][DIM]);
extern void afficherGrille(char grille[DIM][DIM], int cacher); // déjà dans battleships_mult.c

int lettreVersIndice(char lettre);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ip_serveur>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connecté au serveur.\n");
    recv(sock_fd, buffer, BUFFER_SIZE, 0);
    printf("%s\n", buffer);

    char grille[DIM][DIM];
    char grille_tirs[DIM][DIM];

    typedef struct {
        char symbole;
        int taille;
        int restants;
        int actif;
    } Bateau;

    Bateau flotte[5] = { {'#',5,5,1},{'@',4,4,1},{'%',3,3,1},{'&',3,3,1},{'$',2,2,1} };

    initialiserGrille(grille);
    initialiserGrille(grille_tirs);

    placement(grille, 1, flotte);

    sauvegarderGrille(buffer, grille);
    send(sock_fd, buffer, DIM * DIM + 1, 0);

    recv(sock_fd, buffer, BUFFER_SIZE, 0);
    printf("%s\n", buffer);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock_fd, buffer, BUFFER_SIZE, 0);
        printf("%s\n", buffer);

        // Affichage des deux grilles à chaque tour
        printf("\n--- VOTRE GRILLE PERSONNELLE ---\n");
        afficherGrille(grille, 0);
        printf("\n--- VOTRE GRILLE DE TIRS ---\n");
        afficherGrille(grille_tirs, 0);

        if (strncmp(buffer, "Votre tour", 10) == 0) {
            char ligne[BUFFER_SIZE];
            while (1) {
                printf("Entrez une case à viser (ex: B4) : ");
                fgets(ligne, sizeof(ligne), stdin);

                if (strlen(ligne) < 2) continue; // entrée trop courte
                ligne[strcspn(ligne, "\n")] = 0;

                char lettre = toupper(ligne[0]);
                char chiffre = ligne[1];
                if (lettre < 'A' || lettre > 'J' || chiffre < '0' || chiffre > '9') {
                    printf("Entrée invalide. Format attendu : A0 - J9\n");
                    continue;
                }

                // Vérifier si déjà tiré sur cette case
                int i = lettreVersIndice(lettre);
                int j = chiffre - '0';
                if (grille_tirs[i][j] != '-') {
                    printf("Vous avez déjà tiré sur cette case.\n");
                    continue;
                }

                send(sock_fd, ligne, strlen(ligne) + 1, 0);
                break;
            }

            recv(sock_fd, buffer, BUFFER_SIZE, 0);
            printf("%s\n", buffer);

            // Mettre à jour la grille de tir selon le résultat
            char lettre = toupper(ligne[0]);
            char chiffre = ligne[1];
            int i = lettreVersIndice(lettre);
            int j = chiffre - '0';
            if (strncmp(buffer, "Touché", 6) == 0)
                grille_tirs[i][j] = 'X';
            else if (strncmp(buffer, "Manqué", 6) == 0)
                grille_tirs[i][j] = 'O';
        }
        else if (strncmp(buffer, "Attente", 7) == 0) {
            // On attend, rien à faire
        }
        else {
            // On suppose que l'adversaire vient de tirer, essayer de deviner la case touchée à partir de la grille personnelle
            // On peut afficher la grille personnelle après chaque tour pour voir l'évolution
        }

        // Affichage des deux grilles après action
        printf("\n--- VOTRE GRILLE PERSONNELLE ---\n");
        afficherGrille(grille, 0);
        printf("\n--- VOTRE GRILLE DE TIRS ---\n");
        afficherGrille(grille_tirs, 0);

        if (strncmp(buffer, "Victoire", 8) == 0) {
            printf("Vous avez gagné !\n");
            break;
        }

        if (strncmp(buffer, "Défaite", 7) == 0) {
            printf("Vous avez perdu.\n");
            break;
        }
    }

    close(sock_fd);
    return 0;
}