/**
 *  \file semSharedMemReferee.c (implementation file)
 *
 *  \brief Problem name: Game of the rope.
 *
 *  \brief Concept: Pedro Mariano
 *
 *  Synchronization based on semaphores and shared memory.
 *  Implementation with SVIPC.
 *
 *  Definition of the operations carried out by the referee:
 *     \li announceNewGame
 *     \li callTrial
 *     \li startTrial
 *     \li assertTrialDecision
 *     \li declareGameWinner
 *     \li declareMatchWinner.
 *
 *  \author Miguel Ferreira nº72583  
 *  \author Antonio Mota nº72622 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

#include "probConst.h"
#include "probDataStruct.h"
#include "logging.h"
#include "sharedDataSync.h"
#include "semaphore.h"
#include "sharedMemory.h"

/** \brief game continuation flag */
#define CONT        'C'

/** \brief logging file name */
static char nFic[51];

/** \brief shared memory block access identifier */
static int shmid;

/** \brief semaphore set access identifier */
static int semgid;

/** \brief pointer to shared memory region */
static SHARED_DATA *sh;

/** \brief announce new game operation operation */
static void announceNewGame (unsigned int g);

/** \brief call trial operation */
static void callTrial (unsigned int t);

/** \brief start trial terminal */
static void startTrial ();

/** \brief assert trial decision operation */
static char assertTrialDecision ();

/** \brief declare game winner operation */
static void declareGameWinner (char decision);

/** \brief declare match winner operation */
static void declareMatchWinner ();

/** \brief referee greeting (internal) operation */
static void refereeGreeting ();

/**
 *  \brief Main program.
 *
 *  Its role is to generate the life cycle of one of intervening entities in the problem: the referee.
 */

int main (int argc, char *argv[])
{
  int key;                                                           /*access key to shared memory and semaphore set */
  char *tinp;                                                                      /* numerical parameters test flag */

  /* validation of command line parameters */

  if (argc != 4)
     { freopen ("error_GRF", "a", stderr);
       fprintf (stderr, "Number of parameters is incorrect!\n");
       return EXIT_FAILURE;
     }
     else freopen (argv[3], "w", stderr);
  strcpy (nFic, argv[1]);
  key = (unsigned int) strtol (argv[2], &tinp, 0);
  if (*tinp != '\0')
     { fprintf (stderr, "Error on the access key communication!\n");
       return EXIT_FAILURE;
     }

  /* connection to the semaphore set and the shared memory region and mapping the shared region onto the
     process address space */

  if ((semgid = semConnect (key)) == -1)
     { perror ("error on connecting to the semaphore set");
       return EXIT_FAILURE;
     }
  if ((shmid = shmemConnect (key)) == -1)
     { perror ("error on connecting to the shared memory region");
       return EXIT_FAILURE;
     }
  if (shmemAttach (shmid, (void **) &sh) == -1)
     { perror ("error on mapping the shared region on the process address space");
       return EXIT_FAILURE;
     }

  /* simulation of the life cycle of the referee */

  usleep((unsigned int) floor (200000.0 * random () / RAND_MAX + 1.5));

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

  /* unmapping the shared region off the process address space */

  if (shmemDettach (sh) == -1)
     { perror ("error on unmapping the shared region off the process address space");
       return EXIT_FAILURE;;
     }

  return EXIT_SUCCESS;
}

/**
 *  \brief Greeting the run
 */
static void refereeGreeting ()
{
  fprintf(stdout, "\e[32;1mI'm the referee\e[0m\n");
  fflush(stdout);
}

/**
 *  \brief Announce new game operation.
 *
 *  The referee starts a game.
 *  The game number should be updated.
 *  Both game header and internal state should be saved.
 *
 *  \param g game number
 */

