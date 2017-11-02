#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "battleship.h"

int getFd(char *arg) {
   int fd;

   if (1 != sscanf(arg, "%d", &fd)) {
      fprintf(stderr, "Bad fd argument\n");
      exit(EXIT_FAILURE);
   }
   return fd;
}

void setAircraftCarrier(char gameBoard[SIZE][SIZE]) {
   int j;
   for (j=0; j<SIZE_AIRCRAFT_CARRIER; j++) {
      gameBoard[0][j] = AIRCRAFT_CARRIER;
   }
}

void setBattleShip(char gameBoard[SIZE][SIZE]) {
   int j;
   for (j=0; j<SIZE_BATTLESHIP; j++) {
      gameBoard[2][j] = BATTLESHIP;
   }
}

void setDestroyer(char gameBoard[SIZE][SIZE]) {
   int j;
   for (j=0; j<SIZE_DESTROYER; j++) {
      gameBoard[5][j] = DESTROYER;
   }
}

void setSubmarine(char gameBoard[SIZE][SIZE]) {
   int i;
   for (i=7; i<SIZE_SUBMARINE + 7; i++) {
      gameBoard[i][0] = SUBMARINE;
   }
}

void setPatrolBoat(char gameBoard[SIZE][SIZE]) {
   int i;
   for (i=8; i<SIZE_PATROL_BOAT + 8; i++) {
      gameBoard[i][7] = PATROL_BOAT;
   }
}

static void sendGameBoard(int writeFd) {
   int i, j;
   char gameBoard[SIZE][SIZE];

   /* set board to open water at first */
   for (i=0; i<SIZE; i++) {
      for (j=0; j<SIZE; j++) {
         gameBoard[i][j] = OPEN_WATER;
      }
   }
   setAircraftCarrier(gameBoard);
   setBattleShip(gameBoard);
   setDestroyer(gameBoard);
   setSubmarine(gameBoard);
   setPatrolBoat(gameBoard);

   for (i=0; i<SIZE; i++) {
      for (j=0; j<SIZE; j++) {
         write(writeFd, &gameBoard[i][j], sizeof(char));
      }
   }
}

static void sendShotStruct(int writeFd, int *row, int *col) {
   Shot playerShot;

   playerShot.row = *row;
   playerShot.col = *col;

   if (*col == 9) {
      (*row)++;
      *col = 0;
   }
   else {
      (*col)++;
   }
   write(writeFd, &playerShot, sizeof(Shot));
}

static void playerLogic(int *hostToPlayerMsg, int *row, int *col,
   int readFd, int writeFd) {

   if (*hostToPlayerMsg == NEW_GAME) {
      *row = 0;
      *col = 0;
      sendGameBoard(writeFd);
   }
   else if (*hostToPlayerMsg == SHOT_REQUEST) {
      sendShotStruct(writeFd, row, col);
   }
   else if (*hostToPlayerMsg == SHOT_RESULT) {
   }
   else if (*hostToPlayerMsg == OPPONENTS_SHOT) {
   }
   read(readFd, hostToPlayerMsg, sizeof(int));
}

int main(int argc, char **argv) {
   int readFd, writeFd;
   int hostToPlayerMsg;
   int row = 0, col = 0;

   readFd = getFd(argv[1]);
   writeFd = getFd(argv[2]);

   read(readFd, &hostToPlayerMsg, sizeof(int));

   while (hostToPlayerMsg != MATCH_OVER) {
      playerLogic(&hostToPlayerMsg, &row, &col, readFd, writeFd);
   }
   return 0;
}