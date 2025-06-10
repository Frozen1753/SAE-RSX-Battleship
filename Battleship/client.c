#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Adresse du serveur
    serv_addr.sin_port = htons(12345);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connexion échouée");
        return 1;
    }
    printf("Connecté au serveur !\n");

    while (1) {
        char coup[16];
        printf("Entrez votre coup (ex: B5) : ");
        scanf("%s", coup);

        send(sockfd, coup, strlen(coup), 0);

        char reponse[32];
        int n = recv(sockfd, reponse, sizeof(reponse)-1, 0);
        if (n <= 0) {
            printf("Déconnexion du serveur.\n");
            break;
        }
        reponse[n] = 0;
        printf("Réponse du serveur : %s\n", reponse);

        if (strcmp(reponse, "victoire") == 0) {
            printf("Vous avez gagné !\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}