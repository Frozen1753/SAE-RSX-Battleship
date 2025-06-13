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
extern void afficherGrille(char grille[DIM][DIM], int cacher);

int lettreVersIndice(char lettre);

void connect_to_server(int* sock_fd, struct sockaddr_in* server_addr, const char* ip) {
    *sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sock_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);
    server_addr->sin_addr.s_addr = inet_addr(ip);

    if (connect(*sock_fd, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
}

void attendre_message(int sock_fd, char* buffer) {
    memset(buffer, 0, BUFFER_SIZE);
    int n = recv(sock_fd, buffer, BUFFER_SIZE, 0);
    if (n <= 0) {
        perror("Erreur de reception");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
}

void placement_bateaux(char grille[DIM][DIM], Bateau flotte[5]) {
    initialiserGrille(grille);
    placement(grille, 1, flotte);
}

void envoyer_grille(int sock_fd, char grille[DIM][DIM]) {
    char buffer[DIM * DIM + 1];
    sauvegarderGrille(buffer, grille);
    send(sock_fd, buffer, DIM * DIM + 1, 0);
}

void afficher_jeu(char grille[DIM][DIM], char grille_tirs[DIM][DIM], const char* status, const char* info) {
    system("clear");
    if (status) printf("%s\n\n", status);
    if (info && info[0]) printf("%s\n\n", info);
    printf("--- VOTRE GRILLE PERSONNELLE ---\n");
    afficherGrille(grille, 0);
    printf("\n--- VOTRE GRILLE DE TIRS ---\n");
    afficherGrille(grille_tirs, 0);
}

void jouer_partie(int sock_fd, char grille[DIM][DIM], char grille_tirs[DIM][DIM]) {
    char buffer[BUFFER_SIZE];
    char status[64] = "";
    char info_message[128] = "";
    char last_enemy_action[128] = ""; // Ce que l'ennemi vient de faire sur nous
    char last_player_result[128] = ""; // Resultat de notre dernier tir

    while (1) {
        attendre_message(sock_fd, buffer);

        // Process all lines in buffer (split on '\n')
        char *saveptr;
        char *line = strtok_r(buffer, "\n", &saveptr);
        while (line) {
            // On efface les statuts
            status[0] = 0;
            info_message[0] = 0;
            // Le resultat de notre tir n'est affiche que quand on attend ("Attente..."), donc on le reset la
            if (strncmp(line, "Votre tour", 10) == 0)
                last_player_result[0] = 0;

            int is_my_turn = 0;

            if (strncmp(line, "Votre tour", 10) == 0) {
                strcpy(status, "Votre tour");
                is_my_turn = 1;
                // Si un coup adverse vient d'arriver, on l'affiche dans info_message
                if (last_enemy_action[0]) {
                    strcpy(info_message, last_enemy_action);
                    last_enemy_action[0] = 0; // on le consomme
                }
            }
            else if (strncmp(line, "Attente...", 10) == 0) {
                strcpy(status, "Attente...");
                // On affiche ici le resultat de notre tir
                if (last_player_result[0]) {
                    strcpy(info_message, last_player_result);
                    last_player_result[0] = 0; // on le consomme
                }
            }
            else if (strncmp(line, "L'adversaire a tire sur", 24) == 0) {
                // Parse le message pour mettre a jour la grille
                char lettre;
                int chiffre;
                if (sscanf(line, "L'adversaire a tire sur %c%d", &lettre, &chiffre) == 2) {
                    int i = lettreVersIndice(lettre);
                    int j = chiffre;
                    if (strncmp(line + 27, "Coule", 5) == 0) {
                        grille[i][j] = 'X';
                        snprintf(last_enemy_action, sizeof(last_enemy_action),
                            "L'ennemi a coule votre bateau en (%c,%d)!", lettre, chiffre);
                    }
                    else if (strncmp(line + 27, "Touche", 6) == 0) {
                        grille[i][j] = 'X';
                        snprintf(last_enemy_action, sizeof(last_enemy_action),
                            "L'ennemi a touche votre bateau en (%c,%d)!", lettre, chiffre);
                    }
                    else if (strncmp(line + 27, "Manque", 6) == 0) {
                        grille[i][j] = 'O';
                        snprintf(last_enemy_action, sizeof(last_enemy_action),
                            "L'ennemi a tire en (%c,%d) et a rate.", lettre, chiffre);
                    }
                }
                // On ne fait pas d'affichage maintenant, on garde en memoire pour le prochain "Votre tour"
                line = strtok_r(NULL, "\n", &saveptr);
                continue; // On continue la boucle pour recevoir "Votre tour" ou autre
            }

            // Affichage general selon l'ordre demande
            system("clear");
            printf("%s\n", status);
            if (info_message[0]) printf("%s\n\n", info_message);

            printf("--- VOTRE GRILLE PERSONNELLE ---\n");
            afficherGrille(grille, 0);
            printf("\n--- VOTRE GRILLE DE TIRS ---\n");
            afficherGrille(grille_tirs, 0);

            if (is_my_turn) {
                char ligne[BUFFER_SIZE];
                while (1) {
                    printf("Entrez une case a viser (ex: B4) : ");
                    fgets(ligne, sizeof(ligne), stdin);

                    if (strlen(ligne) < 2) continue;
                    ligne[strcspn(ligne, "\n")] = 0;

                    char lettre = toupper(ligne[0]);
                    char chiffre = ligne[1];
                    if (lettre < 'A' || lettre > 'J' || chiffre < '0' || chiffre > '9') {
                        printf("Entree invalide. Format attendu : A0 - J9\n");
                        continue;
                    }

                    int i = lettreVersIndice(lettre);
                    int j = chiffre - '0';
                    if (grille_tirs[i][j] != '-') {
                        printf("Vous avez deja tire sur cette case.\n");
                        continue;
                    }

                    send(sock_fd, ligne, strlen(ligne) + 1, 0);
                    break;
                }

                // Resultat du tir
                attendre_message(sock_fd, buffer);

                char lettre = toupper(ligne[0]);
                char chiffre = ligne[1];
                int i = lettreVersIndice(lettre);
                int j = chiffre - '0';
                if (strcmp(buffer, "Coule !") == 0) {
                    grille_tirs[i][j] = 'X';
                    snprintf(last_player_result, sizeof(last_player_result), "Vous avez coule un bateau en (%c,%d) !", lettre, j);
                }
                else if (strcmp(buffer, "Touche !") == 0) {
                    grille_tirs[i][j] = 'X';
                    snprintf(last_player_result, sizeof(last_player_result), "Vous avez touche un bateau en (%c,%d) !", lettre, j);
                }
                else if (strcmp(buffer, "Manque.") == 0) {
                    grille_tirs[i][j] = 'O';
                    snprintf(last_player_result, sizeof(last_player_result), "Vous avez tire en (%c,%d) et avez manque.", lettre, j);
                }
                else {
                    snprintf(last_player_result, sizeof(last_player_result), "Reponse du serveur : %s", buffer);
                }
            }

            // Fin de partie
            if (strncmp(line, "Victoire", 8) == 0) {
                printf("Vous avez gagne !\n");
                exit(0);
            }
            if (strncmp(line, "Defaite", 7) == 0) {
                printf("Vous avez perdu.\n");
                exit(0);
            }

            line = strtok_r(NULL, "\n", &saveptr);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ip_serveur>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock_fd;
    struct sockaddr_in server_addr;
    char grille[DIM][DIM];
    char grille_tirs[DIM][DIM];

    Bateau flotte[5] = { {'#',5,5,1},{'@',4,4,1},{'%',3,3,1},{'&',3,3,1},{'$',2,2,1} };

    connect_to_server(&sock_fd, &server_addr, argv[1]);

    printf("Connecte au serveur.\n");

    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock_fd, buffer, BUFFER_SIZE, 0);
        if (strncmp(buffer, "Demmarage du jeu", 16) == 0) {
            printf("Demmarage du jeu\n");
            break;
        }
        else {
            printf("%s\n", buffer); // "En attente de l'autre joueur..."
        }
    }

    placement_bateaux(grille, flotte);
    initialiserGrille(grille_tirs);

    envoyer_grille(sock_fd, grille);

    afficherGrille(grille, 0);

    attendre_message(sock_fd, buffer);
    printf("%s\n", buffer);

    jouer_partie(sock_fd, grille, grille_tirs);

    close(sock_fd);
    return 0;
}