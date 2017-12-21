/**
 *  \file soWrite.c (implementation file)
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
 *  \brief Write data into an open regular file.
 *
 *  It tries to emulate <em>write</em> system call.
 *
 *  \param ePath path to the file
 *  \param buff pointer to the buffer where data to be written is stored
 *  \param count number of bytes to be written
 *  \param pos starting [byte] position in the file data continuum where data is to be written into
 *
 *  \return <em>number of bytes effectively written</em>, on success
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
 *  \return -\c ENOSPC, if there are no free data clusters
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soWrite (const char *ePath, void *buff, uint32_t count, int32_t pos)
{
  soColorProbe (230, "07;31", "soWrite (\"%s\", %p, %u, %u)\n", ePath, buff, count, pos);
  int error, i, nbytes;
  uint32_t nInodeDir;     //inode associado a directory 
  uint32_t nInodeEnt;     //inode associado a entry
  uint32_t clustInd;      //posicao do primeiro byte a escrever na tabela de referncias
  uint32_t offset;        //byte dentro do cluster de dados a escrever
  SOInode *p_Inode;       //ponteiro para o inode a modificar 
  SOSuperBlock *p_sb;     //ponteiro para o superbloco 
  char buff_temp[BSLPC];  //buffer contendo os bytes do cluster
  char* tmp;

  //load sb
  if((error = soLoadSuperBlock())!=0) return error; 
  if((p_sb = soGetSuperBlock())==NULL) return -ELIBBAD; 

  //numero do inode a partir do path 
  if((error = soGetDirEntryByPath(ePath, &nInodeDir, &nInodeEnt))!=0) 
    return error;

  if((pos+count)>=MAX_FILE_SIZE)
    return -EFBIG; 

  p_Inode = (SOInode*) malloc(sizeof(SOInode));
    	if(p_Inode == NULL)
      	return -ELIBBAD;
  //read inode 
  if((error = soReadInode(p_Inode, nInodeEnt))!=0)
    return error;
	
  //verificacao do mode do inode 
  if((p_Inode->mode & INODE_TYPE_MASK)==INODE_DIR)
    return -EISDIR;

  //verificar permissoes de escrita
  if((error = soAccessGranted(nInodeEnt, (uint32_t) 0x2))!=0) 
  {
    if(error==-EACCES)
      return -EPERM;
    else
      return error;
  }

  //caso tamanho do ficheiro seja menor -> atualiza o tamanho do ficheiro
  if(p_Inode->size<(pos+count))
    p_Inode->size=(pos+count);

  if((error = soWriteInode(p_Inode, nInodeEnt))!=0) //actualiza o inode
    return error;

  if((error = soConvertBPIDC(pos, &clustInd, &offset))!=0)
    return error;

  if((error=soReadFileCluster(nInodeEnt, clustInd, &buff_temp))!=0)
    return error;

  nbytes = 0;
  tmp = buff;

  for(i=0; i<count; i++)
  {
    if(offset==BSLPC)
    {
      if((error = soWriteFileCluster(nInodeEnt, clustInd, &buff_temp))!=0) //write no cluster -> actualizar cluster
        return error;
      clustInd++; 
      if((error = soReadFileCluster (nInodeEnt,clustInd, &buff_temp))!=0) //read cluster
        return error;
      offset = 0;
    }

    buff_temp[offset]=tmp[i];
    offset++;

    nbytes+=1;
  } 

  if((error = soWriteFileCluster(nInodeEnt, clustInd, &buff_temp))!=0) 
  //write do cluster de dados com a nova informacao caso estejamos na situacao em que 0<=offset<BSLPC
    return error;

  return nbytes;
}

