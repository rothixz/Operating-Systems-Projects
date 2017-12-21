/**
 *  \file logging.h (interface file)
 *
 *  \brief Problem name: Game of the rope.
 *
 *  \brief Concept: Pedro Mariano
 *
 *  \brief Logging the internal state of the problem into a file.
 *
 *  Defined operations:
 *     \li file initialization
 *     \li writing the match header at the end of the file
 *     \li writing the game header at the end of the file
 *     \li writing the present state as a single line at the end of the file
 *     \li writing the game result at the end of the file
 *     \li writing the match result at the end of the file.
 *
 *  \author António Rui Borges - November 2015
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include "probDataStruct.h"

/**
 *  \brief File initialization.
 *
 *  The function creates the logging file and writes its header.
 *  If <tt>nFic</tt> is a null pointer or a null string, the file is created under a predefined name <em>log</em>.
 *
 *  The header consists of
 *       \li a line title
 *       \li a blank line
 *       \li a double line describing the meaning of the different fields of the state line.
 *
 *  \param nFic name of the logging file
 */

extern void createLog (char nFic[]);

/**
 *  \brief Writing the match header at the end of the file.
 *
 *  If <tt>nFic</tt> is a null pointer or a null string, the lines are appended to a file under the predefined
 *  name <em>log</em>.
 *
 *  The match header consists of
 *       \li a double line describing the meaning of the different fields of the state line.
 *
 *  \param nFic name of the logging file
 *  \param p_fSt pointer to the location where the full internal state of the problem is stored
 */

extern void saveMatchHeader (char nFic[], FULL_STAT *p_fSt);

/**
 *  \brief Writing the game header at the end of the file.
 *
 *  If <tt>nFic</tt> is a null pointer or a null string, the lines are appended to a file under the predefined
 *  name <em>log</em>.
 *
 *  The game header consists of
 *       \li a line stating the game number
 *       \li a double line describing the meaning of the different fields of the state line.
 *
 *  \param nFic name of the logging file
 *  \param p_fSt pointer to the location where the full internal state of the problem is stored
 */

extern void saveGameHeader (char nFic[], FULL_STAT *p_fSt);

/**
 *  \brief Writing the present full state as a single line at the end of the file.
 *
 *  If <tt>nFic</tt> is a null pointer or a null string, the lines are appended to a file under the predefined
 *  name <em>log</em>.
 *
 *  The following layout is obeyed for the full state in a single line
 *    \li referee state
 *    \li coaches state (c = 0,...,C-1)
 *    \li contestants state and strength (n = 0,..., N-1)
 *    \li trial state (c = 0,...,C-1 and m = 0,..., M-1).
 *
 *  \param nFic name of the logging file
 *  \param p_fSt pointer to the location where the full internal state of the problem is stored
 */

extern void saveState (char nFic[], FULL_STAT *p_fSt);

/**
 *  \brief Writing the game result at the end of the file.
 *
 *  If <tt>nFic</tt> is a null pointer or a null string, the lines are appended to a file under the predefined
 *  name <em>log</em>.
 *
 *  The game result consists of a single line describing who has won and how.
 *
 *  \param nFic name of the logging file
 *  \param p_fSt pointer to the location where the full internal state of the problem is stored
 */

extern void saveGameResult (char nFic[], FULL_STAT *p_fSt);

/**
 *  \brief Writing the match result at the end of the file.
 *
 *  If <tt>nFic</tt> is a null pointer or a null string, the lines are appended to a file under the predefined
 *  name <em>log</em>.
 *
 *  The match result consists of a single line describing who has won.
 *
 *  \param nFic name of the logging file
 *  \param p_fSt pointer to the location where the full internal state of the problem is stored
 */

extern void saveMatchResult (char nFic[], FULL_STAT *p_fSt);

#endif /* LOGGING_H_ */