static void announceNewGame (unsigned int g)
{
	if (semDown (semgid, sh->access) == -1)														/* Enter critical region */
	{
		perror ("error on the down operation for semaphore access (RF)");
		exit (EXIT_FAILURE);
	}

	sh->fSt.st.refereeStat = START_OF_A_GAME;													/* State = START_OF_A_GAME */
	sh->fSt.nGame = g;																			/* Updates game number */
	saveGameHeader (nFic, &(sh->fSt));															/* Save game header */
	saveState (nFic, &(sh->fSt));																/* Save state */

	if (semUp (semgid, sh->access) == -1)														/* Exit critical region */
	{
		perror ("error on the up operation for semaphore access (RF)");
		exit (EXIT_FAILURE);
	}
}

/**
 *  \brief Call trial operation.
 *
 *  The referee calls the coaches to assemble the teams for a trial and waits for the teams to be ready.
 *  The trial number and the trial initial position should be updated.
 *  The internal state should be saved.
 *
 *  \param t trial number
 */

static void callTrial (unsigned int t)
{
	if (semDown (semgid, sh->access) == -1)																				/* Enter critical region */
	{
		perror ("error on the down operation for semaphore access (RF)");
		exit (EXIT_FAILURE);
	}

	sh->fSt.st.refereeStat = TEAMS_READY;			 																	/* State = TEAMS_READY */
	sh->fSt.game[sh->fSt.nGame].nTrial = t;																				/* Updates trial number */
	if (t == 0)
	{
		sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].pos = 0;	
	}
	else{
		sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].pos = 
		          sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial-1].pos;		/* Initial pos = Final pos */
	}
	saveState (nFic, &(sh->fSt));																					/* Save state */
	int c;
	for (c = 0; c < C; c++)
	{
		if (semUp (semgid, sh->waitForNotice[c]) == -1)																	/* Wakes coaches */
		{
			perror ("error on the up operation for semaphore array waitForNotice (RF)");
			exit (EXIT_FAILURE);
		}
	}

	if (semUp (semgid, sh->access) == -1)																				/* Exit critical region */
	{
		perror ("error on the up operation for semaphore access (RF)");
		exit (EXIT_FAILURE);
	}

	if (semDown (semgid, sh->proceed) == -1)																			/* Sleeps */
	{
		perror ("error on the down operation for semaphore proceed (RF)");
		exit (EXIT_FAILURE);
	}
}

/**
 *  \brief Start trial operation.
 *
 *  The referee starts a trial and waits for its conclusion.
 *  The contestants at the ends of the rope have to be alerted for the fact.
 *  The internal state should be saved.
 */

static void startTrial ()
{
	if (semDown (semgid, sh->access) == -1)                                                   							/* Enter critical region */
	{
		perror ("error on the down operation for semaphore access (RF)");
		exit (EXIT_FAILURE);
	}

	sh->fSt.st.refereeStat = WAIT_FOR_TRIAL_CONCLUSION;																	/* State = WAIT_FOR_TRIAL_CONCLUSION */
	saveState (nFic, &(sh->fSt));																						/* Save state */
	int m, c;
	for (c = 0; c < C; c++)
		for (m = 0; m < M; m++)
		{                                     																			/* Wakes Contestants */
			if (semUp (semgid, sh->waitForCommand[c][sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].id[c][m]]) == -1)
			{
				perror ("error on the up operation for semaphore array waitForCommand (RF)");
				exit (EXIT_FAILURE);
			}
		}

	if (semUp (semgid, sh->access) == -1)                                                      							/* Exit critical region */
	{
		perror ("error on the up operation for semaphore access (RF)");
		exit (EXIT_FAILURE);
	}

	if (semDown (semgid, sh->proceed) == -1)																			/* Sleeps */
	{
		perror ("error on the down operation for semaphore proceed (RF)");
		exit (EXIT_FAILURE);
	}
}

/**
 *  \brief Assert trial decision.
 *
 *  The referee computes and checks the trial result.
 *  Both the coaches and the contestants should be advised to return to their resting positions.
 *  The end of operations should determined.
 *  The internal state should not be saved.
 *
 *  \return -c 'C', if the game should continue
 *  \return -c 'E', if the game is over
 */

