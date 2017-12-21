/**
 *  \file probDataStruct.h (interface file)
 *
 *  \brief Problem name: Game of the rope.
 *
 *  \brief Concept: Pedro Mariano
 *
 *  Definition of internal data structures.
 *
 *  They specify internal metadata about the status of the intervening entities.
 *
 *  \author António Rui Borges - November 2015
 */

#ifndef PROBDATASTRUCT_H_
#define PROBDATASTRUCT_H_

#include <stdbool.h>

#include "probConst.h"

/**
 *  \brief Definition of <em>state of a contestant</em> data type.
 */
typedef struct
        { /** \brief internal state */
          unsigned int stat;
          /** \brief physical strength */
          unsigned int strength;
        } STAT_CONT;

/**
 *  \brief Definition of <em>state of the intervening entities</em> data type.
 */
typedef struct
        { /** \brief referee state */
          unsigned int refereeStat;
          /** \brief coaches state array */
          unsigned int coachStat[C];
          /** \brief contestants state array */
          STAT_CONT contStat[C][N];
        } STAT;

/**
 *  \brief Definition of <em>trial</em> data type.
 */
typedef struct
        { /** \brief teams composition */
          unsigned int id[C][M];
          /** \brief starting position */
          int pos;
        } TRIAL;

/**
 *  \brief Definition of <em>game</em> data type.
 */
typedef struct
        { /** \brief trials composition */
          TRIAL trial[T];
          /** \brief trial number */
          unsigned int nTrial;
          /** \brief ending position */
          int pos;
        } GAME;

/**
 *  \brief Definition of <em>full state of the problem</em> data type.
 */
typedef struct
        { /** \brief state of all intervening entities */
          STAT st;
          /** \brief games description */
          GAME game[G];
          /** \brief game number */
          unsigned int nGame;
          /** \brief end of operations */
          bool end;
          /** \brief run once flag */
          bool alreadyRun;
        } FULL_STAT;

#endif /* PROBDATASTRUCT_H_ */
