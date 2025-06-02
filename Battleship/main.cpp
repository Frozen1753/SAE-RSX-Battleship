#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

const int DIM = 11;
const vector<string> ALPHA = { "A","B","C","D","E","F","G","H","I","J" };

// Représente un bateau
struct Bateau {
    char symbole;
    int taille;
    int restants;
};

// Initialise une grille avec les entêtes
vector<vector<string>> initialiserGrille() {
    vector<vector<string>> grille(DIM, vector<string>(DIM, "-"));
    grille[0][0] = " ";
    for (int i = 1; i < DIM; ++i) {
        grille[0][i] = to_string(i);
        grille[i][0] = ALPHA[i - 1];
    }
    return grille;
}

// Affiche une grille
void afficherGrille(const vector<vector<string>>& grille, bool cacher = false) {
    for (const auto& ligne : grille) {
        for (const auto& cell : ligne) {
            if (cacher && (cell == "#" || cell == "@" || cell == "%" || cell == "&" || cell == "$"))
                cout << "- ";
            else
                cout << cell << " ";
        }
        cout << endl;
    }
}

// Convertit une lettre en index
int lettreVersIndice(const string& lettre) {
    if (lettre.length() != 1) return 0;
    char c = toupper(lettre[0]);
    return (c >= 'A' && c <= 'J') ? (c - 'A' + 1) : 0;
}

// Vérifie si les coordonnées sont valides
bool valides(int i, int j) {
    return i >= 1 && i < DIM && j >= 1 && j < DIM;
}

// Vérifie si un bateau peut être placé
bool peutPlacer(int taille, int rot, int i, int j, const vector<vector<string>>& grille) {
    for (int k = 0; k < taille; ++k) {
        int x = i + (rot == 3 ? k : rot == 1 ? -k : 0);
        int y = j + (rot == 2 ? k : rot == 4 ? -k : 0);
        if (!valides(x, y) || grille[x][y] != "-") return false;
    }
    return true;
}

// Place un bateau sur la grille
void placerBateau(int taille, int rot, int i, int j, char symbole, vector<vector<string>>& grille) {
    for (int k = 0; k < taille; ++k) {
        int x = i + (rot == 3 ? k : rot == 1 ? -k : 0);
        int y = j + (rot == 2 ? k : rot == 4 ? -k : 0);
        grille[x][y] = string(1, symbole);
    }
}

// Fonction principale pour le placement des bateaux
void placement(vector<vector<string>>& grille, int joueur) {
    map<int, Bateau> flotte = {
        {1, {'#', 5, 1}},
        {2, {'@', 4, 1}},
        {3, {'%', 3, 1}},
        {4, {'&', 3, 1}},
        {5, {'$', 2, 1}}
    };

    cout << "Placement des bateaux du Joueur " << joueur << endl;
    while (!flotte.empty()) {
        afficherGrille(grille);
        cout << "Choisissez un bateau (1-5) :\n";
        for (auto& elem : flotte) {
            int id = elem.first;
            Bateau& b = elem.second;
            cout << id << ". Bateau taille " << b.taille << " symbole '" << b.symbole << "'\n";
        }

        int choix;
        cin >> choix;
        if (!flotte.count(choix)) continue;
        Bateau b = flotte[choix];

        cout << "Lettre (A-J) : ";
        string l; cin >> l;
        int i = lettreVersIndice(l);

        cout << "Chiffre (1-10) : ";
        int j; cin >> j;

        cout << "Rotation (1=nord, 2=est, 3=sud, 4=ouest) : ";
        int rot; cin >> rot;

        if (!peutPlacer(b.taille, rot, i, j, grille)) {
            cout << "Placement invalide.\n";
            continue;
        }

        placerBateau(b.taille, rot, i, j, b.symbole, grille);
        flotte.erase(choix);
        system("cls");
    }
}

bool tirer(vector<vector<string>>& grilleAdverse, vector<vector<string>>& grilleTirs, map<char, int>& vieBateaux) {
    cout << "Lettre (A-J) : ";
    string l;
    cin >> l;
    int i = lettreVersIndice(l);

    cout << "Chiffre (1-10) : ";
    int j;
    cin >> j;

    if (!valides(i, j)) {
        cout << "Coordonnées invalides.\n";
        return false;
    }

    string& caseAdverse = grilleAdverse[i][j];
    string& caseTirs = grilleTirs[i][j];

    if (caseTirs != "-") {
        cout << "Vous avez déjà tiré ici.\n";
        return false;
    }

    if (caseAdverse != "-" && caseAdverse != "X") {
        cout << "Touché !\n";
        caseTirs = "X";
        char symbole = caseAdverse[0];
        caseAdverse = "X";
        vieBateaux[symbole]--;

        if (vieBateaux[symbole] == 0) {
            cout << "Bateau coulé !\n";
        }

        return true;
    }
    else {
        cout << "À l'eau.\n";
        caseTirs = "O";
        return false;
    }
}

void jouer() {
    vector<vector<string>> grilleJ1 = initialiserGrille();
    vector<vector<string>> grilleJ2 = initialiserGrille();
    vector<vector<string>> tirsJ1 = initialiserGrille();
    vector<vector<string>> tirsJ2 = initialiserGrille();

    placement(grilleJ1, 1);
    placement(grilleJ2, 2);

    // Map des points de vie restants par symbole
    map<char, int> vieJ1 = { {'#',5},{'@',4},{'%',3},{'&',3},{'$',2} };
    map<char, int> vieJ2 = vieJ1;

    int tour = 1;
    while (true) {
        system("cls");
        cout << "Tour du Joueur " << tour << endl;
        if (tour == 1) {
            cout << "Votre grille de tirs :\n";
            afficherGrille(tirsJ1);
            if (tirer(grilleJ2, tirsJ1, vieJ2)) {}
            if (all_of(vieJ2.begin(), vieJ2.end(), [](auto& p) { return p.second == 0; })) {
                cout << "Joueur 1 a gagné !\n";
                break;
            }
        }
        else {
            cout << "Votre grille de tirs :\n";
            afficherGrille(tirsJ2);
            if (tirer(grilleJ1, tirsJ2, vieJ1)) {}
            if (all_of(vieJ1.begin(), vieJ1.end(), [](auto& p) { return p.second == 0; })) {
                cout << "Joueur 2 a gagné !\n";
                break;
            }
        }
        tour = 3 - tour; // Alterne entre 1 et 2
        system("pause");
    }
}


int main() {
    srand(time(0));
    jouer();
    return 0;
}