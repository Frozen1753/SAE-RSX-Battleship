#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>

#define DIM 11
const char* ALPHA[] = { "A","B","C","D","E","F","G","H","I","J" };

// Structure représentant un bateau
typedef struct {
    char symbole;    // Symbole du bateau sur la grille
    int taille;      // Taille du bateau
    int restants;    // Cases restantes (non utilisées ici)
    int actif;       // Statut actif (non utilisé ici)
} Bateau;

// Initialise la grille de jeu avec les entêtes et les cases vides
void initialiserGrille(char grille[DIM][DIM][3]) {
    strcpy(grille[0][0], " ");
    for (int i = 1; i < DIM; i++) {
        snprintf(grille[0][i], 3, "%d", i);         // Chiffres en haut
        strcpy(grille[i][0], ALPHA[i - 1]);         // Lettres à gauche
    }
    for (int i = 1; i < DIM; i++)
        for (int j = 1; j < DIM; j++)
            strcpy(grille[i][j], "-");              // Cases vides
}

// Affiche la grille de jeu
void afficherGrille(char grille[DIM][DIM][3]) {
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            printf("%s ", grille[i][j]);
        }
        printf("\n");
    }
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
        if (!valides(x, y) || strcmp(grille[x][y], "-") != 0) return 0;
    }
    return 1;
}

// Place un bateau sur la grille
void placerBateau(int taille, int rot, int i, int j, char symbole, char grille[DIM][DIM][3]) {
    for (int k = 0; k < taille; ++k) {
        int x = i + (rot == 3 ? k : rot == 1 ? -k : 0);
        int y = j + (rot == 2 ? k : rot == 4 ? -k : 0);
        grille[x][y][0] = symbole;
        grille[x][y][1] = '\0';
    }
}

// Placement manuel des bateaux par le joueur
void placementManuel(char grille[DIM][DIM][3], Bateau flotte[]) {
    printf("Placement de vos bateaux :\n");
    for (int b = 0; b < 5; b++) {
        int ok = 0;
        while (!ok) {
            afficherGrille(grille);
            printf("Bateau %d (taille %d, symbole %c)\n", b+1, flotte[b].taille, flotte[b].symbole);
            printf("Lettre (A-J) : ");
            char lettre;
            scanf(" %c", &lettre);
            int i = lettreVersIndice(lettre);
            printf("Chiffre (1-10) : ");
            int j;
            scanf("%d", &j);
            printf("Rotation (1=nord, 2=est, 3=sud, 4=ouest) : ");
            int rot;
            scanf("%d", &rot);
            if (peutPlacer(flotte[b].taille, rot, i, j, grille)) {
                placerBateau(flotte[b].taille, rot, i, j, flotte[b].symbole, grille);
                ok = 1;
            } else {
                printf("Placement invalide, recommencez.\n");
            }
        }
    }
}

// Traite un tir reçu sur la grille du joueur
// Cette fonction vérifie si le tir est valide, met à jour la grille et les vies des bateaux, et prépare la réponse à envoyer à l'adversaire.
int traiterTir(char grille[DIM][DIM][3], int* vieBateaux, int i, int j, char* reponse) {
    // Vérifie si la position du tir est dans la grille
    if (!valides(i, j)) {
        strcpy(reponse, "invalide"); // Position hors grille
        return 0;
    }
    // Si la case contient un bateau (ni vide, ni déjà touchée, ni déjà ratée)
    if (strcmp(grille[i][j], "-") != 0 && strcmp(grille[i][j], "X") != 0 && strcmp(grille[i][j], "O") != 0) {
        char symbole = grille[i][j][0]; // Récupère le symbole du bateau touché
        grille[i][j][0] = 'X'; // Marque la case comme touchée
        grille[i][j][1] = '\0';
        vieBateaux[symbole]--; // Décrémente la vie du bateau correspondant
        if (vieBateaux[symbole] == 0)
            strcpy(reponse, "coule"); // Le bateau est coulé
        else
            strcpy(reponse, "touche"); // Le bateau est touché mais pas coulé
        return 1;
    } else if (strcmp(grille[i][j], "-") == 0) {
        // Si la case est vide, c'est raté
        strcpy(grille[i][j], "O"); // Marque la case comme ratée
        strcpy(reponse, "eau");    // Réponse "à l'eau"
        return 0;
    } else {
        // Si la case était déjà touchée ou ratée
        strcpy(reponse, "eau");
        return 0;
    }
}

