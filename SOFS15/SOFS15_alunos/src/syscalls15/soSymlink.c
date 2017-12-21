/**
 *  \file soSymlink.c (implementation file)
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
 *  \brief Make a new name for a regular file or a directory.
 *
 *  It tries to emulate <em>symlink</em> system call.
 *
 *  \remark The permissions set for the symbolic link should have read (r), write (w) and execution (x) permissions for
 *          both <em>user</em>, <em>group</em> and <em>other</em>.
 *
 *  \param effPath path to be stored in the symbolic link file
 *  \param ePath path to the symbolic link
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the pointer to either of the strings is \c NULL or or any of the path strings is a \c NULL
 *                      string or the second string does not describe an absolute path
 *  \return -\c ENAMETOOLONG, if the either path names or any of its components exceed the maximum allowed length
 *  \return -\c ERELPATH, if the second path is relative
 *  \return -\c ENOTDIR, if any of the components of the second path, but the last one, is not a directory
 *  \return -\c ELOOP, if the second path resolves to more than one symbolic link
 *  \return -\c ENOENT, if no entry with a name equal to any of the components of <tt>ePath</tt>, but the last one, is
 *                      found
 *  \return -\c EEXIST, if a file described by <tt>ePath</tt> already exists
 *  \return -\c EACCES, if the process that calls the operation has not execution permission on any of the components
 *                      of the second path, but the last one
 *  \return -\c EPERM, if the process that calls the operation has not write permission on the directory where
 *                     <tt>ePath</tt> entry is to be added
 *  \return -\c ENOSPC, if there are no free data clusters
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soSymlink (const char *effPath, const char *ePath)
{
  soColorProbe (235, "07;31", "soSymlink (\"%s\", \"%s\")\n", effPath, ePath);

  /* insert your code here */

  return 0;
}
