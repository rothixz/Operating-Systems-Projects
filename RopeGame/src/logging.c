/**
 *  \file logging.c (implementation file)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "probConst.h"
#include "probDataStruct.h"

/**
 *  \brief File initialization.
 *
 *  The function creates the logging file and writes its header.
 *  If <tt>nFic</tt> is a null pointer or a null string, the file is created under a predefined name <em>log</em>.
 *
 *  The file header consists of
 *       \li a title line
 *       \li a blank line.
 *
 *  \param nFic name of the logging file
 */

void createLog (char nFic[])
{
  FILE *fic;                                                                                      /* file descriptor */
  char *dName = "log",                                                                      /* default log file name */
       *fName;                                                                                      /* log file name */

  if ((nFic == NULL) || (strlen (nFic) == 0))
     fName = dName;
     else fName = nFic;
  if ((fic = fopen (fName, "w")) == NULL)
     { perror ("error on the creation of log file");
       exit (EXIT_FAILURE);
     }

  /* title line + blank line */

  fprintf (fic, "%31cGame of the Rope - Description of the internal state\n\n", ' ');

  if (fclose (fic) == EOF)
     { perror ("error on closing of log file");
       exit (EXIT_FAILURE);
     }
}

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

void saveMatchHeader (char nFic[], FULL_STAT *p_fSt)
{
  FILE *fic;                                                                                      /* file descriptor */
  char *dName = "log",                                                                      /* default log file name */
       *fName;                                                                                      /* log file name */
  unsigned int c, n;                                                                           /* counting variables */

  if ((nFic == NULL) || (strlen (nFic) == 0))
     fName = dName;
     else fName = nFic;
  if ((fic = fopen (fName, "a")) == NULL)
     { perror ("error on opening for appending of log file");
       exit (EXIT_FAILURE);
     }

  /* first line of field description */

  fprintf (fic, "Ref ");
  for (c= 1; c <= C; c++)
  { fprintf (fic, "Coa %u ", c);
    for (n= 1; n <= N; n++)
      fprintf (fic, "Cont %u ", n);
  }
  fprintf (fic, "      Trial\n");

  /* second line of field description */

  fprintf (fic, "Sta ");
  for (c= 1; c <= C; c++)
  { fprintf (fic, " Stat ");
    for (n= 1; n <= N; n++)
      fprintf (fic, "Sta SG ");
  }
  fprintf (fic, "3 2 1 . 1 2 3 NB PS\n");

  if (fclose (fic) == EOF)
     { perror ("error on closing of log file");
       exit (EXIT_FAILURE);
     }
}

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

void saveGameHeader (char nFic[], FULL_STAT *p_fSt)
{
  FILE *fic;                                                                                      /* file descriptor */
  char *dName = "log",                                                                      /* default log file name */
       *fName;                                                                                      /* log file name */
  unsigned int c, n;                                                                           /* counting variables */

  if ((nFic == NULL) || (strlen (nFic) == 0))
     fName = dName;
     else fName = nFic;
  if (p_fSt->nGame == (unsigned int) -1) return;
  if ((fic = fopen (fName, "a")) == NULL)
     { perror ("error on opening for appending of log file");
       exit (EXIT_FAILURE);
     }

  fprintf (fic, "Game %u\n", p_fSt->nGame + 1);

  /* first line of field description */

  fprintf (fic, "Ref ");
  for (c= 1; c <= C; c++)
  { fprintf (fic, "Coa %u ", c);
    for (n= 1; n <= N; n++)
      fprintf (fic, "Cont %u ", n);
  }
  fprintf (fic, "      Trial\n");

  /* second line of field description */

  fprintf (fic, "Sta ");
  for (c= 1; c <= C; c++)
  { fprintf (fic, " Stat ");
    for (n= 1; n <= N; n++)
      fprintf (fic, "Sta SG ");
  }
  fprintf (fic, "3 2 1 . 1 2 3 NB PS\n");

  if (fclose (fic) == EOF)
     { perror ("error on closing of log file");
       exit (EXIT_FAILURE);
     }
}

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

