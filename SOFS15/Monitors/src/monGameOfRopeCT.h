/**
 *  \file monGameOfRopeCT.h (interface file)
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
 *     \li amDone.
 *
 *  \author António Rui Borges - November 2015
 */

#ifndef MONGAMEOFROPECT_H_
#define MONGAMEOFROPECT_H_

#include <stdbool.h>

/**
 *  \brief Greeting the run
 */
extern void contestantGreeting(unsigned int coachId, unsigned int contId);


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

#endif /* MONGAMEOFROPECT_H_ */
