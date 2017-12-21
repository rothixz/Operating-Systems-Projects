/**
 *  \file soReadFileCluster.c (implementation file)
 *
 *  \author ---
 */

#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "sofs_probe.h"
#include "sofs_buffercache.h"
#include "sofs_superblock.h"
#include "sofs_inode.h"
#include "sofs_datacluster.h"
#include "sofs_basicoper.h"
#include "sofs_basicconsist.h"
#include "sofs_ifuncs_1.h"
#include "sofs_ifuncs_2.h"

/** \brief operation get the logical number of the referenced data cluster */
#define GET         0
/** \brief operation  allocate a new data cluster and include it into the list of references of the inode which
 *                    describes the file */
#define ALLOC       1
/** \brief operation free the referenced data cluster and dissociate it from the inode which describes the file */
#define FREE        2

/* Allusion to external function */

int soHandleFileCluster (uint32_t nInode, uint32_t clustInd, uint32_t op, uint32_t *p_outVal);

/**
 *  \brief Read a specific data cluster.
 *
 *  Data is read from a specific data cluster which is supposed to belong to an inode associated to a file (a regular
 *  file, a directory or a symbolic link). Thus, the inode must be in use and belong to one of the legal file types.
 *
 *  If the referred cluster has not been allocated yet, the returned data will consist of a byte stream filled with the
 *  character null (ascii code 0).
 *
 *  \param nInode number of the inode associated to the file
 *  \param clustInd index to the list of direct references belonging to the inode where the reference to the data cluster
 *                  whose contents is to be read is stored
 *  \param buff pointer to the buffer where data must be read into
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>inode number</em> or the <em>index to the list of direct references</em> are out of
 *                      range or the <em>pointer to the buffer area</em> is \c NULL
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soReadFileCluster (uint32_t nInode, uint32_t clustInd, void *buff)
{
  soColorProbe (411, "07;31", "soReadFileCluster (%"PRIu32", %"PRIu32", %p)\n", nInode, clustInd, buff);

  SOSuperBlock* p_sb;       /* Pointer to SuperBlock */
  SOInode inode;            /* Inode */  
  uint32_t outVal;          /* Logical number of data cluster */
  uint32_t clustFn;         /* Physical number of data cluster*/
  int error;                /* Variable to check the consistence */
     
  /* Get SuperBlock */
  if((error = soLoadSuperBlock())!=0) return error;
  if((p_sb = soGetSuperBlock())==NULL) return -EINVAL;
   
  /* Check arguments */
  if(nInode>=(p_sb->itotal) || clustInd>=MAX_FILE_CLUSTERS || buff==NULL) return -EINVAL;
   
  /* Read, verificacao de uso e validacao do inode*/
  if((error = soReadInode(&inode, nInode))!=0) return error;
   
  /* Logical Number of Data Cluster */
  if((error = soHandleFileCluster(nInode, clustInd, GET, &outVal))!=0) return error;

  /* Check cluster allocated */
  if(outVal==NULL_CLUSTER){
     /* for(i=0;i < BSLPC;i++){}*/
     memset(buff, '\0', sizeof(unsigned char) * BSLPC);  //com memset e melhor
  /* caso esteja */ 
  }else{
      clustFn = p_sb->dzone_start + outVal * BLOCKS_PER_CLUSTER;  //tem de ser com o valor fisico nao com o logico
      error = soReadCacheCluster(clustFn, buff);
      if(error != 0)
        return error;
  }  
   
  return 0;
}