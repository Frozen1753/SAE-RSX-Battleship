#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ip_serveur>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Création du socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configuration du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Connexion au serveur
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connecté au serveur.\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(sock_fd, buffer, BUFFER_SIZE, 0);

        // Vérification des erreurs
        if (bytes <= 0) {
            printf("Connexion perdue. Le serveur s'est peut-être arrêté.\n");
            break;
        }

        // Détection d’un arrêt du serveur
        if (strcmp(buffer, "Le serveur a été arrêté.\n") == 0) {
            printf("Le serveur a été arrêté. Déconnexion en cours...\n");
            break;
        }

        // Détection si l'autre client se déconnecte
        if (strncmp(buffer, "Le client [", 11) == 0 && strstr(buffer, "s'est déconnecté")) {
            printf("%s\nFin de la conversation.\n", buffer);
            break;
        }

        // Identifier le message reçu
        if (strncmp(buffer, "Envoyer :", 8) == 0) {
            printf("> %s", buffer);
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            send(sock_fd, buffer, strlen(buffer) + 1, 0);

            if (strcmp(buffer, "--quitchat") == 0) {
                printf("Déconnecté\nVous pouvez fermer le programme.\n");
                break;
            }
        }
        else {
            printf("%s", buffer);
        }
    }

    close(sock_fd);
    return 0;
}
