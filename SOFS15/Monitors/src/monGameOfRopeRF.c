/**
 *  \file monGameOfRopeRF.c (implementation file)
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
 *  Definition of the operations carried out by the referee:
 *     \li announceNewGame
 *     \li callTrial
 *     \li startTrial
 *     \li assertTrialDecision
 *     \li declareGameWinner
 *     \li declareMatchWinner.
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

#include "probConst.h"
#include "probDataStruct.h"
#include "logging.h"
#include "monGameOfRopeDS.h"

/** \brief logging file name */
extern char nFic[51];

/** \brief referee thread return status array */
extern int statusRef;

/**
 *  \brief Greeting the run
 */
extern void refereeGreeting()
{
  fprintf(stderr, "\e[32;1mI'm the referee\e[0m\n");
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

void announceNewGame (unsigned int g)
{
  if ((statusRef = pthread_mutex_lock (&accessCR)) != 0)                                            /* enter monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on entering monitor(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  /* insert your code here */
  fSt.st.refereeStat = START_OF_A_GAME;
  saveGameHeader(nFic, &fSt);
  saveState(nFic, &fSt);
  fSt.st.nGame = g;

  if ((statusRef = pthread_mutex_unlock (&accessCR)) != 0)                                           /* exit monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on exiting monitor(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
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

void callTrial (unsigned int t)
{
  if ((statusRef = pthread_mutex_lock (&accessCR)) != 0)                                            /* enter monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on entering monitor(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  /* insert your code here */
  fSt.st.refereeStat = TEAMS_READY;                                           /* State = TEAMS_READY */
  fst.game[fSt.nGame].nTrial = t;                                         /* Updates trial number */
  if(fSt.game[fSt.nGame].nTrial == 0)
    fSt.game[fSt.nGame].trial[fSt.game[fSt.nGame].nTrial].pos = 0;                      /* Initial pos = 0 */
  else
    fSt.game[fSt.nGame].trial[fSt.game[fSt.nGame].nTrial].pos = fSt.game[fSt.nGame].pos;      /* Initial pos = Final pos */
  saveState (nFic, &(fSt)); 

  int c;
  for (c = 0; c < C; c++)
  {
    pthread_cond_wait(&waitForNotice[coaches], &accessCR);                                               /* Wakes coaches */
  }  

  pthread_cond_wait(&proceed, &accessCR);


  if ((statusRef = pthread_mutex_unlock (&accessCR)) != 0)                                           /* exit monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on exiting monitor(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
     }
}

/**
 *  \brief Start trial operation.
 *
 *  The referee starts a trial and waits for its conclusion.
 *  The contestants at the ends of the rope have to be alerted for the fact.
 *  The internal state should be saved.
 */

void startTrial ()
{
  if ((statusRef = pthread_mutex_lock (&accessCR)) != 0)                                            /* enter monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on entering monitor(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  /* insert your code here */
  fSt.st.refereeStat = WAIT_FOR_TRIAL_CONCLUSION;                                   /* State = WAIT_FOR_TRIAL_CONCLUSION */
  saveState (nFic, &(fSt));                                               /* Save state */
  int c;
  for (c = 0; c < C; c++)
  {
    if ((statusRef = pthread_cond_signal(&waitForCommand[coachId][fSt.game[fSt.nGame].trial[fSt.game[fSt.nGame].nTrial].id[c][M]], &accessCR)) != 0)                                            /* enter monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on waking the contestants(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
     }
  }
  while(!nContestants != C*M){
    if ((statusRef = pthread_cond_wait(&proceed, &accessCR)) != 0)                                            /* enter monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on waiting for end of game(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
     }
  }
  nContestants = 0;

  if ((statusRef = pthread_mutex_unlock (&accessCR)) != 0)                                           /* exit monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on exiting monitor(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
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

char assertTrialDecision ()
{
  if ((statusRef = pthread_mutex_lock (&accessCR)) != 0)                                            /* enter monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on entering monitor(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  /* insert your code here */
  

  if ((statusRef = pthread_mutex_unlock (&accessCR)) != 0)                                           /* exit monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on exiting monitor(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
     }

  return 'E';
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

void declareGameWinner (char decision)
{
  if ((statusRef = pthread_mutex_lock (&accessCR)) != 0)                                            /* enter monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on entering monitor(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  /* insert your code here */

  if ((statusRef = pthread_mutex_unlock (&accessCR)) != 0)                                           /* exit monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on exiting monitor(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
     }
}

/**
 *  \brief Declare match winner.
 *
 *  The referee announces which team has won the match.
 *  Both internal state and match result should be saved.
 */

void declareMatchWinner ()
{
  if ((statusRef = pthread_mutex_lock (&accessCR)) != 0)                                            /* enter monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on entering monitor(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  /* insert your code here */

  if ((statusRef = pthread_mutex_unlock (&accessCR)) != 0)                                           /* exit monitor */
     { errno = statusRef;                                                                     /* save error in errno */
       perror ("error on exiting monitor(RF)");
       statusRef = EXIT_FAILURE;
       pthread_exit (&statusRef);
     }
}
