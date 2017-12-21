/**
 *  \file probMonGameOfRope.c (implementation file)
 *
 *  \brief Problem name: Game of the rope.
 *
 *  \brief Concept: Pedro Mariano
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Generator thread of the intervening entities.
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
#include <pthread.h>
#include <math.h>

#include "probConst.h"
#include "probDataStruct.h"
#include "logging.h"
#include "monGameOfRopeDS.h"
#include "monGameOfRopeCH.h"
#include "monGameOfRopeCT.h"
#include "monGameOfRopeRF.h"

/* allusion to local function */

static void pullTheRope (void);

/** \brief logging file name */
char nFic[51];

/** \brief referee's thread return status */
int statusRef;

/** \brief coaches' thread return status array */
int statusCoach[C];

/** \brief contestants' thread return status array */
int statusCont[C][N];

/** \brief referee's routine */
static void *referee (void *dummy);

/** \brief coach's routine */
static void *coach (void *dIdCH);

/** \brief contestant's routine */
static void *contestant (void *dIdCT);

/** \brief contestant application defined thread id data type */
typedef struct
        { unsigned int c;                                                                                 /* team id */
          unsigned int n;                                                                                  /* own id */
        } CONT_PAR;

/**
 *  \brief Main thread.
 *
 *  Its role is starting the simulation by generating the intervening entities threads (referee, coaches and
 *  contestants) and waiting for their termination.
 */

int main (void)
{
  FILE *fic;                                                                                      /* file descriptor */
  int t;                                                                               /* keyboard reading test flag */
  char opt;                                                                                                /* answer */
  unsigned int c, n;                                                                           /* counting variables */
  pthread_t tIdR,                                                                      /* referee internal thread id */
            tIdCoach[C],                                                         /* coaches internal thread id array */
            tIdCont[C][N];                                                   /* contestants internal thread id array */
  unsigned int coa[C];                                                /* coaches application defined thread id array */
  CONT_PAR cont[C][N];                                            /* contestants application defined thread id array */
  int *status_p;                                                                      /* pointer to execution status */

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

  /* initializing coaches and contestants application defined thread id arrays and random generator */

  for (c = 0; c < C; c++)
    coa[c] = c;
  for (c = 0; c < C; c++)
  for (n = 0; n < N; n++)
  { cont[c][n].c = c;
    cont[c][n].n = n;
  }
  srandom ((unsigned int) getpid ());

  /* generation of intervening entities threads */

  for (c = 0; c < C; c++)
  for (n = 0; n < N; n++)
    if (pthread_create (&tIdCont[c][n], NULL, contestant, &cont[c][n]) != 0)                    /* thread contestant */
       { perror ("error on creating thread contestant");
         printf ("contestant\n");
         exit (EXIT_FAILURE);
       }
  for (c = 0; c < C; c++)
    if (pthread_create (&tIdCoach[c], NULL, coach, &coa[c]) != 0)                                    /* thread coach */
       { perror ("error on creating thread coach");
         printf ("coach\n");
         exit (EXIT_FAILURE);
       }
  usleep((unsigned int) floor (200000.0 * random () / RAND_MAX + 1.5));
  if (pthread_create (&tIdR, NULL, referee, NULL) != 0)                                            /* thread referee */
     { perror ("error on creating thread referee");
       printf ("referee\n");
       exit (EXIT_FAILURE);
     }

  /* waiting for the termination of the intervening entities threads */

  printf ("\nFinal report\n");
  for (c = 0; c < C; c++)
  for (n = 0; n < N; n++)
  { if (pthread_join (tIdCont[c][n], (void *) &status_p) != 0)                                  /* thread contestant */
       { perror ("error on waiting for thread contestant");
         exit (EXIT_FAILURE);
       }
    printf ("thread contestant, with id %u-%u, has terminated: ", c,n);
    printf ("its status was %d\n", *status_p);
  }
  for (c = 0; c < C; c++)
  { if (pthread_join (tIdCoach[c], (void *) &status_p) != 0)                                         /* thread coach */
       { perror ("error on waiting for thread coach");
         exit (EXIT_FAILURE);
       }
    printf ("thread coach, with id %u, has terminated: ", c);
    printf ("its status was %d\n", *status_p);
  }
  if (pthread_join (tIdR, (void *) &status_p) != 0)                                                /* thread referee */
     { perror ("error on waiting for thread referee");
       exit (EXIT_FAILURE);
     }
  printf ("thread referee has terminated: ");
  printf ("its status was %d\n", *status_p);

  exit (EXIT_SUCCESS);
}

/**
 *  \brief Function referee.
 *
 *  Its role is to simulate the life cycle of the referee.
 *
 *  \param dummy pointer to a meaningless data structure required for compatibility reasons with pthread_create
 */

static void *referee (void *dummy)
{
  unsigned int g;                                                                                     /* game number */
  unsigned int t;                                                                                    /* trial number */
  char decision;                                                                                   /* trial decision */

  refereeGreeting ();
  for (g = 0; g < G; g++)
  { announceNewGame (g);                                                             /* the referee opens a new game */
    t = 0;                                                                   /* the referee initializes trial number */
    do
    { callTrial (t);                                             /* the referee calls the contestants to a new trial */
      startTrial ();                                                            /* the referee gives pulling command */
      decision = assertTrialDecision ();                                          /* the referee checks trial result */
      t += 1;                                                                              /* increment trial number */
    } while (decision == CONT);                                                          /* is game to be continued? */
    declareGameWinner (decision);                                    /* the referee announces the winner of the game */
  }
  declareMatchWinner ();                                            /* the referee announces the winner of the match */

  statusRef = EXIT_SUCCESS;
  pthread_exit (&statusRef);
}

/**
 *  \brief Function coach.
 *
 *  Its role is to simulate the life cycle of a coach.
 *
 *  \param dIdCH pointer to application defined coach identification
 */

static void *coach (void *dIdCH)
{
  unsigned int c = *((unsigned int *) dIdCH);                                                            /* coach id */

  coachGreeting (c);
  do
  { reviewNotes (c);                                                                  /* the coach reviews his notes */
    callContestants (c);                                                   /* the coach calls contestants to a trial */
    informReferee (c);                                            /* the coach informs the referee the team is ready */
  } while (!endOperCoach (c));

  statusCoach[c] = EXIT_SUCCESS;
  pthread_exit (&statusCoach[c]);
}

/**
 *  \brief Function contestant.
 *
 *  Its role is to simulate the life cycle of a contestant.
 *
 *  \param dIdCT pointer to application defined contestant identification
 */

static void *contestant (void *dIdCT)
{
  unsigned int c = ((CONT_PAR *) dIdCT)->c,                                                               /* team id */
               n = ((CONT_PAR *) dIdCT)->n;                                                                /* own id */

  contestantGreeting (c,n);
  do
  { if (seatDown (c,n))                                     /* the contestant goes to the bench to rest a little bit */
       break;
    followCoachAdvice (c,n);                                            /* the contestant complies to coach decision */
    getReady (c,n);                                             /* the contestant takes place at his end of the rope */
    pullTheRope ();                                       /* the contestant pulls the rope along with his companions */
    amDone (c,n);                                                                  /* the contestant ends his effort */
  } while (!endOperContestant (c,n));

  statusCont[c][n] = EXIT_SUCCESS;
  pthread_exit (&statusCont[c][n]);
}

/**
 *  \brief Pull the rope operation.
 *
 *  The contestant pulls the rope for a random generated time interval (internal operation).
 */

static void pullTheRope (void)
{
  usleep((unsigned int) floor (300.0 * random () / RAND_MAX + 1.5));
}
