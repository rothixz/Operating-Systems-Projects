/**
 *  \file probConst.h (interface file)
 *
 *  \brief Problem name: Game of the rope.
 *
 *  \brief Concept: Pedro Mariano
 *
 *  Problem simulation parameters.
 *
 *  \author António Rui Borges - November 2015
 */

#ifndef PROBCONST_H_
#define PROBCONST_H_

/* Generic parameters */

/** \brief number of opposing teams */
#define  C       2
/** \brief number of elements in a team */
#define  N       5
/** \brief number of contestants in a trial */
#define  M       3
/** \brief number of games per match */
#define  G       3
/** \brief maximum number of trials per game */
#define  T       6
/** \brief minimum rope shift to win by knock out */
#define  S       4

/* Referee state constants */

/** \brief the referee starts the match */
#define  START_OF_THE_MATCH                  0
/** \brief the referee starts a new game */
#define  START_OF_A_GAME                     1
/** \brief the referee waits for teams to ready to start a trial */
#define  TEAMS_READY                         2
/** \brief the referee waits for a trial to end */
#define  WAIT_FOR_TRIAL_CONCLUSION           3
/** \brief the referee ends a game */
#define  END_OF_A_GAME                       4
/** \brief the referee ends the match */
#define  END_OF_THE_MATCH                    5

/* Coach state constants */

/** \brief the coach waits for the referee command for a new trial */
#define  WAIT_FOR_REFEREE_COMMAND            0
/** \brief the coach waits for the selected team to be assembled */
#define  ASSEMBLE_TEAM                       1
/** \brief the coach waits the trial to take place */
#define  WATCH_TRIAL                         2

/* Contestant state constants */

/** \brief the contestant seats at the bench waiting for coach advice */
#define  SEAT_AT_THE_BENCH                   0
/** \brief the contestant stands in position at his end of the rope */
#define  STAND_IN_POSITION                   1
/** \brief the contestant pulls as hard as he can */
#define  DO_YOUR_BEST                        2

#endif /* PROBCONST_H_ */
