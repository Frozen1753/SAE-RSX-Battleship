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

int server_fd, client1_fd = -1, client2_fd = -1;

void chargerGrille(const char* buffer, char grille[DIM][DIM]);
int lettreVersIndice(char c);

int estFini(char grille[DIM][DIM]) {
    for (int i = 0; i < DIM; i++)
        for (int j = 0; j < DIM; j++)
            if (strchr("#@%&$", grille[i][j]))
                return 0;
    return 1;
}

void cleanup() {
    char shutdown_msg[] = "Le serveur a ete arrete.\n";
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

void init_server(struct sockaddr_in* server_addr) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket failed"); exit(EXIT_FAILURE); }

    // evite l'erreur "Address already in use" (ladresse peut etre reutilisee)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    listen(server_fd, 2);
    printf("Serveur en ecoute sur le port %d...\n", PORT);
}

void accept_clients(struct sockaddr_in *client_addr1, struct sockaddr_in *client_addr2, socklen_t *addr_len) {
    client1_fd = accept(server_fd, (struct sockaddr*)client_addr1, addr_len);
    printf("Client 1 connecte (%s).\n", inet_ntoa(client_addr1->sin_addr));
    send(client1_fd, "En attente de l'autre joueur...", 32, 0);

    client2_fd = accept(server_fd, (struct sockaddr*)client_addr2, addr_len);
    printf("Client 2 connecte (%s).\n", inet_ntoa(client_addr2->sin_addr));

    // demarrage du jeu pour les deux
    send(client1_fd, "Demmarage du jeu", 17, 0);
    send(client2_fd, "Demmarage du jeu", 17, 0);
	printf("Demmarage du jeu...\n");
}

void send_until_success(int sock_fd, const char *msg, int bufsize, int flags) {
    while (send(sock_fd, msg, bufsize, flags) < 0) {
        perror("send failed");
        usleep(100000);
    }
}

void recevoir_grilles(char grille1[DIM][DIM], char grille2[DIM][DIM], struct sockaddr_in* client_addr1, struct sockaddr_in* client_addr2) {
    char grille1_ok = 0, grille2_ok = 0;
    char buffer1[DIM * DIM + 1] = { 0 };
    char buffer2[DIM * DIM + 1] = { 0 };

    printf("En attentes des grilles...\n");

    while (!(grille1_ok && grille2_ok)) {
        if (!grille1_ok) {
            int n = recv(client1_fd, buffer1, DIM * DIM + 1, MSG_DONTWAIT);
            if (n > 0) {
                chargerGrille(buffer1, grille1);
                send(client1_fd, "Grille recu, merci d'attendre l'autre joueur...\n", 49, 0);
                grille1_ok = 1;
                printf("Grille du client 1 recu (%s)\n", inet_ntoa(client_addr1->sin_addr));
            }
        }
        if (!grille2_ok) {
            int n = recv(client2_fd, buffer2, DIM * DIM + 1, MSG_DONTWAIT);
            if (n > 0) {
                chargerGrille(buffer2, grille2);
                send(client2_fd, "Grille recu, merci d'attendre l'autre joueur...\n", 49, 0);
                grille2_ok = 1;
                printf("Grille du client 2 recu (%s)\n", inet_ntoa(client_addr2->sin_addr));
            }
        }
        usleep(10000);
    }
    printf("Grilles des joueurs chargees avec succes.\n");
}

int bateau_coule(char grille[DIM][DIM], char symbole) {
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            if (grille[i][j] == symbole)
                return 0;
    return 1;
}

char *ChoisirGrille(int grille_num) {
    switch (grille_num) {
        case 1: return "Grille 1";
        case 2: return "Grille 2";
        default: return "Grille inconnue";
    }
}