int main(int argc, char *argv[]) {
    // Vérifie que l'utilisateur a bien fourni l'adresse IP du serveur
    if (argc < 2) {
        printf("Usage: %s <adresse_ip_serveur>\n", argv[0]);
        return 1;
    }
    // Création de la socket TCP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); // Adresse IP du serveur
    serv_addr.sin_port = htons(12345);              // Port du serveur

    // Connexion au serveur
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connexion échouée"); // Affiche une erreur si la connexion échoue
        return 1;
    }
    printf("Connecté au serveur !\n");

    // Initialisation des grilles de jeu (joueur et tirs) et de la flotte du joueur
    char grilleJoueur[DIM][DIM][3], grilleTirs[DIM][DIM][3];
    initialiserGrille(grilleJoueur); // Grille où sont placés les bateaux du joueur
    initialiserGrille(grilleTirs);   // Grille où le joueur note ses tirs
    Bateau flotteJoueur[5] = { {'#',5,5,1},{'@',4,4,1},{'%',3,3,1},{'&',3,3,1},{'$',2,2,1} };
    int vieJoueur[128] = {0}; // Tableau pour suivre les points de vie de chaque type de bateau
    vieJoueur['#'] = 5; vieJoueur['@'] = 4; vieJoueur['%'] = 3; vieJoueur['&'] = 3; vieJoueur['$'] = 2;

    // Placement manuel des bateaux par le joueur
    placementManuel(grilleJoueur, flotteJoueur);

    // Synchronisation avec le serveur pour s'assurer que les deux joueurs sont prêts
    send(sockfd, "pret", 4, 0);

    char syncbuf[16];
    int syncn = recv(sockfd, syncbuf, sizeof(syncbuf)-1, 0);
    if (syncn <= 0 || strncmp(syncbuf, "pret", 4) != 0) {
        printf("Erreur de synchronisation avec le serveur.\n");
        close(sockfd);
        return 1;
    }

    // Variables pour la boucle de jeu
    char buffer[32], reponse[16];
    char coup[16];
    char lettre;
    int chiffre;
    int n;

    // Boucle principale du jeu : alternance des tours entre le joueur et le serveur
    while (1) {
        int tir_valide = 0;
        // Tour du joueur : il doit entrer un coup valide
        while (!tir_valide) {
            afficherGrille(grilleTirs); // Affiche la grille des tirs précédents
            printf("A vous de tirer !\n");
            printf("Entrez votre coup (ex: B5) : ");
            scanf("%s", coup);

            char lettreTir;
            int chiffreTir;
            // Vérifie que l'entrée est correcte (lettre + chiffre)
            if (sscanf(coup, " %c%d", &lettreTir, &chiffreTir) != 2) {
                printf("Coup invalide, réessayez.\n");
                continue;
            }
            int iTir = lettreVersIndice(lettreTir);
            int jTir = chiffreTir;
            // Vérifie que la case est bien sur la grille
            if (!valides(iTir, jTir)) {
                printf("Coup hors grille, réessayez.\n");
                continue;
            }
            // Vérifie que le joueur n'a pas déjà tiré ici
            if (strcmp(grilleTirs[iTir][jTir], "X") == 0 || strcmp(grilleTirs[iTir][jTir], "O") == 0) {
                printf("Vous avez déjà tiré ici, réessayez.\n");
                continue;
            }

            // Envoie le coup au serveur
            send(sockfd, coup, strlen(coup), 0);

            // Attend la réponse du serveur (touche, coule, eau, invalide, victoire)
            n = recv(sockfd, reponse, sizeof(reponse)-1, 0);
            if (n <= 0) {
                printf("Déconnexion du serveur.\n");
                exit(1);
            }
            reponse[n] = 0;
            printf("Réponse du serveur : %s\n", reponse);

            // Si le coup est refusé, recommencer
            if (strcmp(reponse, "invalide") == 0) {
                printf("Coup refusé par le serveur, recommencez.\n");
                continue;
            }

            tir_valide = 1; // Coup accepté
        }

        // Met à jour la grille des tirs selon la réponse du serveur
        if (sscanf(coup, " %c%d", &lettre, &chiffre) == 2) {
            int ti = lettreVersIndice(lettre);
            int tj = chiffre;
            if (strncmp(reponse, "touche", 6) == 0 || strncmp(reponse, "coule", 5) == 0)
                strcpy(grilleTirs[ti][tj], "X"); // Marque la case comme touchée
            else if (strncmp(reponse, "eau", 3) == 0)
                strcpy(grilleTirs[ti][tj], "O"); // Marque la case comme ratée
        }

        // Vérifie si le joueur a gagné
        if (strcmp(reponse, "victoire") == 0) {
            printf("\n=== FIN DE PARTIE ===\n");
            printf("%s\n", reponse + 9); // Affiche le message après "victoire: "
            printf("=====================\n");
            break;
        }

        // Tour du serveur (adversaire) : reçoit le coup du serveur
        n = recv(sockfd, buffer, sizeof(buffer)-1, 0);
        if (n <= 0) break;
        buffer[n] = 0;
        // Vérifie que le coup reçu est valide
        if (sscanf(buffer, " %c%d", &lettre, &chiffre) != 2) {
            send(sockfd, "invalide", 8, 0);
            continue;
        }
        int i = lettreVersIndice(lettre);
        int j = chiffre;
        // Traite le tir du serveur sur la grille du joueur
        traiterTir(grilleJoueur, vieJoueur, i, j, reponse);
        send(sockfd, reponse, strlen(reponse), 0); // Envoie la réponse au serveur

        printf("Le serveur a tiré sur %c%d : %s\n", lettre, chiffre, reponse);
        afficherGrille(grilleJoueur); // Affiche la grille du joueur après le tir

        // Vérifie si tous les bateaux du joueur sont coulés (défaite)
        if (vieJoueur['#'] == 0 && vieJoueur['@'] == 0 && vieJoueur['%'] == 0 && vieJoueur['&'] == 0 && vieJoueur['$'] == 0) {
            send(sockfd, "victoire", 8, 0); // Informe le serveur de sa victoire
            printf("Le serveur a gagné !\n");
            break;
        }
    }

    close(sockfd); // Ferme la connexion
    return 0;
}