#ifndef HOSTGAMELOGIC_H
#define HOSTGAMELOGIC_H
#include "battleship.h"

typedef struct battleship{
   int aircraft_size;
   int battleship_size;
   int destroyer_size;
   int submarine_size;
   int patrol_size;

   int numShips;
} BattleShip;

typedef struct shotCounter {
   int missCounter;
   int hitCounter;
   int sinkCounter;
   int numShots;
} ShotCounter;

typedef struct player {
   char *name;
   int wins;
   int draws;
   int losses;
} Player;

void runBattleshipMatch(int gFlag, int dFlag, int p1Pipe1[2], int p1Pipe2[2], int p2Pipe1[2], int p2Pipe2[2], char player1Board[SIZE][SIZE],
   char player2Board[SIZE][SIZE], char *firstName, char *secondName);

#endif