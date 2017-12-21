/**
 *  \file soAllocDataClustere.c (implementation file)
 *
 *  \author António Rui Borges - September 2012
 */

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include "sofs_probe.h"
#include "sofs_buffercache.h"
#include "sofs_superblock.h"
#include "sofs_inode.h"
#include "sofs_datacluster.h"
#include "sofs_basicoper.h"
#include "sofs_basicconsist.h"

/* Allusion to internal functions */

int soReplenish (SOSuperBlock *p_sb);
int soDeplete (SOSuperBlock *p_sb);

/**
 *  \brief Allocate a free data cluster.
 *
 *  The cluster is retrieved from the retrieval cache of free data cluster references. If the cache is empty, it has to
 *  be replenished before the retrieval may take place.
 *
 *  \param p_nClust pointer to the location where the logical number of the allocated data cluster is to be stored
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, the <em>pointer to the logical data cluster number</em> is \c NULL
 *  \return -\c ENOSPC, if there are no free data clusters
 *  \return -\c ESBDZINVAL, if the data zone metadata in the superblock is inconsistent
 *  \return -\c ESBFCCINVAL, if the free data clusters caches in the superblock are inconsistent
 *  \return -\c EFCTINVAL, if the table of references to free data clusters is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

/*Author: Carla Gonçalves 	71816*/
int soAllocDataCluster (uint32_t *p_nClust)
{
   soColorProbe (613, "07;33", "soAllocDataCluster (%p)\n", p_nClust);

   if(p_nClust==NULL)	return -EINVAL;

   SOSuperBlock* p_sb; /*pointer to super block*/
   int error;

   /*Load SB*/
   if((error=soLoadSuperBlock())!=0) 	return error; 	
   if((p_sb=soGetSuperBlock())==NULL)	return -EIO; 
   /*Quick consistency check of Super Block*/
   if((error=soQCheckSuperBlock(p_sb))!=0)	return error;	
   /*Quick consistency check of Data Zone (associated fields in the SB, 
   data clusters && table of references for free data clusters)*/
   if((error=soQCheckDZ(p_sb))!=0)		return error;	
   if(p_sb->dzone_free==0)				return -ENOSPC;	
   if(p_sb->dzone_retriev.cache_idx==DZONE_CACHE_SIZE){
      /* cache empty: replenish */
   		if((error=soReplenish(p_sb))!=0) return error;	
   }

   *p_nClust=p_sb->dzone_retriev.cache[p_sb->dzone_retriev.cache_idx]; /* p_nclust = content of cache @ index*/ ////////////////////////////
   /* increments index of retrieved cache */
   p_sb->dzone_retriev.cache_idx+=1; 
   /* decrement number of free data clusters*/
   p_sb->dzone_free-=1; 

   /*Verify & store SB */
   if((error=soStoreSuperBlock())!=0) return error; 

   return 0;
}

/**
 *  \brief Replenish the retrieval cache of references to free data clusters.
 *
 *  \param p_sb pointer to a buffer where the superblock data is stored
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soReplenish (SOSuperBlock *p_sb)
{
	uint32_t nclustt = (p_sb->dzone_free < DZONE_CACHE_SIZE) ? p_sb->dzone_free : DZONE_CACHE_SIZE;
   uint32_t index = p_sb->tbfreeclust_head; //tbfreeclust_head nao esta declarado...
   uint32_t n;
   uint32_t *ref;
   uint32_t error;
   uint32_t block;
   uint32_t offset;

   for (n = DZONE_CACHE_SIZE - nclustt; n < DZONE_CACHE_SIZE; n++){ 
      if (index == p_sb->tbfreeclust_tail) break;
      /*Get  references to Block and ref=soGetBlockFCT())==NULL_set from the index*/
      if((error=soConvertRefFCT(index, &block, &offset))!=0) return error;
      /*Load Block*/
      if((error=soLoadBlockFCT(block))!=0) return error;
      /*Get pointer to ref from the table of references to free data clusters*/
      if((ref=soGetBlockFCT())==NULL) return -EIO;
      p_sb->dzone_retriev.cache[n] = ref[offset];
      ref[offset]=NULL_CLUSTER;
      if((error=soStoreBlockFCT())!=0) return error;	
      index = (index + 1) % p_sb->dzone_total;
   }
   
   if (n != DZONE_CACHE_SIZE){ /* esvaziar a cache de inserção para se obter as referências restantes */
      if((error=soDeplete(p_sb))!=0) return error;
      
      for (; n < DZONE_CACHE_SIZE; n++){ 
         if((error=soConvertRefFCT(index, &block, &offset))!=0) return error;
         if((error=soLoadBlockFCT(block))!=0) return error;
         if((ref=soGetBlockFCT())==NULL) return -EIO;
         p_sb->dzone_retriev.cache[n] = ref[offset];
         ref[offset]=NULL_CLUSTER;
	 if((error=soStoreBlockFCT())!=0) return error;	
         index = (index + 1) % p_sb->dzone_total;
      }
   }

   p_sb->dzone_retriev.cache_idx = DZONE_CACHE_SIZE - nclustt;
   p_sb->tbfreeclust_head = index;

   return 0;
}
