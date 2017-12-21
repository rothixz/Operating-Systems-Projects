/**
 *  \file soHandleFileClusters.c (implementation file)
 *
 *  \author Carla Goncalves [71816]
 */

#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <stdlib.h>

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
 *  \brief Handle all data clusters from the list of references starting at a given point.
 *
 *  The file (a regular file, a directory or a symlink) is described by the inode it is associated to.
 *
 *  Only one operation (FREE) is available and can be applied to the file data clusters starting from the index to the
 *  list of direct references which is given.
 *
 *  The field <em>clucount</em> and the lists of direct references, single indirect references and double indirect
 *  references to data clusters of the inode associated to the file are updated.
 *
 *  Thus, the inode must be in use and belong to one of the legal file types.
 *
 *  \param nInode number of the inode associated to the file
 *  \param clustIndIn index to the list of direct references belonging to the inode which is referred (it contains the
 *                    index of the first data cluster to be processed)
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>inode number</em> or the <em>index to the list of direct references</em> are out of
 *                      range
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c EDCINVAL, if the data cluster header is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soHandleFileClusters (uint32_t nInode, uint32_t clustIndIn)
{
	soColorProbe (414, "07;31", "soHandleFileClusters (%"PRIu32", %"PRIu32")\n", nInode, clustIndIn);

	 //uint32_t i;
   SOInode p_inode;
   SOSuperBlock* p_sb; /*pointer to super block*/
   int error;
   SODataClust smpIndRefClust; /*double ref cluster*/
   SODataClust dirRefClust; 
   uint32_t dirRefindex;
   uint32_t smpIndRefindex;
   /*Load SB*/
	if((error=soLoadSuperBlock())!=0) 	return error; 	
  if((p_sb=soGetSuperBlock())==NULL)	return -EIO; 
   	/*Quick consistency check of Super Block*/

    /*check if the inode number is out of range*/
  if(nInode>=p_sb->itotal) return -EINVAL;

  if((error=soReadInode(&p_inode, nInode))!= 0) return error;
  /*check if the cluster index is within the valid range*/
  if(clustIndIn>=MAX_FILE_CLUSTERS) return -EINVAL;



  /*refs duplamente indirectas*/
  if(p_inode.i2!=NULL_CLUSTER){
    if((error=soLoadSngIndRefClust(p_sb->dzone_start+p_inode.i2*BLOCKS_PER_CLUSTER))!=0) return error;
    /*Get a pointer to the contents of a specific cluster of the table of single indirect references to data clusters. */
    smpIndRefClust = *soGetSngIndRefClust();
    
    if(clustIndIn<N_DIRECT+RPC)
      smpIndRefindex=0;
    else
      smpIndRefindex=(clustIndIn-N_DIRECT-RPC)/RPC;
    
    for(; smpIndRefindex<RPC; smpIndRefindex++){
      if(smpIndRefClust.ref[smpIndRefindex]!=NULL_CLUSTER){
        if((error=soLoadDirRefClust(p_sb->dzone_start+smpIndRefClust.ref[smpIndRefindex]*BLOCKS_PER_CLUSTER))!=0) return error;
    /*pointer to the contents of a specific cluster of the table of direct references to data clusters. */
        dirRefClust=*soGetDirRefClust();

        if(clustIndIn<N_DIRECT+RPC) 
          dirRefindex=0;
        else 
          dirRefindex=(clustIndIn-N_DIRECT-RPC)%RPC;

        for(; dirRefindex<RPC; dirRefindex++){
          if(dirRefClust.ref[dirRefindex]!=NULL_CLUSTER)
            if((error=soHandleFileCluster(nInode, N_DIRECT+RPC*(smpIndRefindex+1)+dirRefindex,FREE,NULL))!=0) return error;
        }
      }
    }
  }
  
   /*refs simplesmente indirectas*/
  if (p_inode.i1 != NULL_CLUSTER && clustIndIn < N_DIRECT + RPC){
    if ((error = soLoadDirRefClust(p_sb->dzone_start + p_inode.i1 * BLOCKS_PER_CLUSTER)) != 0) 
    return error;
    /*Get a pointer to the contents of a specific cluster of the table of direct references to data clusters. */
    dirRefClust = *soGetDirRefClust();
    
    if(clustIndIn < N_DIRECT)
      dirRefindex = 0;
    else
      dirRefindex = clustIndIn - N_DIRECT;
    
    for(; dirRefindex < RPC; dirRefindex++){
      if(dirRefClust.ref[dirRefindex] != NULL_CLUSTER)
        if((error = soHandleFileCluster(nInode, N_DIRECT + dirRefindex, FREE, NULL)) != 0) return error;
    }
  }
  /*refs directas*/
  if(clustIndIn < N_DIRECT)
    for (dirRefindex = clustIndIn; dirRefindex < N_DIRECT; dirRefindex++)

      if(p_inode.d[dirRefindex] != NULL_CLUSTER)
        
        if((error = soHandleFileCluster(nInode, dirRefindex, FREE, NULL)) != 0) return error;

  return 0;
}

