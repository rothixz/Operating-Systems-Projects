/**
 *  \file soRmdir.c (implementation file)
 *
 *  \author ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <libgen.h>
#include <string.h>

#include "sofs_probe.h"
#include "sofs_const.h"
#include "sofs_rawdisk.h"
#include "sofs_buffercache.h"
#include "sofs_superblock.h"
#include "sofs_inode.h"
#include "sofs_direntry.h"
#include "sofs_datacluster.h"
#include "sofs_basicoper.h"
#include "sofs_basicconsist.h"
#include "sofs_ifuncs_1.h"
#include "sofs_ifuncs_2.h"
#include "sofs_ifuncs_3.h"
#include "sofs_ifuncs_4.h"

/**
 *  \brief Delete a directory.
 *
 *  It tries to emulate <em>rmdir</em> system call.
 *
 *  \param ePath path to the directory to be deleted
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the pointer to the string is \c NULL or the path string is a \c NULL string or the path does
 *                      not describe an absolute path
 *  \return -\c ENAMETOOLONG, if the path name or any of its components exceed the maximum allowed length
 *  \return -\c ENOTDIR, if any of the components of the path is not a directory
 *  \return -\c ELOOP, if the path resolves to more than one symbolic link
 *  \return -\c ENOENT, if no entry with a name equal to any of the components of <tt>ePath</tt> is found
 *  \return -\c ENOTEMPTY, if <tt>ePath</tt> describes a non-empty directory
 *  \return -\c EACCES, if the process that calls the operation has not execution permission on any of the components
 *                      of the path, but the last one
 *  \return -\c EPERM, if the process that calls the operation has not write permission on the directory where
 *                     <tt>ePath</tt> entry is to be removed
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soRmdir (const char *ePath)
{
  soColorProbe (233, "07;31", "soRmdir (\"%s\")\n", ePath);

  /* insert your code here */

  return 0;
}
