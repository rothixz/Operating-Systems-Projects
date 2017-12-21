/**
 *  \file monGameOfRopeCH.c (implementation file)
 *
 *  \brief Problem name: Game of the rope.
 *
 *  \brief Concept: Pedro Mariano
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Monitor internal data structure is made visible so that the students can develop and test separately the operations
 *  carried out by the referee, the coaches and the contestants.
 *
 *  Definition of the operations carried out by the coaches:
 *     \li reviewNotes
 *     \li callContestants
 *     \li informReferee
 *     \li endOperCoach.
 *
 *  \author António Rui Borges - November 2015
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>

#include "probConst.h"
#include "probDataStruct.h"
#include "logging.h"
#include "monGameOfRopeDS.h"

/** \brief logging file name */
extern char nFic[51];

/** \brief coaches' thread return status array */
extern int statusCoach[C];

/* allusion to internal function */
static void selectContestants (unsigned int coachId, unsigned int select[]);

/**
 *  \brief Greeting the run
 */
extern void coachGreeting(unsigned int coachId)
{
  fprintf(stderr, "\e[32;1mI'm coach #%u\e[0m\n", coachId);
}


/**
 *  \brief Review notes operation.
 *
 *  The coach review his notes before a trial and waits for a call from the referee to a new trial.
 *  The internal state should be saved.
 *
 *  \param coachId coach id
 */

