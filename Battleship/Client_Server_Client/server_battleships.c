#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "battleships_mult.h"

#define PORT 12345
#define BUFFER_SIZE 1024
#define DIM 10

int estFini(char grille[DIM][DIM]) {
    for (int i = 0; i < DIM; i++)
        for (int j = 0; j < DIM; j++)
            if (strchr("#@%&$", grille[i][j]))
                return 0;
    return 1;
}

int server_fd, client1_fd = -1, client2_fd = -1;

void cleanup() {
    char shutdown_msg[] = "Le serveur a �t� arr�t�.\n";
    if (client1_fd != -1) send(client1_fd, shutdown_msg, strlen(shutdown_msg) + 1, 0);
    if (client2_fd != -1) send(client2_fd, shutdown_msg, strlen(shutdown_msg) + 1, 0);
    printf("Le serveur ferme.\n");
    close(client1_fd);
    close(client2_fd);
    close(server_fd);
    exit(0);
}

void signal_handler(int sig) {
    cleanup();
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char grille1[DIM][DIM], grille2[DIM][DIM];

    signal(SIGINT, signal_handler);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket failed"); exit(EXIT_FAILURE); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    listen(server_fd, 2);
    printf("Serveur en �coute sur le port %d...\n", PORT);

    client1_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    printf("Client 1 connect�.\n");
    send(client1_fd, "En attente de l'autre joueur...\n", 34, 0);

    client2_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    printf("Client 2 connect�.\n");

    send(client1_fd, "Les deux joueurs sont connect�s.\n", 35, 0);
    send(client2_fd, "Les deux joueurs sont connect�s.\n", 35, 0);

    // R�ception des grilles
    char grille1_ok = 0, grille2_ok = 0;
    char buffer1[DIM * DIM + 1] = { 0 };
    char buffer2[DIM * DIM + 1] = { 0 };

    while (!(grille1_ok && grille2_ok)) {
        if (!grille1_ok) {
            int n = recv(client1_fd, buffer1, DIM * DIM + 1, MSG_DONTWAIT);
            if (n > 0) {
                chargerGrille(buffer1, grille1);
                send(client1_fd, "Grille re�ue, merci d'attendre l'autre joueur...\n", 49, 0);
                grille1_ok = 1;
                printf("Grille du client 1 re�ue\n");
            }
        }
        if (!grille2_ok) {
            int n = recv(client2_fd, buffer2, DIM * DIM + 1, MSG_DONTWAIT);
            if (n > 0) {
                chargerGrille(buffer2, grille2);
                send(client2_fd, "Grille re�ue, merci d'attendre l'autre joueur...\n", 49, 0);
                grille2_ok = 1;
                printf("Grille du client 2 re�ue\n");
            }
        }
        usleep(10000); // Petite pause pour �viter de surcharger le CPU
    }

    send(client1_fd, "La partie commence ! Vous jouez en premier.\n", 45, 0);
    send(client2_fd, "La partie commence ! Attendez votre tour.\n", 43, 0);

    int current = 0;
    int clients[2] = { client1_fd, client2_fd };
    char (*grilles[2])[DIM][DIM] = { &grille1, &grille2 };

    // Ajoutez ces prints dans la boucle principale�:
    while (1) {
        int a = current;
        int d = 1 - current;

        printf("Tour du joueur %d (fd=%d)\n", a + 1, clients[a]);
        send(clients[a], "Votre tour\n", 11, 0);
        send(clients[d], "Attente...\n", 11, 0);

        memset(buffer, 0, BUFFER_SIZE);
        int recvd = recv(clients[a], buffer, BUFFER_SIZE, 0);
        if (recvd <= 0) {
            printf("D�connexion ou erreur du client %d\n", a + 1);
            break;
        }
        printf("Coordonn�es re�ues du client %d : %s\n", a + 1, buffer);

        int i = lettreVersIndice(buffer[0]);
        int j = buffer[1] - '0';

        char* msg;
        if ((*grilles[d])[i][j] != '-') {
            (*grilles[d])[i][j] = 'X';
            msg = "Touch� !\n";
        }
        else {
            (*grilles[d])[i][j] = 'O';
            msg = "Manqu�.\n";
        }
        send(clients[a], msg, strlen(msg) + 1, 0);

        if (estFini(*grilles[d])) {
            send(clients[a], "Victoire !\n", 11, 0);
            send(clients[d], "D�faite.\n", 9, 0);
            printf("La partie est termin�e.\n");
            break;
        }

        current = 1 - current;
    }

    cleanup();
    return 0;
}
