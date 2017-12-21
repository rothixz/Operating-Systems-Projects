/**
 *  \file monGameOfRopeCT.c (implementation file)
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
 *  Definition of the operations carried out by the contestants:
 *     \li seatDown
 *     \li followCoachAdvice
 *     \li getReady
 *     \li amDone
 *     \li endOperContestant.
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

/** \brief contestants' thread return status array */
extern int statusCont[C][N];

/**
 *  \brief Greeting the run
 */
extern void contestantGreeting(unsigned int coachId, unsigned int contId)
{
  fprintf(stderr, "\e[32;1mI'm costestant #%u-%u\e[0m\n", coachId, contId);
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

bool seatDown (unsigned int coachId, unsigned int contId)
{
  if ((statusCont[coachId][contId] = pthread_mutex_lock (&accessCR)) != 0)                          /* enter monitor */
     { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on entering monitor(CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  fSt.st.contStat[coachId][contId].stat = SEAT_AT_THE_BENCH;
  saveState(nFic, &fSt);

  while(!joinTheTeam[coachId][contId])
  {
    if ( ( statusCont[coachId][contId] = pthread_cond_wait(&waitForCommand[coachId][contId], &accessCR) ) != 0 )
      { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on wait in waitForCommand (CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
     }
  }
  joinTheTeam[coachId][contId] = false;

  bool endOp = fSt.end;

  if ((statusCont[coachId][contId] = pthread_mutex_unlock (&accessCR)) != 0)                         /* exit monitor */
     { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on exiting monitor(CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
     }
  return endOp;
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

void followCoachAdvice (unsigned int coachId, unsigned int contId)
{
  if ((statusCont[coachId][contId] = pthread_mutex_lock (&accessCR)) != 0)                          /* enter monitor */
     { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on entering monitor(CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */
  fSt.st.contStat[coachId][contId].stat = STAND_IN_POSITION;
  fSt.game[fSt.nGame].trial[fSt.game[fSt.nGame].nTrial].id[coachId][nContInPosition[coachId]] = contId;
  nContInPosition[coachId]++;
  saveState(nFic, &fSt);
   
  if(nContInPosition[coachId] == M)
  { 
    if ((statusCont[coachId][contId] = pthread_cond_signal(&waitForNotice[coachId])) != 0 )
      { errno = statusCont[coachId][contId];                   
        perror ("error on sending signal to coach (CT)");
        statusCont[coachId][contId] = EXIT_FAILURE;
        pthread_exit (&statusCont[coachId][contId]);
      }
  }
  

  while(!startPulling[coachId][contId]){
    if ((statusCont[coachId][contId] = pthread_cond_wait(&waitForCommand[coachId][contId], &accessCR) ) != 0 )
       { errno = statusCont[coachId][contId];                                              
         perror ("error on waiting in waitForCommand (CT)");
         statusCont[coachId][contId] = EXIT_FAILURE;
         pthread_exit (&statusCont[coachId][contId]);
       }
  }
  startPulling[coachId][contId] = false;
  
  if ((statusCont[coachId][contId] = pthread_mutex_unlock (&accessCR)) != 0)                         /* exit monitor */
     { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on exiting monitor(CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
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

void getReady (unsigned int coachId, unsigned int contId)
{
  if ((statusCont[coachId][contId] = pthread_mutex_lock (&accessCR)) != 0)                          /* enter monitor */
     { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on entering monitor(CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */
  fSt.st.contStat[coachId][contId].stat = DO_YOUR_BEST;
  saveState(nFic, &fSt);

  if ((statusCont[coachId][contId] = pthread_mutex_unlock (&accessCR)) != 0)                         /* exit monitor */
     { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on exiting monitor(CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
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

void amDone (unsigned int coachId, unsigned int contId)
{
  if ((statusCont[coachId][contId] = pthread_mutex_lock (&accessCR)) != 0)                          /* enter monitor */
     { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on entering monitor(CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */
  nContestants++;
  if(nContestants == C*M)
  {
    if ( ( statusCont[coachId][contId] = pthread_cond_signal(&proceed) ) != 0 )
      { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on wait in waitForCommand (CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
     }
  }

  while(!returnToBench[coachId][contId]){
    if ( ( statusCont[coachId][contId] = pthread_cond_wait(&waitForCommand[coachId][contId], &accessCR) ) != 0 )
      { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on wait in waitForNotice (CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
     }
       
  }
  returnToBench[coachId][contId] = false;
  if ((statusCont[coachId][contId] = pthread_mutex_unlock (&accessCR)) != 0)                         /* exit monitor */
     { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on exiting monitor(CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
     }
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

bool endOperContestant (unsigned int coachId, unsigned int contId)
{
  if ((statusCont[coachId][contId] = pthread_mutex_lock (&accessCR)) != 0)                          /* enter monitor */
     { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on entering monitor(CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  bool endOp;                                                                              /* end of operations flag */

  endOp = fSt.end;                                                                     /* get end of operations flag */
  if (endOp)
     { fSt.st.contStat[coachId][contId].stat = SEAT_AT_THE_BENCH;                                    /* state change */
       saveState (nFic, &fSt);                                                 /* save present state in the log file */
     }

  if ((statusCont[coachId][contId] = pthread_mutex_unlock (&accessCR)) != 0)                         /* exit monitor */
     { errno = statusCont[coachId][contId];                                                   /* save error in errno */
       perror ("error on exiting monitor(CT)");
       statusCont[coachId][contId] = EXIT_FAILURE;
       pthread_exit (&statusCont[coachId][contId]);
     }

  return endOp;
}
