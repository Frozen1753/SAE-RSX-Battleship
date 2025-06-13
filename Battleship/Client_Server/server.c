#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

// Définition de la taille de la grille (10x10 + entêtes)
#define DIM 11
const char* ALPHA[] = { "A","B","C","D","E","F","G","H","I","J" };

// Structure représentant un bateau
typedef struct {
    char symbole;    // Symbole du bateau sur la grille
    int taille;      // Taille du bateau
    int restants;    // Cases restantes (non utilisé ici)
    int actif;       // Statut actif (non utilisé ici)
} Bateau;

// Initialise la grille avec entêtes et cases vides
void initialiserGrille(char grille[DIM][DIM][3]) {
    strcpy(grille[0][0], " "); // Coin supérieur gauche vide
    for (int i = 1; i < DIM; i++) {
        snprintf(grille[0][i], 3, "%d", i);         // Chiffres en haut
        strcpy(grille[i][0], ALPHA[i - 1]);         // Lettres à gauche
    }
    for (int i = 1; i < DIM; i++)
        for (int j = 1; j < DIM; j++)
            strcpy(grille[i][j], "-");              // Cases vides
}

// Convertit une lettre (A-J) en indice de ligne
int lettreVersIndice(char lettre) {
    lettre = toupper(lettre);
    return (lettre >= 'A' && lettre <= 'J') ? (lettre - 'A' + 1) : 0;
}

// Vérifie si une position est valide sur la grille
int valides(int i, int j) {
    return i >= 1 && i < DIM && j >= 1 && j < DIM;
}

// Vérifie si un bateau peut être placé à la position donnée avec la rotation
int peutPlacer(int taille, int rot, int i, int j, char grille[DIM][DIM][3]) {
    for (int k = 0; k < taille; ++k) {
        int x = i + (rot == 3 ? k : rot == 1 ? -k : 0); // Sud/Nord
        int y = j + (rot == 2 ? k : rot == 4 ? -k : 0); // Est/Ouest
        if (!valides(x, y) || strcmp(grille[x][y], "-") != 0) return 0; // Case occupée ou hors grille
    }
    return 1;
}

// Place un bateau sur la grille
void placerBateau(int taille, int rot, int i, int j, char symbole, char grille[DIM][DIM][3]) {
    for (int k = 0; k < taille; ++k) {
        int x = i + (rot == 3 ? k : rot == 1 ? -k : 0);
        int y = j + (rot == 2 ? k : rot == 4 ? -k : 0);
        grille[x][y][0] = symbole;   // Place le symbole du bateau
        grille[x][y][1] = '\0';
    }
}

// Affiche la grille complète (bateaux visibles)
void afficherGrille(char grille[DIM][DIM][3]) {
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            printf("%s ", grille[i][j]);
        }
        printf("\n");
    }
}

// Affiche la grille des tirs effectués sur l'adversaire (cache les bateaux non touchés)
void afficherGrilleEnnemie(char grille[DIM][DIM][3]) {
    printf("Grille ennemie (vos tirs) :\n");
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            if (i == 0 || j == 0) {
                printf("%s ", grille[i][j]); // Affiche entêtes
            } else if (strcmp(grille[i][j], "X") == 0 || strcmp(grille[i][j], "O") == 0) {
                printf("%s ", grille[i][j]); // Affiche coups joués
            } else {
                printf("· "); // Cache le reste
            }
        }
        printf("\n");
    }
}

// Traite un tir reçu sur la grille du serveur
int traiterTir(char grille[DIM][DIM][3], int* vieBateaux, int i, int j, char* reponse) {
    if (!valides(i, j)) {
        strcpy(reponse, "invalide"); // Position hors grille
        return 0;
    }
    // Si la case contient un bateau (ni vide, ni déjà touchée, ni déjà ratée)
    if (strcmp(grille[i][j], "-") != 0 && strcmp(grille[i][j], "X") != 0 && strcmp(grille[i][j], "O") != 0) {
        char symbole = grille[i][j][0];
        grille[i][j][0] = 'X'; grille[i][j][1] = '\0'; // Marque la case comme touchée
        if (vieBateaux[(int)symbole] > 0) vieBateaux[(int)symbole]--; // Décrémente la vie du bateau
        if (vieBateaux[(int)symbole] == 0)
            strcpy(reponse, "coule"); // Bateau coulé
        else
            strcpy(reponse, "touche"); // Bateau touché
        return 1;
    } else if (strcmp(grille[i][j], "-") == 0) {
        strcpy(grille[i][j], "O"); // Marque la case comme ratée
        strcpy(reponse, "eau");    // Tir à l'eau
        return 0;
    } else if (strcmp(grille[i][j], "X") == 0 || strcmp(grille[i][j], "O") == 0) {
        strcpy(reponse, "deja");   // Case déjà jouée
        return 0;
    } else {
        strcpy(reponse, "eau");
        return 0;
    }
}

// Demande à l'utilisateur (serveur) d'entrer un coup à jouer
void demanderTir(char* coup) {
    printf("À vous de tirer !\n");
    printf("Entrez votre coup (ex: B5) : ");
    scanf("%s", coup);
}

