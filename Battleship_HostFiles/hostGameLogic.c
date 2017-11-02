#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "hostGameLogic.h"
#include "hostUI.h"
#define NUMSIZE 4
#define DEFAULTGAMES 3
#define HIT_ALREADY 'h'
#define PLAYER1WIN 1
#define PLAYER2WIN 2
#define DRAW 3
#define BOTHLOSE 4

void initializeBattleShip(BattleShip *ship) {
   ship->aircraft_size = SIZE_AIRCRAFT_CARRIER;
   ship->battleship_size = SIZE_BATTLESHIP;
   ship->destroyer_size = SIZE_DESTROYER;
   ship->submarine_size = SIZE_SUBMARINE;
   ship->patrol_size = SIZE_PATROL_BOAT;
   ship->numShips = NUMBER_OF_SHIPS;
}

void initializeShotCounter(ShotCounter *shotcounter) {
   shotcounter->missCounter = 0;
   shotcounter->hitCounter = 0;
   shotcounter->sinkCounter = 0;
   shotcounter->numShots = 0;
}

void initializePlayer(Player *player) {
   player->wins = 0;
   player->draws = 0;
   player->losses = 0;
}

static void sendMsgNewGame(int readFd, int writeFd,
   char playerBoard[SIZE][SIZE]) {

   int new_game;
   int i, j;

   new_game = NEW_GAME;
   /* send NEW_GAME message to player */
   if (write(writeFd, &new_game, sizeof(int)) == -1) {
      perror(NULL);
      exit(EXIT_FAILURE);
   }
   for (i=0; i<SIZE; i++) {
      for (j=0; j<SIZE; j++) {
         read(readFd, &playerBoard[i][j], sizeof(char));
      }
   }
}

static void sendShotRequest(int readFd, int writeFd, Shot *shot) {
   int shot_request;
   int status;

   shot_request = SHOT_REQUEST;
   if (write(writeFd, &shot_request, sizeof(int)) == -1) {
      perror(NULL);
      exit(EXIT_FAILURE);
   }
   status = read(readFd, shot, sizeof(Shot));
   if (status == -1) {
      perror(NULL);
      exit(EXIT_FAILURE);
   }
}

static void sendShotResult(int writeFd, int *shotResult) {
   int shot_result;

   shot_result = SHOT_RESULT;
   write(writeFd, &shot_result, sizeof(int));
   write(writeFd, shotResult, sizeof(int));
}

static void sendOpponentsShot(int writeFd, Shot *shot) {
   int opponents_shot;

   opponents_shot = OPPONENTS_SHOT;
   write(writeFd, &opponents_shot, sizeof(Shot));
   write(writeFd, shot, sizeof(Shot));
}

static void sendMatchOver(int writeFd1, int writeFd2) {
   int match_over;

   match_over = MATCH_OVER;
   write(writeFd1, &match_over, sizeof(int));
   write(writeFd2, &match_over, sizeof(int));
}

void setActualShot(char opponentBoard[SIZE][SIZE], int row, int col,
   BattleShip *opponentShip, int *actualShot, int *type_size) {

   opponentBoard[row][col] = HIT_ALREADY;
   (*type_size)--;
   *actualShot = HIT;
   if (*type_size == 0) {
      opponentShip->numShips--;
      *actualShot = SINK;
   }
}

int getShotAndDetermineShot(char playerBoard[SIZE][SIZE],
   char opponentBoard[SIZE][SIZE], Shot *playerShot,
   BattleShip *opponentShip) {

   int actualShot;
   int row, col;

   row = playerShot->row;
   col = playerShot->col;

   if (row < 0 || row > 9 || col < 0 || col > 9) {
      actualShot = MISS;
   }
   else if (opponentBoard[row][col] == OPEN_WATER) {
      actualShot = MISS;
   }
   else if (opponentBoard[row][col] == AIRCRAFT_CARRIER) {
      setActualShot(opponentBoard, row, col, opponentShip,
         &actualShot, &(opponentShip->aircraft_size));
   }
   else if (opponentBoard[row][col] == BATTLESHIP) {
      setActualShot(opponentBoard, row, col, opponentShip,
         &actualShot, &(opponentShip->battleship_size));
   }
   else if (opponentBoard[row][col] == DESTROYER) {
      setActualShot(opponentBoard, row, col, opponentShip,
         &actualShot, &(opponentShip->destroyer_size));
   }
   else if (opponentBoard[row][col] == SUBMARINE) {
      setActualShot(opponentBoard, row, col, opponentShip,
         &actualShot, &(opponentShip->submarine_size));
   }
   else if (opponentBoard[row][col] == PATROL_BOAT) {
      setActualShot(opponentBoard, row, col, opponentShip,
         &actualShot, &(opponentShip->patrol_size));
   }
   else if (opponentBoard[row][col] == HIT_ALREADY) {
      actualShot = HIT;
   }
   return actualShot;
}

void addToShotCounter(int playerShotVal, ShotCounter *shotCounter) {
   shotCounter->numShots++;
   if (playerShotVal == HIT) {
      shotCounter->hitCounter++;
   }
   else if (playerShotVal == MISS) {
      shotCounter->missCounter++;
   }
   else if (playerShotVal == SINK) {
      shotCounter->sinkCounter++;
      shotCounter->hitCounter++;
   }
}

