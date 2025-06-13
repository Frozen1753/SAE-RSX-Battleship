cd program
gcc -o client client_battleships.c battleships_mult.c && gcc -o server server_battleships.c battleships_mult.c
mv client ../client
mv server ../server
cd ..