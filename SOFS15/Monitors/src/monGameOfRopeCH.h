/**
 *  \file monGameOfRopeCH.h (interface file)
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

#ifndef MONGAMEOFROPECH_H_
#define MONGAMEOFROPECH_H_

#include <stdbool.h>

/**
 *  \brief Greeting the run
 */
extern void coachGreeting(unsigned int coachId);


/**
 *  \brief Review notes operation.
 *
 *  The coach review his notes before a trial and waits for a call from the referee to a new trial.
 *  The internal state should be saved.
 *
 *  \param coachId coach id
 */

extern void reviewNotes (unsigned int coachId);

/**
 *  \brief Call contestants operation.
 *
 *  The coach updates the contestants strenghts, selects some of them to form the team according to a predefined strategy,
 *  calls them to stand at the end of the rope and waits for all of them to be in position.
 *  The internal state should be saved.
 *
 *  \param coachId coach id
 */

extern void callContestants(unsigned int coachId);

/**
 *  \brief Inform referee operation.
 *
 *  The coach of the last team to become ready informs the referee.
 *  The coach waits for the trial to take place.
 *  The internal state should be saved.
 *
 *  \param coachId coach id
 */

extern void informReferee (unsigned int coachId);

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

extern bool endOperCoach (unsigned int coachId);

#endif /* MONGAMEOFROPECH_H_ */
