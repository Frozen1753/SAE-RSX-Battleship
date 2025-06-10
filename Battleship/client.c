#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define DIM 11

void afficherGrille(char grille[DIM][DIM]) {
    printf("  ");
    for (int j = 1; j < DIM; j++) printf("%d ", j);
    printf("\n");
    for (int i = 1; i < DIM; i++) {
        printf("%c ", 'A' + i - 1);
        for (int j = 1; j < DIM; j++) {
            printf("%c ", grille[i][j]);
        }
        printf("\n");
    }
}

int lettreVersIndice(char lettre) {
    lettre = toupper(lettre);
    return (lettre >= 'A' && lettre <= 'J') ? (lettre - 'A' + 1) : 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <adresse_ip_serveur>\n", argv[0]);
        return 1;
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); // Adresse du serveur passée en argument
    serv_addr.sin_port = htons(12345);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connexion échouée");
        return 1;
    }
    printf("Connecté au serveur !\n");

    // Initialisation de la grille de tirs
    char grille[DIM][DIM];
    for (int i = 0; i < DIM; i++)
        for (int j = 0; j < DIM; j++)
            grille[i][j] = (i == 0 || j == 0) ? ' ' : '.';

    while (1) {
        afficherGrille(grille);

        char coup[16];
        printf("Entrez votre coup (ex: B5) : ");
        scanf("%s", coup);

        // Conversion du coup en indices
        char lettre;
        int chiffre;
        if (sscanf(coup, " %c%d", &lettre, &chiffre) != 2) {
            printf("Format invalide. Essayez encore.\n");
            continue;
        }
        int i = lettreVersIndice(lettre);
        int j = chiffre;
        if (i < 1 || i > 10 || j < 1 || j > 10) {
            printf("Coordonnées hors grille.\n");
            continue;
        }

        send(sockfd, coup, strlen(coup), 0);

        char reponse[32];
        int n = recv(sockfd, reponse, sizeof(reponse)-1, 0);
        if (n <= 0) {
            printf("Déconnexion du serveur.\n");
            break;
        }
        reponse[n] = 0;
        printf("Réponse du serveur : %s\n", reponse);

        // Mise à jour de la grille
        if (strncmp(reponse, "touche", 6) == 0 || strncmp(reponse, "coule", 5) == 0) {
            grille[i][j] = 'X';
        } else if (strncmp(reponse, "eau", 3) == 0) {
            grille[i][j] = 'O';
        }

        if (strcmp(reponse, "victoire") == 0) {
            printf("Vous avez gagné !\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}