int jouer_partie(char grille1[DIM][DIM], char grille2[DIM][DIM], struct sockaddr_in* client_addr1, struct sockaddr_in* client_addr2) {
    char buffer[BUFFER_SIZE];
    printf("Debut de la partie...\n");

    int current = 0; // joueur actuelle (0 ou 1)
    int clients[2] = { client1_fd, client2_fd }; //sockets
    char (*grilles[2])[DIM] = { grille1, grille2 };
    struct sockaddr_in* client_addrs[2] = { client_addr1, client_addr2 };

    char notif[128] = ""; // message d'info action adverse

    int tour = 0;

    while (1) {
        int joueur_actif = current;
        int joueur_suivant = 1 - current; // adversaire

        // instruction de tour
        if (tour > 0) {
			printf("Notification de l'action adverse : %s\n", notif);
            send_until_success(clients[joueur_actif], notif, strlen(notif), 0);
        }
        printf("Tour %d : Joueur %d (%s) joue contre Joueur %d (%s)\n", tour + 1, joueur_actif + 1, inet_ntoa(client_addrs[joueur_actif]->sin_addr), joueur_suivant + 1, inet_ntoa(client_addrs[joueur_suivant]->sin_addr));
        send_until_success(clients[joueur_actif], "Votre tour\n", 11, 0);
        send_until_success(clients[joueur_suivant], "Attente...\n", 11, 0);

        // reception des coords
        memset(buffer, 0, BUFFER_SIZE);
        int recvd = recv(clients[joueur_actif], buffer, BUFFER_SIZE, 0);
        if (recvd <= 0) {
            printf("Deconnexion ou erreur du client %d (%s)\n", joueur_actif + 1, inet_ntoa(client_addrs[joueur_actif]->sin_addr));
            cleanup();
            return 1;
        }
        printf("Coordonnees recues du client %d (%s) : '%s'\n", joueur_actif + 1, inet_ntoa(client_addrs[joueur_actif]->sin_addr), buffer);

        // debugage
        printf("DEBUG: buffer='%s', len=%zu, bytes=[%02x %02x %02x]\n", buffer, strlen(buffer), buffer[0], buffer[1], buffer[2]);
        if (strlen(buffer) < 2) {
            printf("Erreur: coordonnees trop courtes ! (buffer='%s')\n", buffer);
            return 1;
        }
        char lettre = toupper(buffer[0]);
        char chiffre = buffer[1];
        if (lettre < 'A' || lettre > 'J') {
            printf("Erreur: lettre hors bornes: %c\n", lettre);
            return 1;
        }
        if (chiffre < '0' || chiffre > '9') {
            printf("Erreur: chiffre hors bornes: %c\n", chiffre);
            return 1;
        }
        int i = lettreVersIndice(lettre);
        int j = chiffre - '0';
        if (i < 0 || i >= DIM || j < 0 || j >= DIM) {
            printf("Erreur: coordonnees hors grille: (%d,%d)\n", i, j);
            return 1;
        }
        printf("DEBUG: i=%d, j=%d, grille[%d][%d]=%c\n", i, j, i, j, grilles[joueur_suivant][i][j]);
        char symb = grilles[joueur_suivant][i][j];
        if (symb == '\0') {
            printf("ERREUR: symbole vide a (%d,%d)\n", i, j);
            return 1;
        }

        char* msg;

        // MAJ grille adverse
        char symbole = grilles[joueur_suivant][i][j];
        if (symbole != '-') {
            grilles[joueur_suivant][i][j] = 'X';
            if (strchr("#@%&$", symbole) && bateau_coule(grilles[joueur_suivant], symbole)) {
                msg = "Coule !";
                snprintf(notif, sizeof(notif), "L'adversaire a tire sur %c%d : Coule !\n", lettre, j);
            }
            else {
                msg = "Touche !";
                snprintf(notif, sizeof(notif), "L'adversaire a tire sur %c%d : Touche !\n", lettre, j);
            }
        }
        else {
            grilles[joueur_suivant][i][j] = 'O';
            msg = "Manque.";
            snprintf(notif, sizeof(notif), "L'adversaire a tire sur %c%d : Manque.\n", lettre, j);
        }

        send_until_success(clients[joueur_suivant], notif, strlen(notif), 0);

        send_until_success(clients[joueur_actif], msg, strlen(msg), 0);

        if (estFini(grilles[joueur_suivant])) {
            send_until_success(clients[joueur_actif], "Victoire !", 11, 0);
            send_until_success(clients[joueur_suivant], "Defaite.", 9, 0);
            printf("La partie est terminee.\n");
            break;
        }

        current = joueur_suivant;
        tour++;
    }
    return 0;
}

int main() {
    struct sockaddr_in server_addr, client_addr1, client_addr2;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    char grille1[DIM][DIM], grille2[DIM][DIM];

    signal(SIGINT, signal_handler);

    init_server(&server_addr);
    accept_clients(&client_addr1, &client_addr2, &addr_len);
    recevoir_grilles(grille1, grille2, &client_addr1, &client_addr2);
    jouer_partie(grille1, grille2, &client_addr1, &client_addr2);

    cleanup();
    return 0;
}