/**
 *  \file semSharedMemCoach.c (implementation file)
 *
 *  \brief Problem name: Game of the rope.
 *
 *  \brief Concept: Pedro Mariano
 *
 *  Synchronization based on semaphores and shared memory.
 *  Implementation with SVIPC.
 *
 *  Definition of the operations carried out by the coaches:
 *     \li reviewNotes
 *     \li callContestants
 *     \li informReferee
 *     \li endOperCoach.
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

/** \brief review notes operation operation */
static void reviewNotes (unsigned int coachId);

/** \brief call contestants operation */
static void callContestants(unsigned int coachId);

/** \brief inform referee operation */
static void informReferee (unsigned int coachId);

/** \brief end of operations of the coach operation */
static bool endOperCoach (unsigned int coachId);

/** \brief select contestants (internal) operation */
static void selectContestants (unsigned int coachId, unsigned int select[]);

/** \brief coach greeting (internal) operation */
static void coachGreeting (unsigned int coachId);

/**
 *  \brief Main program.
 *
 *  Its role is to generate the life cycle of one of intervening entities in the problem: the coach.
 */

int main (int argc, char *argv[])
{
  int key;                                                           /*access key to shared memory and semaphore set */
  char *tinp;                                                                      /* numerical parameters test flag */
  unsigned int c;                                                                            /* coach identification */

  /* validation of command line parameters */

  if (argc != 5)
     { freopen ("error_GCH", "a", stderr);
       fprintf (stderr, "Number of parameters is incorrect!\n");
       return EXIT_FAILURE;
     }
     else freopen (argv[4], "w", stderr);
  c = (unsigned int) strtol (argv[1], &tinp, 0);
  if ((*tinp != '\0') || (c >= C))
     { fprintf (stderr, "Coach process identification is wrong!\n");
       return EXIT_FAILURE;
     }
  strcpy (nFic, argv[2]);
  key = (unsigned int) strtol (argv[3], &tinp, 0);
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

  /* simulation of the life cycle of the passenger */

  coachGreeting (c);
  do
  { reviewNotes (c);                                                                  /* the coach reviews his notes */
    callContestants (c);                                                   /* the coach calls contestants to a trial */
    informReferee (c);                                            /* the coach informs the referee the team is ready */
  } while (!endOperCoach (c));

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
static void coachGreeting (unsigned int coachId)
{
  fprintf(stdout, "\e[32;1mI'm coach #%u\e[0m\n", coachId);
  fflush(stdout);
}

/**
 *  \brief Review notes operation.
 *
 *  The coach review his notes before a trial and waits for a call from the referee to a new trial.
 *  The internal state should be saved.
 *
 *  \param coachId coach id
 */

static void reviewNotes (unsigned int coachId)
{
	if (semDown (semgid, sh->access) == -1)                                                   /* enter critical region */
	{
		perror ("error on the down operation for semaphore access (CH)");
		exit (EXIT_FAILURE);
	}

	sh->fSt.st.coachStat[coachId] = WAIT_FOR_REFEREE_COMMAND;
	saveState (nFic, &(sh->fSt));

	if (semUp (semgid, sh->access) == -1)                                                      /* exit critical region */
	{
		perror ("error on the up operation for semaphore access (CH)");
		exit (EXIT_FAILURE);
	}

	if (semDown (semgid, sh->waitForNotice[coachId]) == -1)                                    /* sleeps coach */
	{
		perror ("error on the down operation for semaphore array waitForNotice (CH)");
		exit (EXIT_FAILURE);
	}
}

/**
 *  \brief Call contestants operation.
 *
 *  The coach updates the contestants strengths, selects some of them to form the team according to a predefined strategy,
 *  calls them to stand at the end of the rope and waits for all of them to be in position.
 *  The internal state should be saved.
 *
 *  \param coachId coach id
 */

static void callContestants(unsigned int coachId)
{
	if (semDown (semgid, sh->access) == -1)                                                   /* enter critical region */
	{
		perror ("error on the down operation for semaphore access (CH)");
		exit (EXIT_FAILURE);
	}

	unsigned int select[M];
	sh->fSt.st.coachStat[coachId] = ASSEMBLE_TEAM;
	selectContestants(coachId, select);
	saveState (nFic, &(sh->fSt));
	int m;
	for (m = 0; m < M; m++)
	{
		if (semUp (semgid, sh->waitForCommand[coachId][select[m]]) == -1)						/* wakes contestants */
		{
			perror ("error on the up operation for semaphore array waitForCommand (CH)");
			exit (EXIT_FAILURE);
		}
	}

	if (semUp (semgid, sh->access) == -1)                                                      /* exit critical region */
	{
		perror ("error on the up operation for semaphore access (CH)");
		exit (EXIT_FAILURE);
	}

	if (semDown (semgid, sh->waitForNotice[coachId]) == -1)                                    /* sleeps coach */
	{
		perror ("error on the down operation for semaphore array waitForNotice (CH)");
		exit (EXIT_FAILURE);
	}
}

/**
 *  \brief Inform referee operation.
 *
 *  The coach of the last team to become ready informs the referee.
 *  The coach waits for the trial to take place.
 *  The internal state should be saved.
 *
 *  \param coachId coach id
 */

static void informReferee (unsigned int coachId)
{
	if (semDown (semgid, sh->access) == -1)                                                   /* enter critical region */
	{
		perror ("error on the down operation for semaphore access (CH)");
		exit (EXIT_FAILURE);
	}

	sh->fSt.st.coachStat[coachId] = WATCH_TRIAL;
	saveState (nFic, &(sh->fSt));
	sh->nCoaches += 1;
	if (sh->nCoaches == C)
	{
		if (semUp (semgid, sh->proceed) == -1)													/* wakes refree */
		{
			perror ("error on the up operation for semaphore proceed (CH)");
			exit (EXIT_FAILURE);
		}
		sh->nCoaches = 0;
	}

	if (semUp (semgid, sh->access) == -1)														/* exit critical region */
	{
		perror ("error on the up operation for semaphore access (CH)");
		exit (EXIT_FAILURE);
	}

	if (semDown (semgid, sh->waitForNotice[coachId]) == -1)										/* sleeps coach */
	{
		perror ("error on the down operation for semaphore array waitForNotice (CH)");
		exit (EXIT_FAILURE);
	}
}

/**
 *  \brief Select contestants (internal operation).
 *
 *  The contestants' strengths are updated prior to the selection if it is not the first time the operation is called.
 *  Two strategies are contemplated:
 *      coach of team 0 always selects the M strongest contestants
 *      coach of team 1 only selects the M strongest contestants if he has not won the last trial, otherwise he keeps
 *       the team.
 *
 *  \param coachId coach id
 *  \param select selection ids array
 */

static void selectContestants (unsigned int coachId, unsigned int select[])
{
  unsigned int id[N];                                                                        /* contestant ids array */
  unsigned int tmp;                                                                            /* temporary variable */
  unsigned int n, m;                                                                           /* counting variables */
  bool inTrial[N];                                                                            /* in trial flag array */

  if ((sh->fSt.nGame != 0) || (sh->fSt.game[sh->fSt.nGame].nTrial != 0))            /* update contestants' strengths */
     { for (n = 0; n < N; n++)
         inTrial[n] = false;
       if (sh->fSt.game[sh->fSt.nGame].nTrial != 0)
          for (m = 0; m < M; m++)
            inTrial[sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial-1].id[coachId][m]] = true;
          else for (m = 0; m < M; m++)
                 inTrial[sh->fSt.game[sh->fSt.nGame-1].trial[sh->fSt.game[sh->fSt.nGame-1].nTrial].id[coachId][m]] = true;
       for (n = 0; n < N; n++)
         if (inTrial[n])
            { if (sh->fSt.st.contStat[coachId][n].strength != 0)
            	sh->fSt.st.contStat[coachId][n].strength -= 1;
            }
            else sh->fSt.st.contStat[coachId][n].strength += 1;
     }

  for (n = 0; n < N; n++)
    id[n] = n;
  if ((coachId == 0) || ((sh->fSt.nGame == 0) && (sh->fSt.game[sh->fSt.nGame].nTrial == 0)) ||
      ((sh->fSt.nGame != 0) && (sh->fSt.game[sh->fSt.nGame].nTrial == 0) && (sh->fSt.game[sh->fSt.nGame-1].pos <= 0)) ||
      ((sh->fSt.game[sh->fSt.nGame].nTrial != 0) && (sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].pos <= 0)))
     { for (m = 0; m < M; m++)
       for (n = m+1; n < N; n++)
         if (sh->fSt.st.contStat[coachId][id[m]].strength <= sh->fSt.st.contStat[coachId][id[n]].strength)
            { tmp = id[m];
              id[m] = id[n];
              id[n] = tmp;
            }
       for (m = 0; m < M; m++)
         select[m] = id[m];
     }
     else if ((sh->fSt.nGame != 0) && (sh->fSt.game[sh->fSt.nGame].nTrial == 0))
             for (m = 0; m < M; m++)
               select[m] = sh->fSt.game[sh->fSt.nGame-1].trial[sh->fSt.game[sh->fSt.nGame-1].nTrial].id[coachId][m];
             else for (m = 0; m < M; m++)
                    select[m] = sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial-1].id[coachId][m];
}