int determineGameResult(ShotCounter player1ShotCounter,
   ShotCounter player2ShotCounter, BattleShip player1Ship,
   BattleShip player2Ship) {

   int result = 0;
   if (player1Ship.numShips == 0 && player2Ship.numShips == 0) {
      result = DRAW;
   }
   else if (player2Ship.numShips == 0) {
      result = PLAYER1WIN;
   }
   else if (player1Ship.numShips == 0) {
      result = PLAYER2WIN;
   }
   else if (player1ShotCounter.numShots == MAX_SHOTS ||
      player2ShotCounter.numShots == MAX_SHOTS) {

      result = BOTHLOSE;
   }
   return result;
}

void convertShotValToString(int shotVal, char stringShotVal[6]) {

   if (shotVal == MISS) {
      strcpy(stringShotVal, "Miss");
   }
   else if (shotVal == HIT) {
      strcpy(stringShotVal, "HIT!");
   }
   else if (shotVal == SINK) {
      strcpy(stringShotVal, "SINK!");
   }
}

int runBattleshipGame(int dFlag, int p1Pipe1[2], int p1Pipe2[2],
   int p2Pipe1[2], int p2Pipe2[2],
   char player1Board[SIZE][SIZE], char player2Board[SIZE][SIZE], int i,
   char *firstPlayerName, char *secondPlayerName) {

   int gameOver = 0;
   int player1ShotVal, player2ShotVal;
   Shot player1Shot, player2Shot;
   BattleShip player1Ship, player2Ship;
   ShotCounter player1ShotCounter, player2ShotCounter;
   int result;
   char stringVal1[6], stringVal2[6];

   initializeBattleShip(&player1Ship);
   initializeBattleShip(&player2Ship);
   initializeShotCounter(&player1ShotCounter);
   initializeShotCounter(&player2ShotCounter);

   if (dFlag == 1) {
      printf("\nGame %d Details:\n", i+1);
   }
   while (gameOver != 1) {
      /* player 1 turn */
      sendShotRequest(p1Pipe2[0], p1Pipe1[1], &player1Shot);
      player1ShotVal = getShotAndDetermineShot(player1Board,
         player2Board, &player1Shot, &player2Ship);
      addToShotCounter(player1ShotVal, &player1ShotCounter);
      sendShotResult(p1Pipe1[1], &player1ShotVal);
      sendOpponentsShot(p2Pipe1[1], &player1Shot);

      /* player 2 turn */
      sendShotRequest(p2Pipe2[0], p2Pipe1[1], &player2Shot);
      player2ShotVal = getShotAndDetermineShot(player2Board,
         player1Board, &player2Shot, &player1Ship);
      addToShotCounter(player2ShotVal, &player2ShotCounter);
      sendShotResult(p2Pipe1[1], &player2ShotVal);
      sendOpponentsShot(p1Pipe1[1], &player2Shot);

      convertShotValToString(player1ShotVal, stringVal1);
      convertShotValToString(player2ShotVal, stringVal2);

      conditionalDFlagShotPrints(dFlag, player1Shot, player2Shot,
         player1ShotCounter, player2ShotCounter, stringVal1, stringVal2,
         firstPlayerName, secondPlayerName);

      result = determineGameResult(player1ShotCounter, player2ShotCounter,
         player1Ship, player2Ship);

      if (result != 0) {
         gameOver = 1;
      }
   }
   /* print game results if -d flag specified */
   if (dFlag == 1) {
      conditionalDFlagGameResults(dFlag, result, i, player1ShotCounter,
         player2ShotCounter, firstPlayerName, secondPlayerName);
   }
   return result;
}

void determineWinLoss(int result, Player *player1, Player *player2) {
   if (result == PLAYER1WIN) {
      player1->wins++;
      player2->losses++;
   }
   else if (result == PLAYER2WIN) {
      player2->wins++;
      player1->losses++;
   }
   else if (result == DRAW) {
      player1->draws++;
      player2->draws++;
   }
   else if (result == BOTHLOSE) {
      player1->losses++;
      player2->losses++;
   }
}

void runBattleshipMatch(int gFlag, int dFlag, int p1Pipe1[2], int p1Pipe2[2],
   int p2Pipe1[2], int p2Pipe2[2], char player1Board[SIZE][SIZE],
   char player2Board[SIZE][SIZE], char *firstName, char *secondName) {

   int i;
   int result;
   Player player1, player2;

   initializePlayer(&player1);
   initializePlayer(&player2);

   if (gFlag == 0) {
      gFlag = DEFAULTGAMES;
   }
   /* loop thru number of games */
   for (i = 0; i < gFlag; i++) {
      sendMsgNewGame(p1Pipe2[0], p1Pipe1[1], player1Board);
      sendMsgNewGame(p2Pipe2[0], p2Pipe1[1], player2Board);
      result = runBattleshipGame(dFlag, p1Pipe1, p1Pipe2, p2Pipe1, p2Pipe2,
         player1Board, player2Board, i, firstName, secondName);

      determineWinLoss(result, &player1, &player2);
   }
   sendMatchOver(p1Pipe1[1], p2Pipe1[1]);
   printMatchResults(player1, player2, gFlag, firstName, secondName);
}