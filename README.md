# Bataille Navale (Battleship) - Version Réseau

Ce projet est une implémentation en C du jeu de la bataille navale (battleship) en mode console, jouable à deux via le réseau (client/serveur, sockets TCP).

## Fonctionnement

- Chaque joueur place ses bateaux sur sa grille.
- Les joueurs jouent chacun leur tour pour tirer sur la grille adverse.
- Le jeu affiche les grilles et indique les touches, les ratés et les bateaux coulés.
- Le premier joueur à couler tous les bateaux adverses gagne la partie.

## Compilation

Assurez-vous d'avoir `gcc` installé.

Dans le dossier du projet, compilez les deux programmes :
```bash
gcc -o serveur Battleship/serveur.c
gcc -o client Battleship/client.c
```

## Utilisation

- **Lancer le serveur** sur la machine qui héberge la partie :
  ```bash
  ./serveur
  ```
- **Lancer le client** sur l'autre machine (ou la même) en précisant l'adresse IP du serveur :
  ```bash
  ./client <adresse_ip_serveur>
  ```

## Communication entre machines (WSL, Linux, Windows)

### Cas classiques

- Si le **client** est sous WSL et le **serveur** sous Linux (ou ailleurs), la connexion fonctionne directement en utilisant l'IP du serveur.
- Si le **serveur** est sous WSL et le **client** sur une autre machine (Linux, Windows, etc.), il faut effectuer une redirection de port pour que le serveur WSL soit accessible depuis l'extérieur.

### Pourquoi une redirection de port est nécessaire avec WSL ?

Par défaut, WSL 2 utilise une interface réseau virtuelle qui n'est pas directement accessible depuis le réseau local. Si vous lancez le serveur dans WSL, il ne sera pas visible depuis une autre machine.  
Pour rendre le serveur WSL accessible, il faut demander à Windows de rediriger les connexions reçues sur un port donné vers l'adresse IP interne de WSL.

### Comment faire la redirection de port

1. **Récupérez l'IP de WSL**  
   Dans WSL, tapez :
   ```bash
   hostname -I
   ```
   Notez l'adresse IP affichée (par exemple, `172.24.112.1`).

2. **Ajoutez la redirection de port sous Windows**  
   Ouvrez PowerShell en administrateur et tapez :
   ```powershell
   netsh interface portproxy add v4tov4 listenport=12345 listenaddress=0.0.0.0 connectport=12345 connectaddress=<IP_WSL>
   ```
   Remplacez `<IP_WSL>` par l'adresse IP trouvée à l'étape précédente.

   **Explication :**  
   Cette commande indique à Windows d'écouter sur le port 12345 de toutes ses interfaces réseau (`listenaddress=0.0.0.0`).  
   Toute connexion reçue sur ce port sera automatiquement transférée vers le port 12345 de l'adresse IP de WSL (`connectaddress=<IP_WSL>`).  
   Cela permet à un client externe de se connecter au serveur qui tourne dans WSL en utilisant simplement l'adresse IP de la machine Windows.

3. **Connectez-vous depuis le client à l'IP de la machine Windows**  
   Sur la machine cliente, utilisez l'adresse IP de la machine Windows comme adresse du serveur.

4. **Vérifiez le pare-feu Windows**  
   Pensez à ouvrir le port 12345 dans le pare-feu Windows si besoin.

5. **Pour supprimer la redirection de port sous Windows**  
   Si vous souhaitez retirer la règle plus tard :
   ```powershell
   netsh interface portproxy delete v4tov4 listenport=12345 listenaddress=0.0.0.0
   ```

## Remarques

- Le jeu fonctionne en mode texte, via le terminal.
- La version réseau nécessite deux exécutables : un serveur et un client.