void saveState (char nFic[], FULL_STAT *p_fSt)
{
  FILE *fic;                                                                                      /* file descriptor */
  char *dName = "log",                                                                      /* default log file name */
       *fName;                                                                                      /* log file name */
  unsigned int c, n, m;                                                                        /* counting variables */

  if (p_fSt->alreadyRun && (p_fSt->nGame == (unsigned int) -1))
     return;
     else p_fSt->alreadyRun = true;
  if ((nFic == NULL) || (strlen (nFic) == 0))
     fName = dName;
     else fName = nFic;
  if ((fic = fopen (fName, "a")) == NULL)
     { perror ("error on opening for appending of log file");
       exit (EXIT_FAILURE);
     }

  /* present full state description */

  switch (p_fSt->st.refereeStat)
  { case START_OF_THE_MATCH:        fprintf (fic, "SOM ");
                                    break;
    case START_OF_A_GAME:           fprintf (fic, "SOG ");
                                    break;
    case TEAMS_READY:               fprintf (fic, "TRY ");
                                    break;
    case WAIT_FOR_TRIAL_CONCLUSION: fprintf (fic, "WTC ");
                                    break;
    case END_OF_A_GAME:             fprintf (fic, "EOG ");
                                    break;
    case END_OF_THE_MATCH:          fprintf (fic, "EOM ");
                                    break;
    default:                        fprintf (fic, "*** ");
  }
  for (c = 0; c < C; c++)
  { switch (p_fSt->st.coachStat[c])
    { case WAIT_FOR_REFEREE_COMMAND: fprintf (fic, " WFRC ");
                                     break;
      case ASSEMBLE_TEAM:            fprintf (fic, " ASTM ");
                                     break;
      case WATCH_TRIAL:              fprintf (fic, " WCTL ");
                                     break;
      default:                       fprintf (fic, " **** ");
    }
    for (n = 0; n < N; n++)
    { switch (p_fSt->st.contStat[c][n].stat)
      { case SEAT_AT_THE_BENCH: fprintf (fic, "SAB ");
                                break;
        case STAND_IN_POSITION: fprintf (fic, "SIP ");
                                break;
        case DO_YOUR_BEST:      fprintf (fic, "DYB ");
                                break;
        default:                fprintf (fic, "*** ");
      }
      fprintf (fic, "%2u ", p_fSt->st.contStat[c][n].strength);
    }
  }
  for (c = 0; c < C; c++)
  { for (m = 0; m < M; m++)
      if ((p_fSt->nGame == (unsigned int) -1)  || (p_fSt->game[p_fSt->nGame].nTrial == (unsigned int) -1))
         fprintf (fic, "- ");
         else if (p_fSt->game[p_fSt->nGame].trial[p_fSt->game[p_fSt->nGame].nTrial].id[c][m] == (unsigned int) -1)
                fprintf (fic, "- ");
                else fprintf (fic, "%u ", p_fSt->game[p_fSt->nGame].trial[p_fSt->game[p_fSt->nGame].nTrial].id[c][m]+1);
    if (c == 0) fprintf (fic, ". ");
  }
  if ((p_fSt->nGame == (unsigned int) -1)  || (p_fSt->game[p_fSt->nGame].nTrial == (unsigned int) -1))
     fprintf (fic, "-- --");
     else fprintf (fic, "%2u %2d", p_fSt->game[p_fSt->nGame].nTrial+1,
                  p_fSt->game[p_fSt->nGame].trial[p_fSt->game[p_fSt->nGame].nTrial].pos);
  fprintf (fic, "\n");

  if (fclose (fic) == EOF)
     { perror ("error on closing of log file");
       exit (EXIT_FAILURE);
     }
}

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

void saveGameResult (char nFic[], FULL_STAT *p_fSt)
{
  FILE *fic;                                                                                      /* file descriptor */
  char *dName = "log",                                                                      /* default log file name */
       *fName;                                                                                      /* log file name */
  unsigned int team;                                                                                      /* team id */

  if ((nFic == NULL) || (strlen (nFic) == 0))
     fName = dName;
     else fName = nFic;
  if (p_fSt->nGame == (unsigned int) -1) return;
  if (p_fSt->game[p_fSt->nGame].nTrial == (unsigned int) -1) return;
  if ((fic = fopen (fName, "a")) == NULL)
     { perror ("error on opening for appending of log file");
       exit (EXIT_FAILURE);
     }

  if (p_fSt->game[p_fSt->nGame].pos == 0)
     fprintf (fic, "Game %u was a draw.\n", p_fSt->nGame+1);
     else { if (p_fSt->game[p_fSt->nGame].pos < 0)
               team = 0;
               else team = 1;
            fprintf (fic, "Game %u was won by team %u ", p_fSt->nGame + 1, team + 1);
            if (abs(p_fSt->game[p_fSt->nGame].pos) >= S)
               fprintf (fic, "by knock out in %u trials.\n", p_fSt->game[p_fSt->nGame].nTrial+1);
               else fprintf (fic, "by points.\n");
          }

  if (fclose (fic) == EOF)
     { perror ("error on closing of log file");
       exit (EXIT_FAILURE);
     }
}

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

void saveMatchResult (char nFic[], FULL_STAT *p_fSt)
{
  FILE *fic;                                                                                      /* file descriptor */
  char *dName = "log",                                                                      /* default log file name */
       *fName;                                                                                      /* log file name */
  unsigned int res[C];                                                                               /* game results */
  unsigned int c, g;                                                                           /* counting variables */

  if ((nFic == NULL) || (strlen (nFic) == 0))
     fName = dName;
     else fName = nFic;
  if (p_fSt->nGame == (unsigned int) -1) return;
  if (p_fSt->nGame != (G-1)) return;
  if ((fic = fopen (fName, "a")) == NULL)
     { perror ("error on opening for appending of log file");
       exit (EXIT_FAILURE);
     }

  for (c = 0; c < C; c++)
    res[c] = 0;
  for (g = 0; g < G; g++)
    if (p_fSt->game[g].pos < 0)
       res[0] += 1;
       else if (p_fSt->game[g].pos > 0)
               res[1] += 1;
  if (res[0] == res[1])
     fprintf (fic, "Match was a draw.\n");
     else if (res[0] > res[1])
             fprintf (fic, "Match was won by team 1 (%u-%u).\n", res[0], res[1]);
             else fprintf (fic, "Match was won by team 2 (%u-%u).\n", res[1], res[0]);

  if (fclose (fic) == EOF)
     { perror ("error on closing of log file");
       exit (EXIT_FAILURE);
     }
}
