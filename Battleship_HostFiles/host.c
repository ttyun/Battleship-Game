#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include "battleship.h"
#include "hostGameLogic.h"
#include "hostUI.h"
#define NUMSIZE 4
#define DEFAULTGAMES 3
#define HIT_ALREADY 'h'
#define PLAYER1WIN 1
#define PLAYER2WIN 2
#define DRAW 3
#define BOTHLOSE 4

static void validateForExec(int readFd, int writeFd,
   char readArg[NUMSIZE], char writeArg[NUMSIZE]) {

   sprintf(readArg, "%d", readFd);
   sprintf(writeArg, "%d", writeFd);
}

void childLogic(int pipeFd1[2], int pipeFd2[2],char readFd[NUMSIZE],
   char writeFd[NUMSIZE], char **argv, int indexPlayer) {

   /* close un-needed pipes */
   close(pipeFd1[1]);
   close(pipeFd2[0]);

   /* turn the fd arguments into char * for execl to work */
   validateForExec(pipeFd1[0], pipeFd2[1], readFd, writeFd);
   execl(argv[indexPlayer], argv[indexPlayer], readFd, writeFd, NULL);
   perror(NULL);
   exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
   int gFlag = 0, dFlag = 0;
   pid_t pid;
   int p1Pipe1[2], p1Pipe2[2];
   int p2Pipe1[2], p2Pipe2[2];
   char readFd1[NUMSIZE], writeFd1[NUMSIZE];
   char readFd2[NUMSIZE], writeFd2[NUMSIZE];
   char player1Board[SIZE][SIZE];
   char player2Board[SIZE][SIZE];
   int indFirstPlayer = 0, indSecondPlayer = 0;
   char *firstPlayerName, *secondPlayerName;

   checkArgs(argc, argv, &gFlag, &dFlag, &indFirstPlayer, &indSecondPlayer);

   firstPlayerName = getPlayerName(argv[indFirstPlayer]);
   secondPlayerName = getPlayerName(argv[indSecondPlayer]);

   /* create the pipes for your first child */
   pipe(p1Pipe1);
   pipe(p1Pipe2);

   if ((pid = fork()) < 0) {   /* error with fork */
      perror(NULL);
      exit(EXIT_FAILURE);
   }
   else if (pid == 0) {   /* child 1 */
      childLogic(p1Pipe1, p1Pipe2, readFd1, writeFd1, argv, indFirstPlayer);
   }
   else {   /* parent during first child */
      /* close un-needed pipes */
      close(p1Pipe1[0]);
      close(p1Pipe2[1]);

      /* create pipes for second child; remember to close first player pipes */
      pipe(p2Pipe1);
      pipe(p2Pipe2);

      if ((pid = fork()) < 0) {
         perror(NULL);
         exit(EXIT_FAILURE);
      }
      else if (pid == 0) {   /* child 2 */
         childLogic(p2Pipe1, p2Pipe2, readFd2, writeFd2, argv, indSecondPlayer);
      }
      else {   /* parent during second child */
         close(p2Pipe1[0]);
         close(p2Pipe2[1]);
      }

      runBattleshipMatch(gFlag, dFlag, p1Pipe1, p1Pipe2, p2Pipe1, p2Pipe2,
         player1Board, player2Board, firstPlayerName, secondPlayerName);
   }
   return 0;
}