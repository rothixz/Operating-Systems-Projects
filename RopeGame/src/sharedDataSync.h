/**
 *  \file sharedDataSync.h (interface file)
 *
 *  \brief Problem name: Game of the rope.
 *
 *  \brief Concept: Pedro Mariano
 *
 *  Synchronization based on semaphores and shared memory.
 *  Implementation with SVIPC.
 *
 *  \brief Definition of the shared data and the synchronization devices.
 *
 *  Both the format of the shared data, which represents the full state of the problem, and the identification of
 *  the different semaphores, which carry out the synchronization among the intervening entities, are provided.
 *
 *  \author António Rui Borges - November 2015
 */

#ifndef SHAREDDATASYNC_H_
#define SHAREDDATASYNC_H_

#include "probConst.h"
#include "probDataStruct.h"

/**
 *  \brief Definition of <em>shared information</em> data type.
 */
typedef struct
        { /** \brief full state of the problem */
          FULL_STAT fSt;
          /** \brief identification of critical region protection semaphore – val = 1 */
          unsigned int access;
          /** \brief identification of referee proceed semaphore – val = 0 */
          unsigned int proceed;
          /** \brief identification of coaches waiting for notice semaphore array – val = 0 (one per coach) */
          unsigned int waitForNotice[C];
          /** \brief identification of contestants waiting for command semaphore array – val = 0 (one per contestant) */
          unsigned int waitForCommand[C][N];
          /** \brief number of inform requests generated by the coaches */
          unsigned int nCoaches;
          /** \brief number of inform requests generated by the contestants after a pulling effort on the rope */
          unsigned int nContestants;
          /** \brief number of contestants in position */
          unsigned int nContInPosition[C];
        } SHARED_DATA;

/** \brief number of semaphores in the set */
#define SEM_NU                 (2+C+(C*N))

/** \brief index of critical region protection semaphore */
#define ACCESS                     1

/** \brief index of referee proceed semaphore */
#define PROCEED                    2

/** \brief base index of coaches waiting for notice semaphore array (one per coach) */
#define B_WAITFORNOTICE            3

/** \brief base index of contestants waiting for command semaphore array (one per contestant) */
#define B_WAITFORCOMMAND           5

#endif /* SHAREDDATASYNC_H_ */
