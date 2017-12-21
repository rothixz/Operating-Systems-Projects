/**
 *  \file soRename.c (implementation file)
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
 *  \brief Change the name or the location of a file in the directory hierarchy of the file system.
 *
 *  It tries to emulate <em>rename</em> system call.
 *
 *  \param oldPath path to an existing file
 *  \param newPath new path to the same file in replacement of the old one
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the pointer to either of the strings is \c NULL or any of the path strings is a \c NULL string
 *                      or any of the paths do not describe absolute paths or <tt>oldPath</tt> describes a directory
 *                      and is a substring of <tt>newPath</tt> (attempt to make a directory a subdirectory of itself)
 *  \return -\c ENAMETOOLONG, if the paths names or any of their components exceed the maximum allowed length
 *  \return -\c ENOTDIR, if any of the components of both paths, but the last one, are not directories, or
 *                       <tt>oldPath</tt> describes a directory and <tt>newPath</tt>, although it exists, does not
 *  \return -\c EISDIR, if <tt>newPath</tt> describes a directory and <tt>oldPath</tt> does not
 *  \return -\c ELOOP, if either path resolves to more than one symbolic link
 *  \return -\c EMLINK, if <tt>oldPath</tt> is a directory and the directory containing <tt>newPath</tt> has already
 *                      the maximum number of links, or <tt>oldPath</tt> has already the maximum number of links and
 *                      is not contained in the same directory that will contain <tt>newPath</tt>
 *  \return -\c ENOENT, if no entry with a name equal to any of the components of <tt>oldPath</tt>, or to any of the
 *                      components of <tt>newPath</tt>, but the last one, is found
 *  \return -\c ENOTEMPTY, if both <tt>oldPath</tt> and <tt>newPath</tt> describe directories and <tt>newPath</tt> is
 *                         not empty
 *  \return -\c EACCES, if the process that calls the operation has not execution permission on any of the components
 *                      of both paths, but the last one
 *  \return -\c EPERM, if the process that calls the operation has not write permission on the directories where
 *                     <tt>newPath</tt> entry is to be added and <tt>oldPath</tt> is to be detached
 *  \return -\c ENOSPC, if there are no free data clusters
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soRename (const char *oldPath, const char *newPath)
{
  soColorProbe (227, "07;31", "soRename (\"%s\", \"%s\")\n", oldPath, newPath);

  int error;
  uint32_t oldInodeDir, oldInodeEnt;
  uint32_t newInodeDir, newInodeEnt;
  SOInode *p_oldEnt;
  SOInode *p_newEnt; //para onde inodeDir vai ser lido
  char *oldDirName;
  char *oldEntName;
  char *newDirName;
  char *newEntName;
  

  if((error = soGetDirEntryByPath(oldPath, &oldInodeDir,&oldInodeEnt)) != 0) //oldpath existe
    return error;

  char *oldPath1 = strdup(oldPath);    //dirname e basename :( 
  oldDirName = dirname(oldPath1);
  char *oldPath2 = strdup(oldPath);
  oldEntName = basename(oldPath2);

  char *newPath1 = strdup(newPath);
  newDirName = dirname(newPath1);
  char *newPath2 = strdup(newPath);
  newEntName = basename(newPath2);

  if(strcmp(oldDirName, newDirName) == 0){ //no mesmo dir
  	error = soGetDirEntryByPath(newPath, &newInodeDir,&newInodeEnt);
  	if(error == -ENOENT){ //newPath nao existe
  		if((error = soRenameDirEntry(oldInodeDir, oldEntName, newEntName)) != 0)
  			return error;
  		return 0;
  	}
  	else if(error != 0){ 
  		return error;
  	}

  	//basename(newPath) existe
  	p_oldEnt = (SOInode*) malloc(sizeof(SOInode));
  	if(p_oldEnt == NULL)
    	return -ELIBBAD;
  	if((error = soReadInode(p_oldEnt, oldInodeEnt)) !=0)   //ler oldEnt
  		return error;

  	p_newEnt = (SOInode*) malloc(sizeof(SOInode));
  	if(p_newEnt == NULL)
    	return -ELIBBAD;
    if((error = soReadInode(p_newEnt, newInodeEnt)) !=0)   //ler newEnt
  		return error;

  	if((p_oldEnt->mode & INODE_DIR) != 0){ //e Dir
  		
  		if((p_newEnt->mode & INODE_DIR) == 0) //newPath nao e dir e oldPath e dir
  			return -ENOTDIR;
  		if((error = soCheckDirectoryEmptiness(newInodeEnt)) != 0) //newPath nao esta vazia
  			return error;
  	}
  	if((p_newEnt->mode & INODE_DIR) != 0) //newPath e dir mas oldPath nao e
  		return -EISDIR;

  	if((error = soRemDetachDirEntry(newInodeDir, newEntName, DETACH)) != 0) //a entrada vai na dir table vai ser apagada
  		return error;

  	if((error = soRenameDirEntry(oldInodeDir, oldEntName, newEntName)) != 0){
  		return error;
  	}
    return 0;
  }
    //DIR diferentes
  if((error=soGetDirEntryByPath(newDirName, NULL, &newInodeDir)) != 0){ 
    return error;//este dir nao existe
  }
  error = soGetDirEntryByName(newInodeDir, newEntName, &newInodeEnt, NULL);
  if(error == -ENOENT)   //newEntName nao existe
  { 
    p_oldEnt = (SOInode*) malloc(sizeof(SOInode));
    if(p_oldEnt == NULL)
      return -ELIBBAD;
    if((error = soReadInode(p_oldEnt, oldInodeEnt)) !=0)   //ler oldEnt
      return error;
    if((p_oldEnt->mode & INODE_DIR) != 0){ //attach de um dir
       if((error = soAddAttDirEntry(newInodeDir, newEntName, oldInodeEnt, ATTACH)) != 0)
         return error;
     }
    else{ //add file
      if((error = soAddAttDirEntry(newInodeDir, newEntName, oldInodeEnt, ADD)) != 0)
        return error;
    }
    if((error = soRemDetachDirEntry(oldInodeDir, oldEntName, DETACH)) != 0)
      return error;
    return 0;
  }
  else if(error != 0){ 
    return error;
  }
  //em DIR diferentes e base(newPath) existe

  p_oldEnt = (SOInode*) malloc(sizeof(SOInode));
  if(p_oldEnt == NULL)
    return -ELIBBAD;
  if((error = soReadInode(p_oldEnt, oldInodeEnt)) !=0)   //ler oldEnt
    return error;
  p_newEnt = (SOInode*) malloc(sizeof(SOInode));
  if(p_newEnt == NULL)
     return -ELIBBAD;
  if((error = soReadInode(p_newEnt, newInodeEnt)) !=0)   //ler newEnt
   return error;

  if((p_oldEnt->mode & INODE_DIR) != 0){
    if((p_newEnt->mode & INODE_DIR) == 0) //newPath nao e dir e oldPath e dir
      return -ENOTDIR;
    if((error = soCheckDirectoryEmptiness(newInodeEnt)) != 0) //newPath nao esta vazia
      return error;
  }
  if((p_newEnt->mode & INODE_DIR) != 0) //newPath e dir mas oldPath nao e
    return -EISDIR;
  if((error = soRemDetachDirEntry(newInodeDir, newEntName, DETACH)) != 0) //a entrada na dir table vai ser apagada
    return error;

  if((p_oldEnt->mode & INODE_DIR) != 0){
    if((error = soAddAttDirEntry(newInodeDir, newEntName, oldInodeEnt, ATTACH)) != 0) //adicionar entrada na nova localizaçao
     return error;
  }
  else{
    if((error = soAddAttDirEntry(newInodeDir, newEntName, oldInodeEnt, ADD)) != 0) //adicionar entrada na nova localizaçao
     return error;
  }

  if((error = soRemDetachDirEntry(oldInodeDir, oldEntName, DETACH)) != 0) //entrada antiga e apagada
      return error;
  return 0;
}

