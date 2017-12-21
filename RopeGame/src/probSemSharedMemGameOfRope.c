/**
 *  \file probSemSharedMemGameOfRope.c (implementation file)
 *
 *  \brief Problem name: Game of the rope.
 *
 *  \brief Concept: Pedro Mariano
 *
 *  Synchronization based on semaphores and shared memory.
 *  Implementation with SVIPC.
 *
 *  Generator process of the intervening entities.
 *
 *  Upon execution, one parameter is requested:
 *    \li name of the logging file.
 *
 *  \author António Rui Borges - November 2015
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <math.h>

#include "probConst.h"
#include "probDataStruct.h"
#include "logging.h"
#include "sharedDataSync.h"
#include "semaphore.h"
#include "sharedMemory.h"

/** \brief name of referee process */
#define   REFEREE         "./referee"

/** \brief name of coach process */
#define   COACH           "./coach"

/** \brief name of contestant process */
#define   CONTESTANT      "./contestant"

/**
 *  \brief Main program.
 *
 *  Its role is starting the simulation by generating the intervening entities processes (referee, coaches and contestants)
 *  and waiting for their termination.
 */

int main (int argc, char *argv[])
{
  char nFic[51];                                                                              /*name of logging file */
  char nFicErr[] = "error      ";                                                        /* base name of error files */
  FILE *fic;                                                                                      /* file descriptor */
  int shmid,                                                                      /* shared memory access identifier */
      semgid;                                                                     /* semaphore set access identifier */
  int t;                                                                               /* keyboard reading test flag */
  char opt;                                                                                                /* answer */
  unsigned int g, c, m, n;                                                                     /* counting variables */
  SHARED_DATA *sh;                                                                /* pointer to shared memory region */
  int pidRF,                                                                      /* entrepreneur process identifier */
      pidCH[C],                                                                  /* coach processes identifier array */
      pidCT[C][N];                                                          /* contestant processes identifier array */
  int key;                                                           /*access key to shared memory and semaphore set */
  char num[3][12];                                                     /* numeric value conversion (up to 10 digits) */
  int status,                                                                                    /* execution status */
      info;                                                                                               /* info id */
  bool term;                                                                             /* process termination flag */

  /* getting log file name */

  do
  { do
    { printf ("\nLog file name? ");
      t = scanf ("%20[^\n]", nFic);
      scanf ("%*[^\n]");
      scanf ("%*c");
    } while (t == 0);
    fic = fopen (nFic, "r");
    if (fic != NULL)
       { fclose (fic);
         printf ("There is already a file with this name! ");
         do
         { printf ("Overwrite? ");
           scanf ("%c", &opt);
           if (opt != '\n')
              { scanf ("%*[^\n]");
                scanf ("%*c");
              }
         } while ((opt == '\n') || ((opt != 'Y') && (opt != 'y') &&
                                    (opt != 'N') && (opt != 'n')));
         if ((opt == 'Y') || (opt == 'y')) break;
       }
  } while (fic != NULL);

  /* composing command line */

  for (c = 0; c < C; c++)
    sprintf (num[c], "%s", "0");
  if ((key = ftok (".", 'a')) == -1)
     { perror ("error on generating the key");
       exit (EXIT_FAILURE);
     }
  sprintf (num[2], "%d", key);

  /* creating and initializing the shared memory region and the log file */

  if ((shmid = shmemCreate (key, sizeof (SHARED_DATA))) == -1)
     { perror ("error on creating the shared memory region");
       exit (EXIT_FAILURE);
     }
  if (shmemAttach (shmid, (void **) &sh) == -1)
     { perror ("error on mapping the shared region on the process address space");
       exit (EXIT_FAILURE);
     }

  srandom ((unsigned int) getpid ());                                                 /* initialize random generator */


    /* initialize problem internal status */

  sh->fSt.st.refereeStat = START_OF_THE_MATCH;                                  /* the referee is starting the match */
  for (c = 0; c < C; c++)
    sh->fSt.st.coachStat[c] = WAIT_FOR_REFEREE_COMMAND;              /* the coach is waiting for the referee command */
  for (c = 0; c < C; c++)
  for (n = 0; n < N; n++)
  { sh->fSt.st.contStat[c][n].stat = SEAT_AT_THE_BENCH;                    /* the contestant is seating at the bench */
    sh->fSt.st.contStat[c][n].strength = (unsigned int) floor (15.0*random ()/RAND_MAX+5.0);     /* set his strength */                                           /* no pieces were produced so far */
  }
  for (g = 0; g < G; g++)
  { for (t = 0; t < T; t++)
    { sh->fSt.game[g].trial[t].pos = 0;                                   /* reset trial middle position of the rope */
      for (c = 0; c < C; c++)
      for (m = 0; m < M; m++)
        sh->fSt.game[g].trial[t].id[c][m] = (unsigned int) -1;                       /* nobody has been selected yet */
    }
    sh->fSt.game[g].pos = 0;                                               /* reset game middle position of the rope */
    sh->fSt.game[g].nTrial = (unsigned int) -1;                                          /* no trial has started yet */
  }
  sh->fSt.nGame = (unsigned int) -1;                                                      /* no game has started yet */
  sh->fSt.end = false;                                                         /* the simulation has not started yet */

  sh->nCoaches = 0;                                                 /* no coach has generated any inform request yet */
  sh->nContestants = 0;                                                     /* no contestant has pulled the rope yet */
  for (c = 0; c < C; c++)
    sh->nContInPosition[c] = 0;                           /* no contestant is in position at his end of the rope yet */
  sh->fSt.alreadyRun = false;                                                         /* logging file initialization */

    /* initialize problem internal status */

  createLog (nFic);                                                                             /* log file creation */
  saveMatchHeader (nFic, &(sh->fSt));                                                           /* save match header */

    /* initialize semaphore ids */

  sh->access = ACCESS;                                                              /* mutual exclusion semaphore id */
  sh->proceed = PROCEED;                                                             /* referee proceed semaphore id */
  for (c = 0; c < C; c++)
    sh->waitForNotice[c] = B_WAITFORNOTICE + c;                    /* coaches waiting for notice semaphore ids array */
  for (c = 0; c < C; c++)
  for (n = 0; n < N; n++)
    sh->waitForCommand[c][n] = B_WAITFORCOMMAND + c*N + n;   /* contestants waiting for command semaphores ids array */

    /* creating and initializing the semaphore set */

  if ((semgid = semCreate (key, SEM_NU)) == -1)
     { perror ("error on creating the semaphore set");
       exit (EXIT_FAILURE);
     }
  if (semUp (semgid, sh->access) == -1)                                        /* enabling access to critical region */
     { perror ("error on executing the up operation for semaphore access");
       exit (EXIT_FAILURE);
     }

  /* generation of intervening entities processes */

  strcpy (nFicErr + 6, "CH");
  for (c = 0; c < C; c++)                                                                         /* coach processes */
  { if ((pidCH[c] = fork ()) < 0)
       { perror ("error on the fork operation for the coach");
         exit (EXIT_FAILURE);
       }
    num[0][0] = '0' + c;
    nFicErr[8] = '0' + c;
    if (pidCH[c] == 0)
       if (execl (COACH, COACH, num[0], nFic, num[2], nFicErr, NULL) < 0)
          { perror ("error on the generation of the coach process");
            exit (EXIT_FAILURE);
          }
  }
  strcpy (nFicErr + 6, "CT");
  for (c = 0; c < C; c++)
  for (n = 0; n < N; n++)
  { if ((pidCT[c][n] = fork ()) < 0)                                                         /* contestant processes */
       { perror ("error on the fork operation for the contestant");
         exit (EXIT_FAILURE);
       }
    num[0][0] = '0' + c;
    num[1][0] = '0' + n;
    nFicErr[8] = '0' + c;
    nFicErr[10] = '0' + n;
    if (pidCT[c][n] == 0)
       if (execl (CONTESTANT, CONTESTANT, num[0], num[1], nFic, num[2], nFicErr, NULL) < 0)
          { perror ("error on the generation of the contestant process");
            exit (EXIT_FAILURE);
          }
  }
  strcpy (nFicErr + 6, "RF");
  if ((pidRF = fork ()) < 0)                                                                      /* referee process */
     { perror ("error on the fork operation for the referee");
       exit (EXIT_FAILURE);
     }
  if (pidRF == 0)
     if (execl (REFEREE, REFEREE, nFic, num[2], nFicErr, NULL) < 0)
          { perror ("error on the generation of the referee process");
            exit (EXIT_FAILURE);
          }

  /* signaling start of operations */

  if (semSignal (semgid) == -1)
     { perror ("error on signaling start of operations");
       exit (EXIT_FAILURE);
     }

  /* waiting for the termination of the intervening entities processes */

  printf ("\nFinal report\n");
  m = 0;
  do
  { info = wait (&status);
    term = false;
    for (n = 0; n < C*(N+1)+1; n++)
      if ((n == 0) && (info == pidRF))
         { term = true;
           break;
         }
	 else if ((n > 0) && (n <= C) && (info == pidCH[n-1]))
                 { term = true;
                   break;
                 }
		     else if ((n > C) && (n <= C*(N+1)) && (info == pidCT[(n-C-1)/N][(n-C-1)%N]))
                         { term = true;
                           break;
                         }
    if (!term)
       { perror ("error on waiting for an intervening process");
         exit (EXIT_FAILURE);
       }
    if (n == 0)
       printf ("the referee process has terminated: ");
       else if (n <= C)
	       printf ("the coach process, with id %u, has terminated: ", n-1);
               else printf ("the contestant process, with id %u-%u, has terminated: ", (n-C-1)/N, (n-C-1)%N);
    if (WIFEXITED (status))
       printf ("its status was %d\n", WEXITSTATUS (status));
    m += 1;
  } while (m < C*(N+1)+1);

  /* destruction of semaphore set and shared region */

  if (semDestroy (semgid) == -1)
     { perror ("error on destructing the semaphore set");
       exit (EXIT_FAILURE);
     }
  if (shmemDettach (sh) == -1)
     { perror ("error on unmapping the shared region off the process address space");
       exit (EXIT_FAILURE);
     }
  if (shmemDestroy (shmid) == -1)
     { perror ("error on destructing the shared region");
       exit (EXIT_FAILURE);
     }

  return EXIT_SUCCESS;
}