static char assertTrialDecision ()
{
	if (semDown (semgid, sh->access) == -1)                                                 										/* enter critical region */
	{
		perror ("error on the down operation for semaphore access (RF)");
		exit (EXIT_FAILURE);
	}

	int c, m, str_t1 = 0, str_t2 = 0; 
	for (c = 0; c < C; c++)
	{
		if (semUp (semgid, sh->waitForNotice[c]) == -1)																					/* Wakes coaches */
		{
			perror ("error on the up operation for semaphore array waitForNotice (RF)");
			exit (EXIT_FAILURE);
		}
		for (m = 0; m < M; m++)
		{
			if (c == 0) str_t1 += sh->fSt.st.contStat[c][sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].id[c][m]].strength;
			else str_t2 += sh->fSt.st.contStat[c][sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].id[c][m]].strength;
			if (semUp (semgid, sh->waitForCommand[c][sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].id[c][m]]) == -1)
			{
				perror ("error on the up operation for semaphore array waitForCommand (RF)");
				exit (EXIT_FAILURE);
			}
		}
	}
	if (str_t1 > str_t2) sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].pos -= 1;
	else if (str_t1 < str_t2) sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].pos += 1;

	char result;
	if( (sh->fSt.game[sh->fSt.nGame].nTrial == T-1) || (abs(sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].pos) >= S) )
	{
		if (sh->fSt.nGame == G-1)
			sh->fSt.end = true;
		result = 'E';
	}
	else result = 'C';
		
	if (semUp (semgid, sh->access) == -1)                                                      /* exit critical region */
	{
		perror ("error on the up operation for semaphore access (RF)");
		exit (EXIT_FAILURE);
	}

	return result;
}

/**
 *  \brief Declare game winner.
 *
 *  The referee announces which team has won the game.
 *  An error message should be generated if decision is not 'end of the game'.
 *  The game result should be updated.
 *  Both internal state and game result should be saved.
 *
 *  \param decision trial decision
 */

static void declareGameWinner (char decision)
{
	if(decision != 'E') 																		/* */
	{
		fprintf (stderr, "error wrong decision (RF)\n");
		exit (EXIT_FAILURE);
	}
		
	if (semDown (semgid, sh->access) == -1)                                                   /* enter critical region */
	{
		perror ("error on the down operation for semaphore access (RF)");
		exit (EXIT_FAILURE);
	}

	sh->fSt.st.refereeStat = END_OF_A_GAME;			 											/* State = END_OF_A_GAME */
	sh->fSt.game[sh->fSt.nGame].pos = sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].pos;
	saveState (nFic, &(sh->fSt));																	/* Save state */
	saveGameResult (nFic, &(sh->fSt));															/* Save Game Result */

	if (semUp (semgid, sh->access) == -1)                                                      /* exit critical region */
	{
		perror ("error on the up operation for semaphore access (RF)");
		exit (EXIT_FAILURE);
	}
}

/**
 *  \brief Declare match winner.
 *
 *  The referee announces which team has won the match.
 *  Both internal state and match result should be saved.
 */

static void declareMatchWinner ()
{
	if (semDown (semgid, sh->access) == -1)                                                   /* enter critical region */
	{
		perror ("error on the down operation for semaphore access (RF)");
		exit (EXIT_FAILURE);	
	}

	sh->fSt.st.refereeStat = END_OF_THE_MATCH;			 										/* State = END_OF_A_MATCH */
	saveState (nFic, &(sh->fSt));																/* Save state */	
	saveMatchResult (nFic, &(sh->fSt));															/* Save Match Result */
	
	if (semUp (semgid, sh->access) == -1)                                                      /* exit critical region */
	{
		perror ("error on the up operation for semaphore access (RF)");
		exit (EXIT_FAILURE);
	}
}
