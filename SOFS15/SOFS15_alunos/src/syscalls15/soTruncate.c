/**
 *  \file soTruncate.c (implementation file)
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
 *  \brief Truncate a regular file to a specified length.
 *
 *  It tries to emulate <em>truncate</em> system call.
 *
 *  \param ePath path to the file
 *  \param length new size for the regular size
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the pointer to the string is \c NULL or or the path string is a \c NULL string or the path does
 *                      not describe an absolute path
 *  \return -\c ENAMETOOLONG, if the path name or any of its components exceed the maximum allowed length
 *  \return -\c ENOTDIR, if any of the components of <tt>ePath</tt>, but the last one, is not a directory
 *  \return -\c EISDIR, if <tt>ePath</tt> describes a directory
 *  \return -\c ELOOP, if the path resolves to more than one symbolic link
 *  \return -\c ENOENT, if no entry with a name equal to any of the components of <tt>ePath</tt> is found
 *  \return -\c EFBIG, if the file may grow passing its maximum size
 *  \return -\c EACCES, if the process that calls the operation has not execution permission on any of the components
 *                      of <tt>ePath</tt>, but the last one
 *  \return -\c EPERM, if the process that calls the operation has not write permission on the file described by
 *                     <tt>ePath</tt>
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soTruncate (const char *ePath, off_t length)
{
  soColorProbe (231, "07;31", "soTruncate (\"%s\", %u)\n", ePath, length);

  int error, i;
  uint32_t nInodeEnt, nClust, offset;
  SOInode inode;
  char dc[BSLPC];

  /*get the corresponding file*/
  if((error=soGetDirEntryByPath(ePath, NULL, &nInodeEnt))!=0) return error;
  /*get the file inode info*/
  if((error=soReadInode(&inode, nInodeEnt))!=0) return error;
  if((inode.mode & INODE_DIR) == INODE_DIR) return -EISDIR;

  if(length>MAX_FILE_SIZE) return -EFBIG;

  /*check permissions*/
  if((error=soAccessGranted(nInodeEnt, W))!=0) return error;

  nClust=length/BSLPC;	/*last clust*/
  offset=length%CLUSTER_SIZE;	/*last byte*/

  if(length<inode.size){
  	if(offset!=0){
  		if((error=soReadFileCluster(nInodeEnt, nClust, &dc))!=0) return error;	/*free all clusters after the last*/

  		for(i=offset; i<BSLPC; i++){
  			dc[i]=0x00;
  		}

  		if((error=soWriteFileCluster(nInodeEnt, nClust, &dc))!=0) return error;
  		if((error=soHandleFileClusters(nInodeEnt, nClust+1))!=0) return error;
  	}
  	else{
  		if((error=soHandleFileClusters(nInodeEnt, nClust))!=0) return error;
  	}
  }

  /*UPDATE INODE*/

  inode.size=length;
  if((error=soWriteInode(&inode, nInodeEnt))!=0) return error;

  return 0;
}