void reviewNotes (unsigned int coachId)
{
  if ((statusCoach[coachId] = pthread_mutex_lock (&accessCR)) != 0)                                /* enter monitor */
     { errno = statusCoach[coachId];                                                         /* save error in errno */
       perror ("error on entering monitor(CH)");
       statusCoach[coachId] = EXIT_FAILURE;
       pthread_exit (&statusCoach[coachId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  fSt.st.coachStat[coachId] = WAIT_FOR_REFEREE_COMMAND;
  saveState(nFic, &fSt);

  while(!chooseTeam[coachId]){
    if ((statusCoach[coachId] = pthread_cond_wait(&waitForNotice[coachId], &accessCR)) != 0)                                /* enter monitor */
     { errno = statusCoach[coachId];                                                         /* save error in errno */
       perror ("error on waiting to chooseTeam(CH)");
       statusCoach[coachId] = EXIT_FAILURE;
       pthread_exit (&statusCoach[coachId]);
     }
  }
  chooseTeam[coachId] = false;

  if ((statusCoach[coachId] = pthread_mutex_unlock (&accessCR)) != 0)                                /* exit monitor */
     { errno = statusCoach[coachId];                                                          /* save error in errno */
       perror ("error on exiting monitor(CH)");
       statusCoach[coachId] = EXIT_FAILURE;
       pthread_exit (&statusCoach[coachId]);
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

void callContestants(unsigned int coachId)
{
  if ((statusCoach[coachId] = pthread_mutex_lock (&accessCR)) != 0)                                /* enter monitor */
     { errno = statusCoach[coachId];                                                         /* save error in errno */
       perror ("error on entering monitor(CH)");
       statusCoach[coachId] = EXIT_FAILURE;
       pthread_exit (&statusCoach[coachId]);
     }
  pthread_once (&init, initialization);

  unsigned int select[M];
  fSt.st.coachStat[coachId] = ASSEMBLE_TEAM;
  selectContestants(coachId, select);
  saveState(nFic, &fSt);

  int m;
  for(m=0;m < M;m++)
  {
    joinTheTeam[coachId][select[m]] = true;
    if ((statusCoach[coachId] = pthread_cond_signal(&waitForCommand[coachId][select[m]])) != 0 )
      { errno = statusCoach[coachId];                                                   /* save error in errno */
       perror ("error on signaling contestants (CH)");
       statusCoach[coachId] = EXIT_FAILURE;
       pthread_exit (&statusCoach[coachId]);
     }
  }

  while(nContInPosition[coachId] < M)
  { 
    if ((statusCoach[coachId] = pthread_cond_wait(&waitForNotice[coachId], &accessCR)) != 0 )
      { errno = statusCoach[coachId];                   
        perror ("error on waiting for CT (CH)");
        statusCoach[coachId] = EXIT_FAILURE;
        pthread_exit (&statusCoach[coachId]);
      }
  }
  nContInPosition[coachId] = 0;

  /* internal data initialization */
  if ((statusCoach[coachId] = pthread_mutex_unlock (&accessCR)) != 0)                                /* exit monitor */
     { errno = statusCoach[coachId];                                                          /* save error in errno */
       perror ("error on exiting monitor(CH)");
       statusCoach[coachId] = EXIT_FAILURE;
       pthread_exit (&statusCoach[coachId]);
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

void informReferee (unsigned int coachId)
{
  if ((statusCoach[coachId] = pthread_mutex_lock (&accessCR)) != 0)                                /* enter monitor */
     { errno = statusCoach[coachId];                                                         /* save error in errno */
       perror ("error on entering monitor(CH)");
       statusCoach[coachId] = EXIT_FAILURE;
       pthread_exit (&statusCoach[coachId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  fSt.st.coachStat[coachId] = WATCH_TRIAL;
  saveState(nFic, &fSt);
  nCoaches++;

  if(nCoaches == C){
    if ((statusCoach[coachId] = pthread_cond_signal(&proceed)) != 0 )
      { errno = statusCoach[coachId];                                                   /* save error in errno */
       perror ("error on signaling referee (CH)");
       statusCoach[coachId] = EXIT_FAILURE;
       pthread_exit (&statusCoach[coachId]);
     }
  }

  while(!trialDecision[coachId]){
      if ((statusCoach[coachId] = pthread_cond_wait(&waitForNotice[coachId], &accessCR)) != 0 )
      { errno = statusCoach[coachId];                   
        perror ("error on waiting for referee (CH)");
        statusCoach[coachId] = EXIT_FAILURE;
        pthread_exit (&statusCoach[coachId]);
      }
  }
  trialDecision[coachId] = false;

  if ((statusCoach[coachId] = pthread_mutex_unlock (&accessCR)) != 0)                                /* exit monitor */
     { errno = statusCoach[coachId];                                                          /* save error in errno */
       perror ("error on exiting monitor(CH)");
       statusCoach[coachId] = EXIT_FAILURE;
       pthread_exit (&statusCoach[coachId]);
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

  if ((fSt.nGame != 0) || (fSt.game[fSt.nGame].nTrial != 0))                        /* update contestants' strengths */
     { for (n = 0; n < N; n++)
         inTrial[n] = false;
       if (fSt.game[fSt.nGame].nTrial != 0)
          for (m = 0; m < M; m++)
            inTrial[fSt.game[fSt.nGame].trial[fSt.game[fSt.nGame].nTrial-1].id[coachId][m]] = true;
          else for (m = 0; m < M; m++)
                 inTrial[fSt.game[fSt.nGame-1].trial[fSt.game[fSt.nGame-1].nTrial].id[coachId][m]] = true;
       for (n = 0; n < N; n++)
         if (inTrial[n])
            { if (fSt.st.contStat[coachId][n].strength != 0)
                 fSt.st.contStat[coachId][n].strength -= 1;
            }
            else fSt.st.contStat[coachId][n].strength += 1;
     }

  for (n = 0; n < N; n++)
    id[n] = n;
  if ((coachId == 0) || ((fSt.nGame == 0) && (fSt.game[fSt.nGame].nTrial == 0)) ||
      ((fSt.nGame != 0) && (fSt.game[fSt.nGame].nTrial == 0) && (fSt.game[fSt.nGame-1].pos <= 0)) ||
      ((fSt.game[fSt.nGame].nTrial != 0) && (fSt.game[fSt.nGame].trial[fSt.game[fSt.nGame].nTrial].pos <= 0)))
     { for (m = 0; m < M; m++)
       for (n = m+1; n < N; n++)
         if (fSt.st.contStat[coachId][id[m]].strength <= fSt.st.contStat[coachId][id[n]].strength)
            { tmp = id[m];
              id[m] = id[n];
              id[n] = tmp;
            }
       for (m = 0; m < M; m++)
         select[m] = id[m];
     }
     else if ((fSt.nGame != 0) && (fSt.game[fSt.nGame].nTrial == 0))
             for (m = 0; m < M; m++)
               select[m] = fSt.game[fSt.nGame-1].trial[fSt.game[fSt.nGame-1].nTrial].id[coachId][m];
             else for (m = 0; m < M; m++)
                    select[m] = fSt.game[fSt.nGame].trial[fSt.game[fSt.nGame].nTrial-1].id[coachId][m];
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

bool endOperCoach (unsigned int coachId)
{
  if ((statusCoach[coachId] = pthread_mutex_lock (&accessCR)) != 0)                                 /* enter monitor */
     { errno = statusCoach[coachId];                                                          /* save error in errno */
       perror ("error on entering monitor(CH)");
       statusCoach[coachId] = EXIT_FAILURE;
       pthread_exit (&statusCoach[coachId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  bool endOp;                                                                              /* end of operations flag */
  bool alert[N];                                                                        /* wake up contestants array */
  unsigned int n, m;                                                                           /* counting variables */

  endOp = fSt.end;                                                                     /* get end of operations flag */
  if (endOp)
     { fSt.st.coachStat[coachId] = WAIT_FOR_REFEREE_COMMAND;                                         /* state change */
       for (n = 0; n < N; n++)
         alert[n] = true;
       for (m = 0; m < M; m++)
         alert[fSt.game[fSt.nGame].trial[fSt.game[fSt.nGame].nTrial].id[coachId][m]] = false;
       for (n = 0; n < N; n++)
         if (alert[n])
            { fSt.st.contStat[coachId][n].strength += 1;                                          /* update strength */
              joinTheTeam[coachId][n] = true;                                      /* wake up non active contestants */
              if ((statusCoach[coachId] = pthread_cond_signal (&waitForCommand[coachId][n])) != 0)
                 { errno = statusCoach[coachId];                                              /* save error in errno */
                   perror ("error on signaling in waitForCommand for seating at the bench");
                   statusCoach[coachId] = EXIT_FAILURE;
                   pthread_exit (&statusCoach[coachId]);
                 }
            }
            else if (fSt.st.contStat[coachId][n].strength != 0)
                    fSt.st.contStat[coachId][n].strength -= 1;                                    /* update strength */
       saveState (nFic, &fSt);                                                 /* save present state in the log file */
     }

  if ((statusCoach[coachId] = pthread_mutex_unlock (&accessCR)) != 0)                                /* exit monitor */
     { errno = statusCoach[coachId];                                                          /* save error in errno */
       perror ("error on exiting monitor(CH)");
       statusCoach[coachId] = EXIT_FAILURE;
       pthread_exit (&statusCoach[coachId]);
     }

  return endOp;
}
