/**
 *  \file semSharedMemReferee.h (interface file)
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
 *  \author António Rui Borges - December 2013
 */

#ifndef SEMSHAREDMEMREFEREE_H_
#define SEMSHAREDMEMREFEREE_H_

/** \brief game continuation flag */
#define CONT        'C'

/**
 *  \brief Announce new game operation.
 *
 *  The referee starts a game.
 *  The game number should be updated.
 *  Both game header and internal state should be saved.
 *
 *  \param g game number
 */

extern void announceNewGame (unsigned int g);

/**
 *  \brief Call trial operation.
 *
 *  The referee calls the coaches to assemble the teams for a trial and waits for the teams to be ready.
 *  The trial number and the trial initial position should be updated.
 *  The internal state should be saved.
 *
 *  \param t trial number
 */

extern void callTrial (unsigned int t);

/**
 *  \brief Start trial operation.
 *
 *  The referee starts a trial and waits for its conclusion.
 *  The contestants at the ends of the rope have to be alerted for the fact.
 *  The internal state should be saved.
 */

extern void startTrial ();

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

extern char assertTrialDecision ();

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

extern void declareGameWinner (char decision);

/**
 *  \brief Declare match winner.
 *
 *  The referee announces which teams has won the match.
 *  Both internal state and match result should be saved.
 */

extern void declareMatchWinner ();

#endif /* SEMSHAREDMEMREFEREE_H_ */
