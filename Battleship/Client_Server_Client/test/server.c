#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int server_fd, client1_fd = -1, client2_fd = -1;

void cleanup() {
    char shutdown_msg[] = "Le serveur a été arrêté.\n";
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
    char client_ips[2][INET_ADDRSTRLEN];
    int current_turn = 0;

    signal(SIGINT, signal_handler); // Capture `^C`

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket failed"); exit(EXIT_FAILURE); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    listen(server_fd, 2);
    printf("Serveur en écoute sur le port %d...\n", PORT);

    // Accepter les clients
    client1_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ips[0], INET_ADDRSTRLEN);
    printf("Client 1 connecté [%s].\n", client_ips[0]);
    send(client1_fd, "En attente de l'autre client...\n", 32, 0);

    client2_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ips[1], INET_ADDRSTRLEN);
    printf("Client 2 connecté [%s].\n", client_ips[1]);

    // Informer les clients
    snprintf(buffer, BUFFER_SIZE, "Client %s connecté.\n", client_ips[1]);
    send(client1_fd, buffer, strlen(buffer) + 1, 0);
    snprintf(buffer, BUFFER_SIZE, "Client %s connecté.\n", client_ips[0]);
    send(client2_fd, buffer, strlen(buffer) + 1, 0);

    printf("Les deux clients sont connectés.\n");

    int clients[2] = { client1_fd, client2_fd };

    while (1) {
        int sender = clients[current_turn];
        int receiver = clients[1 - current_turn];

        printf("C'est au tour de [%s] de parler\n", client_ips[current_turn]);
        send(sender, "Envoyer : ", 10, 0);
        snprintf(buffer, BUFFER_SIZE, "En attente d'un message de %s...\n", client_ips[current_turn]);
        send(receiver, buffer, strlen(buffer) + 1, 0);

        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(sender, buffer, BUFFER_SIZE, 0);

        if (bytes <= 0) { // Détection de déconnexion d'un client
            printf("Le client [%s] s'est déconnecté.\n", client_ips[current_turn]);
            snprintf(buffer, BUFFER_SIZE, "Le client [%s] s'est déconnecté.\n", client_ips[current_turn]);
            send(receiver, buffer, strlen(buffer) + 1, 0);
            break;
        }

        if (strcmp(buffer, "--quitchat") == 0) {
            send(receiver, buffer, strlen(buffer) + 1, 0);
            break;
        }

        printf("Transfert de [%s] à [%s] du message : %s\n", client_ips[current_turn], client_ips[1 - current_turn], buffer);

        char formatted_msg[BUFFER_SIZE]; // Nouveau buffer temporaire
        snprintf(formatted_msg, BUFFER_SIZE, "[%s] dit : \"%s\"\n", client_ips[current_turn], buffer);
        send(receiver, formatted_msg, strlen(formatted_msg) + 1, 0);
        send(receiver, buffer, strlen(buffer) + 1, 0);

        current_turn = 1 - current_turn;
    }

    cleanup();
    return 0;
}
