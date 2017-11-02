#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hostUI.h"
#define NUMSIZE 4
#define DEFAULTGAMES 3
#define HIT_ALREADY 'h'
#define PLAYER1WIN 1
#define PLAYER2WIN 2
#define DRAW 3
#define BOTHLOSE 4

static void printUsageMsg() {
   fprintf(stderr, "Usage: battleship [-gN|-d] player1 player2\n");
   exit(EXIT_FAILURE);
}

static void checkFlags(char **argv, int *gVal, int *dFlag, int i) {
   if (argv[i][1] == 'g') {
      if (1 != sscanf(argv[i], "-g%d", gVal)) {
         printUsageMsg();
      }
      if (*gVal < 1) {
         printUsageMsg();
      }
   }
   else if (argv[i][1] == 'd') {
      if (argv[i][2] == 0) {
         *dFlag = 1;
      }
      else {
         printUsageMsg();
      }
   }
   else {
      printUsageMsg();
   }
}

static void checkNumPlayers(int *firstPlayer, int *secondPlayer,
   int numPlayers) {

   if (*firstPlayer == 0 && *secondPlayer == 0) {
      printUsageMsg();
   }
   if (numPlayers != 2) {
      printUsageMsg();
   }
}

static void setIndexPlayers(int *firstPlayer, int *secondPlayer, int i) {
   if (*firstPlayer == 0 && *secondPlayer == 0) {
      *firstPlayer = i;
   }
   else if (*firstPlayer > 0 && *secondPlayer == 0) {
      *secondPlayer = i;
   }
}

void checkArgs(int argc, char **argv, int *gVal,
   int *dFlag, int *firstPlayer, int *secondPlayer) {

   int i;
   int numPlayers = 0;
   for (i = 1; i < argc; i++) {
      if (argv[i][0] == '-') {
         checkFlags(argv, gVal, dFlag, i);
      }
      else {
         numPlayers++;
         setIndexPlayers(firstPlayer, secondPlayer, i);
      }
   }
   checkNumPlayers(firstPlayer, secondPlayer, numPlayers);
}

char *getPlayerName(char *argv) {
   char *playerName;
   int lengthPathName;

   if ((playerName = strstr(argv, "/")) == NULL) {
      return argv;
   }
   do {
      lengthPathName = strlen(playerName) + 1;
      argv = &argv[strlen(argv) - lengthPathName + 2];
      playerName = strstr(argv, "/");
   } while (playerName);
   return argv;
}

void conditionalDFlagShotPrints(int dFlag, Shot player1Shot, Shot player2Shot,
   ShotCounter player1ShotCounter, ShotCounter player2ShotCounter,
   char stringVal1[6], char stringVal2[6], char *firstPlayerName,
   char *secondPlayerName) {

   if (dFlag == 1) {
      printf("%16s: shot[%2d][%2d], %5s,", firstPlayerName,
         player1Shot.row, player1Shot.col, stringVal1);
      printf(" shots:%3d, misses:%2d, hits:%2d, sunk:%d\n",
         player1ShotCounter.numShots, player1ShotCounter.missCounter,
         player1ShotCounter.hitCounter, player1ShotCounter.sinkCounter);

      printf("%16s: shot[%2d][%2d], %5s,", secondPlayerName,
         player2Shot.row, player2Shot.col, stringVal2);
      printf(" shots:%3d, misses:%2d, hits:%2d, sunk:%d\n",
         player2ShotCounter.numShots, player2ShotCounter.missCounter,
         player2ShotCounter.hitCounter, player2ShotCounter.sinkCounter);
   }
}

void conditionalDFlagGameResults (int dFlag, int result, int i,
   ShotCounter player1ShotCounter, ShotCounter player2ShotCounter,
   char *firstPlayerName, char *secondPlayerName) {

   printf("\n");
   if (result == PLAYER1WIN) {
      printf("Game %d Results: %s won!\n", i+1, firstPlayerName);
   }
   else if (result == PLAYER2WIN) {
      printf("Game %d Results: %s won!\n", i+1, secondPlayerName);
   }
   else if (result == DRAW) {
      printf("Game %d Results: Draw\n", i+1);
   }
   else if (result == BOTHLOSE) {
      printf("Game %d Results: No winner within 100 shots\n", i+1);
   }
   printf("%16s: %2d shots, %d hits, and %d sinks\n", firstPlayerName,
      player1ShotCounter.numShots, player1ShotCounter.hitCounter,
      player1ShotCounter.sinkCounter);
   printf("%16s: %2d shots, %d hits, and %d sinks\n", secondPlayerName,
      player2ShotCounter.numShots, player2ShotCounter.hitCounter,
      player2ShotCounter.sinkCounter);
}

void printMatchResults(Player player1, Player player2, int games,
   char *firstPlayerName, char *secondPlayerName) {

   printf("\n");
   if (player1.wins > player2.wins) {
      printf("Match Results: %s won!\n", firstPlayerName);
   }
   else if (player2.wins > player1.wins) {
      printf("Match Results: %s won!\n", secondPlayerName);
   }
   else {
      printf("Match Results: No winner in %d games\n", games);
   }
   printf("%16s: %d wins, %d draws, and %d losses\n", firstPlayerName,
      player1.wins, player1.draws, player1.losses);
   printf("%16s: %d wins, %d draws, and %d losses\n", secondPlayerName,
      player2.wins, player2.draws, player2.losses);
}