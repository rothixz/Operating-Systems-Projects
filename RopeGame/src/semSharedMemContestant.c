/**
 *  \file semSharedMemContestant.c (implementation file)
 *
 *  \brief Problem name: Game of the rope.
 *
 *  \brief Concept: Pedro Mariano
 *
 *  Synchronization based on semaphores and shared memory.
 *  Implementation with SVIPC.
 *
 *  Definition of the operations carried out by the contestants:
 *     \li seatDown
 *     \li followCoachAdvice
 *     \li getReady
 *     \li amDone
 *     \li endOperContestant.
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

#include "probConst.h"
#include "probDataStruct.h"
#include "logging.h"
#include "sharedDataSync.h"
#include "semaphore.h"
#include "sharedMemory.h"

/** \brief logging file name */
static char nFic[51];

/** \brief shared memory block access identifier */
static int shmid;

/** \brief semaphore set access identifier */
static int semgid;

/** \brief pointer to shared memory region */
static SHARED_DATA *sh;

/** \brief seat down operation */
static bool seatDown (unsigned int coachId, unsigned int contId);

/** \brief follow coach advice operation */
static void followCoachAdvice (unsigned int coachId, unsigned int contId);

/** \brief get ready operation */
static void getReady (unsigned int coachId, unsigned int contId);

/** \brief am done operation */
static void amDone (unsigned int coachId, unsigned int contId);

/** \brief end of operations of the contestant operation */
static bool endOperContestant (unsigned int coachId, unsigned int contId);

/** \brief pull the rope operation */
static void pullTheRope (void);

/** \brief contestant greeting (internal) operation */
static void contestantGreeting (unsigned int coachId, unsigned int contId);

/**
 *  \brief Main program.
 *
 *  Its role is to generate the life cycle of one of intervening entities in the problem: the contestant.
 */

int main (int argc, char *argv[])
{
  int key;                                                           /*access key to shared memory and semaphore set */
  char *tinp;                                                                      /* numerical parameters test flag */
  unsigned int c;                                                                            /* coach identification */
  unsigned int n;                                                                       /* contestant identification */

  /* validation of command line parameters */

  if (argc != 6)
     { freopen ("error_GCT", "a", stderr);
       fprintf (stderr, "Number of parameters is incorrect!\n");
       return EXIT_FAILURE;
     }
     else freopen (argv[5], "w", stderr);
  c = (unsigned int) strtol (argv[1], &tinp, 0);
  if ((*tinp != '\0') || (c >= C))
     { fprintf (stderr, "Coach process identification is wrong!\n");
       return EXIT_FAILURE;
     }
  n = (unsigned int) strtol (argv[2], &tinp, 0);
  if ((*tinp != '\0') || (n >= N))
     { fprintf (stderr, "Contestant process identification is wrong!\n");
       return EXIT_FAILURE;
     }
  strcpy (nFic, argv[3]);
  key = (unsigned int) strtol (argv[4], &tinp, 0);
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

  /* simulation of the life cycle of the contestant */

  contestantGreeting (c,n);
  do
  { if (seatDown (c,n))                                     /* the contestant goes to the bench to rest a little bit */
       break;
    followCoachAdvice (c,n);                                            /* the contestant complies to coach decision */
    getReady (c,n);                                             /* the contestant takes place at his end of the rope */
    pullTheRope ();                                       /* the contestant pulls the rope along with his companions */
    amDone (c,n);                                                                  /* the contestant ends his effort */
  } while (!endOperContestant (c,n));

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
static void contestantGreeting (unsigned int coachId, unsigned int contId)
{
  fprintf(stdout, "\e[32;1mI'm costestant #%u-%u\e[0m\n", coachId, contId);
  fflush(stdout);
}

/**
 *  \brief Seat down operation.
 *
 *  The contestant seats at the bench and waits to be called by the coach.
 *  The internal state should be saved.
 *
 *  \param coachId coach id
 *  \param contId contestant id
 *
 *  \return -c false, if it is not the end of operations
 *  \return -c true, otherwise
 */

static bool seatDown (unsigned int coachId, unsigned int contId)
{
	if (semDown (semgid, sh->access) == -1)										/* enter critical region */
	{
		perror ("error on the down operation for semaphore access (CT)");
		exit (EXIT_FAILURE);
	}
	
	sh->fSt.st.contStat[coachId][contId].stat = SEAT_AT_THE_BENCH;				/* State = SEAT_AT_THE_BENCH */
	saveState (nFic, &(sh->fSt));												/* Save state */

	if (semUp (semgid, sh->access) == -1)										/* exit critical region */
	{
		perror ("error on the up operation for semaphore access (CT)");
		exit (EXIT_FAILURE);
	}

	if (semDown (semgid, sh->waitForCommand[coachId][contId]) == -1)			/* Put contestant to sleep */
	{
		perror ("error on the down operation for semaphore array WaitForCommand (CT)");
		exit (EXIT_FAILURE);
	}

	if (semDown (semgid, sh->access) == -1)										/* enter critical region */
	{
		perror ("error on the down operation for semaphore access (CT)");
		exit (EXIT_FAILURE);
	}

	bool endOP = sh->fSt.end;

	if (semUp (semgid, sh->access) == -1)										/* exit critical region */
	{
		perror ("error on the up operation for semaphore access (CT)");
		exit (EXIT_FAILURE);
	}

	return endOP;
}

/**
 *  \brief Follow coach advice operation.
 *
 *  The contestant join the trial team if requested by the coach and waits for the referee's command to start pulling.
 *  The last contestant to join his end of the rope should alert the coach.
 *  The internal state should be saved.
 *
 *
 *  \param coachId coach id
 *  \param contId contestant id
 */

static void followCoachAdvice (unsigned int coachId, unsigned int contId)
{
	if (semDown (semgid, sh->access) == -1)																					/* enter critical region */
	{
		perror ("error on the down operation for semaphore access (CT)");
		exit (EXIT_FAILURE);
	}

	sh->fSt.st.contStat[coachId][contId].stat = STAND_IN_POSITION;															/* state = STAND_IN_POSITION */
	sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].id[coachId][sh->nContInPosition[coachId]] = contId;
	sh->nContInPosition[coachId]+=1;
	saveState (nFic, &(sh->fSt));																							/* Save state */
	if (sh->nContInPosition[coachId] == M)
		{
			if (semUp (semgid, sh->waitForNotice[coachId]) == -1)																/* wake coach */
			{
				perror ("error on the up operation for semaphore array waitForNotice (CT)");
				exit (EXIT_FAILURE);
			}
			sh->nContInPosition[coachId] = 0;
		}

	if (semUp (semgid, sh->access) == -1)																					/* exit critical region */
	{
		perror ("error on the up operation for semaphore access (CT)");
		exit (EXIT_FAILURE);
	}

	if (semDown (semgid, sh->waitForCommand[coachId][contId]) == -1)														/* Put contestant to sleep */
	{
		perror ("error on the down operation for semaphore array WaitForCommand (CT)");
		exit (EXIT_FAILURE);
	}
}