// Placement manuel des bateaux pour le serveur
void placementManuel(char grille[DIM][DIM][3], Bateau flotte[]) {
    printf("=== Placement manuel des bateaux (Serveur) ===\n");
    for (int b = 0; b < 5; b++) {
        int ok = 0;
        while (!ok) {
            afficherGrille(grille);
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
    srand(time(NULL)); // Initialise le générateur aléatoire (utile si placement auto)
    // Création de la socket serveur
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(12345);
    bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(sockfd, 1);
    printf("En attente d'un client...\n");
    int client_sock = accept(sockfd, NULL, NULL); // Attend la connexion d'un client
    printf("Client connecté !\n");

    // Synchronisation avec le client (attend "pret")
    char syncbuf[16];
    int syncn = recv(client_sock, syncbuf, sizeof(syncbuf)-1, 0);
    if (syncn <= 0 || strncmp(syncbuf, "pret", 4) != 0) {
        printf("Erreur de synchronisation avec le client.\n");
        close(client_sock);
        close(sockfd);
        return 1;
    }

    // Initialisation des grilles et de la flotte du serveur
    char grilleServeur[DIM][DIM][3];
    initialiserGrille(grilleServeur);
    Bateau flotteServeur[5] = { {'#',5,5,1},{'@',4,4,1},{'%',3,3,1},{'&',3,3,1},{'$',2,2,1} };
    int vieServeur[128] = {0};
    vieServeur['#'] = 5; vieServeur['@'] = 4; vieServeur['%'] = 3; vieServeur['&'] = 3; vieServeur['$'] = 2;
    placementManuel(grilleServeur, flotteServeur);

    // Indique au client que le serveur est prêt
    send(client_sock, "pret", 4, 0);

    // Initialisation des buffers et de la grille des tirs sur l'adversaire
    char buffer[32], reponse[16];
    char grilleEnnemie[DIM][DIM][3];
    initialiserGrille(grilleEnnemie);

    int n;
    char lettre;
    int chiffre;

    // Boucle principale du jeu
    while (1) {
        int coup_valide = 0;
        // Boucle pour recevoir un coup valide du client
        while (!coup_valide) {
            n = recv(client_sock, buffer, sizeof(buffer)-1, 0); // Attend le coup du client
            if (n <= 0) goto fin_serveur; // Déconnexion
            buffer[n] = 0;
            char lettre;
            int chiffre;
            // Vérifie le format du coup
            if (sscanf(buffer, " %c%d", &lettre, &chiffre) != 2) {
                send(client_sock, "invalide", 8, 0);
                continue;
            }
            int i = lettreVersIndice(lettre);
            int j = chiffre;
            // Vérifie la validité de la case
            if (!valides(i, j)) {
                send(client_sock, "invalide", 8, 0);
                continue;
            }
            // Vérifie que la case n'a pas déjà été jouée
            if (strcmp(grilleServeur[i][j], "X") == 0 || strcmp(grilleServeur[i][j], "O") == 0) {
                send(client_sock, "invalide", 8, 0);
                continue;
            }
            // Traite le tir du client
            traiterTir(grilleServeur, vieServeur, i, j, reponse);
            send(client_sock, reponse, strlen(reponse), 0);

            printf("Le client a tiré sur %c%d : %s\n", lettre, chiffre, reponse);
            afficherGrille(grilleServeur);
            coup_valide = 1;
        }

        // Vérifie si tous les bateaux du serveur sont coulés (défaite)
        if (vieServeur['#'] == 0 && vieServeur['@'] == 0 && vieServeur['%'] == 0 && vieServeur['&'] == 0 && vieServeur['$'] == 0) {
            const char* msgVictoire = "victoire: Bravo ! Vous avez coulé tous les bateaux du serveur !";
            send(client_sock, msgVictoire, strlen(msgVictoire), 0);
            printf("\n=== FIN DE PARTIE ===\n");
            printf("Bravo ! Le client a coulé tous vos bateaux et remporte la victoire !\n");
            printf("=====================\n");
            break;
        }

        // Tour du serveur pour tirer sur le client
        char coup[16];
        int tir_valide = 0;
        while (!tir_valide) {
            demanderTir(coup); // Demande à l'utilisateur d'entrer un coup
            char lettreTir;
            int chiffreTir;
            if (sscanf(coup, " %c%d", &lettreTir, &chiffreTir) != 2) {
                printf("Coup invalide, réessayez.\n");
                continue;
            }
            int iTir = lettreVersIndice(lettreTir);
            int jTir = chiffreTir;
            if (!valides(iTir, jTir)) {
                printf("Coup hors grille, réessayez.\n");
                continue;
            }
            if (strcmp(grilleEnnemie[iTir][jTir], "X") == 0 || strcmp(grilleEnnemie[iTir][jTir], "O") == 0) {
                printf("Vous avez déjà tiré ici, réessayez.\n");
                continue;
            }
            tir_valide = 1;
        }
        send(client_sock, coup, strlen(coup), 0); // Envoie le coup au client

        n = recv(client_sock, reponse, sizeof(reponse)-1, 0); // Attend la réponse du client
        if (n <= 0) break;
        reponse[n] = 0;
        printf("Réponse du client à %s : %s\n", coup, reponse);

        // Met à jour la grille ennemie selon la réponse
        if (sscanf(coup, " %c%d", &lettre, &chiffre) == 2) {
            int i = lettreVersIndice(lettre);
            int j = chiffre;
            if (strcmp(reponse, "touche") == 0 || strcmp(reponse, "coule") == 0) {
                strcpy(grilleEnnemie[i][j], "X");
            } else if (strcmp(reponse, "eau") == 0) {
                strcpy(grilleEnnemie[i][j], "O");
            }
        }
        
        afficherGrille(grilleServeur);        // Affiche la grille du serveur
        afficherGrilleEnnemie(grilleEnnemie); // Affiche la grille des tirs du serveur

        // Vérifie si le client a perdu (le serveur a gagné)
        if (strcmp(reponse, "victoire") == 0) {
            printf("Le serveur a gagné !\n");
            break;
        }
    }

fin_serveur:
    close(client_sock); // Ferme la connexion client
    close(sockfd);      // Ferme la socket serveur
    return 0;
}