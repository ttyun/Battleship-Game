#ifndef HOSTUI_H
#define HOSTUI_H
#include "hostGameLogic.h"

void checkArgs(int argc, char **argv, int *gVal, int *dFlag, int *firstPlayer, int *secondPlayer);
void conditionalDFlagShotPrints(int dFlag, Shot player1Shot, Shot player2Shot,
   ShotCounter player1ShotCounter, ShotCounter player2ShotCounter,
   char stringVal1[6], char stringVal2[6], char *firstPlayerName, char *secondPlayerName);

char *getPlayerName(char *argv);
void conditionalDFlagGameResults (int dFlag, int result, int i,
   ShotCounter player1ShotCounter, ShotCounter player2ShotCounter,
   char *firstPlayerName, char *secondPlayerName);

void printMatchResults(Player player1, Player player2, int games,
   char *firstPlayerName, char *secondPlayerName);

#endif
