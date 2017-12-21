/**
 *  \file semSharedMemContestant.h (interface file)
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
 *  \author António Rui Borges - November 2015
 */

#ifndef SEMSHAREDMEMCONTESTANT_H_
#define SEMSHAREDMEMCONTESTANT_H_

#include <stdbool.h>

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

extern bool seatDown (unsigned int coachId, unsigned int contId);

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

extern void followCoachAdvice (unsigned int coachId, unsigned int contId);

/**
 *  \brief Get ready operation.
 *
 *  The contestant gets ready to start pulling the rope.
 *  The internal state should be saved.
 *
 *  \param coachId coach id
 *  \param contId contestant id
 */

extern void getReady (unsigned int coachId, unsigned int contId);

/**
 *  \brief Am done operation.
 *
 *  The contestant ends his pulling effort, informs the referee and waits for the referee decision to return to the bench.
 *  The internal state should not be saved.
 *
 *  \param coachId coach id
 *  \param contId contestant id
 */

extern void amDone (unsigned int coachId, unsigned int contId);

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

extern bool endOperContestant (unsigned int coachId, unsigned int contId);

#endif /* SEMSHAREDMEMCONTESTANT_H_ */