/**
 *  \brief End of operations of the coach.
 *
 *  The coach asserts if the end of operations has arrived.
 *
 *  \param coachId coach id
 *
 *  \return -c false, if it is not the end of operations
 *  \return -c true, otherwise
 */

static bool endOperCoach (unsigned int coachId)
{
  if (semDown (semgid, sh->access) == -1)                                                   /* enter critical region */
     { perror ("error on the down operation for semaphore access (CH)");
       exit (EXIT_FAILURE);
     }

  bool endOp;                                                                              /* end of operations flag */
  bool alert[N];                                                                        /* wake up contestants array */
  unsigned int n, m;                                                                           /* counting variables */

  endOp = sh->fSt.end;                                                                 /* get end of operations flag */
  if (endOp)
     { sh->fSt.st.coachStat[coachId] = WAIT_FOR_REFEREE_COMMAND;                                     /* state change */
       for (n = 0; n < N; n++)
         alert[n] = true;
       for (m = 0; m < M; m++)
         alert[sh->fSt.game[sh->fSt.nGame].trial[sh->fSt.game[sh->fSt.nGame].nTrial].id[coachId][m]] = false;
       for (n = 0; n < N; n++)
         if (alert[n])
            { sh->fSt.st.contStat[coachId][n].strength += 1;                                      /* update strength */
              if (semUp (semgid, sh->waitForCommand[coachId][n]) == -1)            /* wake up non active contestants */
                 { perror ("error on the up operation for semaphore array waitForCommand (CH)");
                   exit (EXIT_FAILURE);
                 }
            }
            else if (sh->fSt.st.contStat[coachId][n].strength != 0)
                    sh->fSt.st.contStat[coachId][n].strength -= 1;                                /* update strength */
       saveState (nFic, &(sh->fSt));                                           /* save present state in the log file */
     }

  if (semUp (semgid, sh->access) == -1)                                                      /* exit critical region */
     { perror ("error on the up operation for semaphore access (CH)");
       exit (EXIT_FAILURE);
     }

  return endOp;
}