/**
 *  \brief Get ready operation.
 *
 *  The contestant gets ready to start pulling the rope.
 *  The internal state should be saved.
 *
 *  \param coachId coach id
 *  \param contId contestant id
 */

static void getReady (unsigned int coachId, unsigned int contId)
{
	if (semDown (semgid, sh->access) == -1)																					/* enter critical region */
	{
		perror ("error on the down operation for semaphore access (CT)");
		exit (EXIT_FAILURE);
	}

	sh->fSt.st.contStat[coachId][contId].stat = DO_YOUR_BEST;																/* State = DO_YOUR_BEST */
	saveState (nFic, &(sh->fSt));																							/* Save state */

	if (semUp (semgid, sh->access) == -1)																					/* exit critical region */
	{
		perror ("error on the up operation for semaphore access (CT)");
		exit (EXIT_FAILURE);
	}
}

/**
 *  \brief Am done operation.
 *
 *  The contestant ends his pulling effort, informs the referee and waits for the referee decision to return to the bench.
 *  The internal state should not be saved.
 *
 *  \param coachId coach id
 *  \param contId contestant id
 */

static void amDone (unsigned int coachId, unsigned int contId)
{
	if (semDown (semgid, sh->access) == -1)																					/* enter critical region */
	{
		perror ("error on the down operation for semaphore access (CT)");
		exit (EXIT_FAILURE);
	}

	sh->nContestants += 1;
	if (sh->nContestants == C*M)
	{
		if (semUp (semgid, sh->proceed) == -1)																				/* Wake refree */
		{
			perror ("error on the up operation for semaphore proceed (CT)");
			exit (EXIT_FAILURE);
		}
		sh->nContestants = 0;
	}

	if (semUp (semgid, sh->access) == -1)																					/* exit critical region */
	{
		perror ("error on the up operation for semaphore access (CT)");
		exit (EXIT_FAILURE);
	}

	if (semDown (semgid, sh->waitForCommand[coachId][contId]) == -1)														/* Put Contestant to sleep */
	{
		perror ("error on the down operation for semaphore array WaitForCommand (CT)");
		exit (EXIT_FAILURE);
	}
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

/**
 *  \brief End of operations of the contestant.
 *
 *  The contestant asserts if the end of operations has arrived.
 *
 *  \param coachId coach id
 *  \param contId contestant id
 *
 *  \return -c false, if it is not the end of operations
 *  \return -c true, otherwise
 */

static bool endOperContestant (unsigned int coachId, unsigned int contId)
{
  if (semDown (semgid, sh->access) == -1)                                                   /* enter critical region */
     { perror ("error on the down operation for semaphore access (CT)");
       exit (EXIT_FAILURE);
     }

  bool endOp;                                                                              /* end of operations flag */

  endOp = sh->fSt.end;                                                                 /* get end of operations flag */
  if (endOp)
     { sh->fSt.st.contStat[coachId][contId].stat = SEAT_AT_THE_BENCH;                                /* state change */
       saveState (nFic, &(sh->fSt));                                           /* save present state in the log file */
     }

  if (semUp (semgid, sh->access) == -1)                                                      /* exit critical region */
     { perror ("error on the up operation for semaphore access (CT)");
       exit (EXIT_FAILURE);
     }

  return endOp;
}
