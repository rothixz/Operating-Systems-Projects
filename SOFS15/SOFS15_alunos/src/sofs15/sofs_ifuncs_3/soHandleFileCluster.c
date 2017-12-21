/**
 *  \file soHandleFileCluster.c (implementation file)
 *
 *  \author ---
 */

#include <stdio.h>
#include <inttypes.h>
#include <errno.h>

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

/* Allusion to internal functions */

int soHandleDirect (SOSuperBlock *p_sb, SOInode *p_inode, uint32_t nClust, uint32_t op, uint32_t *p_outVal);
int soHandleSIndirect (SOSuperBlock *p_sb, SOInode *p_inode, uint32_t nClust, uint32_t op, uint32_t *p_outVal);
int soHandleDIndirect (SOSuperBlock *p_sb, SOInode *p_inode, uint32_t nClust, uint32_t op, uint32_t *p_outVal);

/**
 *  \brief Handle of a file data cluster.
 *
 *  The file (a regular file, a directory or a symlink) is described by the inode it is associated to.
 *
 *  Several operations are available and can be applied to the file data cluster whose logical number is given.
 *
 *  The list of valid operations is
 *
 *    \li GET:        get the logical number (or reference) of the referred data cluster
 *    \li ALLOC:      allocate a new data cluster and associate it to the inode which describes the file
 *    \li FREE:       free the referred data cluster.
 *
 *  Depending on the operation, the field <em>clucount</em> and the lists of direct references, single indirect
 *  references and double indirect references to data clusters of the inode associated to the file are updated.
 *
 *  Thus, the inode must be in use and belong to one of the legal file types in all cases.
 *
 *  \param nInode number of the inode associated to the file
 *  \param clustInd index to the list of direct references belonging to the inode where the reference to the data cluster
 *                  is stored
 *  \param op operation to be performed (GET, ALLOC, FREE)
 *  \param p_outVal pointer to a location where the logical number of the data cluster is to be stored (GET / ALLOC);
 *                  in the other case (FREE) it is not used (it should be set to \c NULL)
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>inode number</em> or the <em>index to the list of direct references</em> are out of
 *                      range or the requested operation is invalid or the <em>pointer to outVal</em> is \c NULL when it
 *                      should not be (GET / ALLOC)
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c EDCARDYIL, if the referenced data cluster is already in the list of direct references (ALLOC)
 *  \return -\c EDCNOTIL, if the referenced data cluster is not in the list of direct references (FREE)
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

/* Author: Miguel Ferreira 72583, Fragata */
int soHandleFileCluster (uint32_t nInode, uint32_t clustInd, uint32_t op, uint32_t *p_outVal)
{
	soColorProbe (413, "07;31", "soHandleFileCluster (%"PRIu32", %"PRIu32", %"PRIu32", %p)\n", nInode, clustInd, op, p_outVal);
	/*---------------------	Variables	---------------------*/
	uint32_t error_status;	/* error  */
	SOInode iNode;			/* i-node data type */
	SOSuperBlock *p_sb;		/* SuperBlock pointer */
	//Validation
	/* Load of SuperBlock */
	if ((error_status = soLoadSuperBlock()) != 0 ) { return error_status; }
	if ((p_sb = soGetSuperBlock()) == NULL) { return -EIO; }
	/* INode is out of range */
	if ((nInode < 0) || (nInode > p_sb -> itotal - 1)) { return -EINVAL; }
	/* INode is valid */
	if ((error_status = soReadInode(&iNode, nInode)) != 0 ) { return error_status; }
	/* Index to direct references list is out of range */
	if ((clustInd < 0) || (clustInd > (N_DIRECT + RPC + RPC * RPC))) { return -EINVAL; }
	/* Validation of options */
	if (op != GET && op != ALLOC && op != FREE) { return -EINVAL; }
	/* Validation of p_outval */
	if (op == GET || op == ALLOC) { if (p_outVal == NULL) { return -EINVAL; } }
	if (op == FREE) { p_outVal = NULL; }
	/*---------------------	Consistency	---------------------*/
	/* INode table */
	if ((error_status = soQCheckInT(p_sb)) != 0 ) { return error_status; }
	/* data zone */
	if ((error_status = soQCheckDZ(p_sb)) != 0 ) { return error_status; }
	/*---------------------	Code		---------------------*/
	/* Direct reference */
	if (clustInd < N_DIRECT) { if ((error_status = soHandleDirect(p_sb, &iNode, clustInd, op, p_outVal)) != 0 ) { return error_status; } }
	/* Single indirect reference */
	else if (clustInd < (N_DIRECT + RPC)) { if ((error_status = soHandleSIndirect(p_sb, &iNode, clustInd, op, p_outVal)) != 0 ) { return error_status; } }
	/* Double indirect reference */
	else { if ((error_status = soHandleDIndirect(p_sb, &iNode, clustInd, op, p_outVal)) != 0 ) { return error_status; } }
	/* Writes the inode */
	if ((error_status = soWriteInode(&iNode, nInode)) != 0 ) { return error_status; }
	/* Stores the SuperBlock */
	if ((error_status = soStoreSuperBlock()) != 0 ) { return error_status; }
	return 0;
}

/**
 *  \brief Handle of a file data cluster whose reference belongs to the direct references list.
 *
 *  \param p_sb pointer to a buffer where the superblock data is stored
 *  \param p_inode pointer to a buffer which stores the inode contents
 *  \param clustInd index to the list of direct references belonging to the inode which is referred
 *  \param op operation to be performed (GET, ALLOC, FREE)
 *  \param p_outVal pointer to a location where the logical number of the data cluster is to be stored (GET / ALLOC);
 *                  in the other case (FREE) it is not used (it should be set to \c NULL)
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the requested operation is invalid
 *  \return -\c EDCARDYIL, if the referenced data cluster is already in the list of direct references (ALLOC)
 *  \return -\c EDCNOTIL, if the referenced data cluster is not in the list of direct references (FREE)
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */


//Fragata
int soHandleDirect (SOSuperBlock *p_sb, SOInode *p_inode, uint32_t clustInd, uint32_t op, uint32_t *p_outVal)
{

  uint32_t status;

  if(p_sb==NULL||p_inode==NULL) return -EINVAL;

  switch(op)
  {
    case ALLOC:
        
        if(p_inode->d[clustInd]==NULL_CLUSTER)
        {
            if(p_sb->dzone_free == 0) return -ELIBBAD;

            if( (status = soAllocDataCluster(p_outVal)) != 0 ) return status;

            p_inode->d[clustInd] = *p_outVal;
        
            p_inode->clucount++;
        }
        else //if not empty returns an error
          return -EDCARDYIL;
        
        break;

    case GET:

        *p_outVal = p_inode->d[clustInd];
        break;

    case FREE:


        if(p_inode->d[clustInd] == NULL_CLUSTER) return -EDCNOTIL;

        if ((status = soFreeDataCluster(p_inode->d[clustInd])) != 0)
          return status;
        
        p_inode->d[clustInd] = NULL_CLUSTER;
        p_inode->clucount--;
      
        break;

    default:
        return -EINVAL; break;

  }


  return 0;
}

/**
 *  \brief Handle of a file data cluster which belongs to the single indirect references list.
 *
 *  \param p_sb pointer to a buffer where the superblock data is stored
 *  \param p_inode pointer to a buffer which stores the inode contents
 *  \param clustInd index to the list of direct references belonging to the inode which is referred
 *  \param op operation to be performed (GET, ALLOC, FREE)
 *  \param p_outVal pointer to a location where the logical number of the data cluster is to be stored (GET / ALLOC);
 *                  in the other case (FREE) it is not used (it should be set to \c NULL)
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the requested operation is invalid
 *  \return -\c EDCARDYIL, if the referenced data cluster is already in the list of direct references (ALLOC)
 *  \return -\c EDCNOTIL, if the referenced data cluster is not in the list of direct references (FREE)
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

/* Author: Miguel Ferreira 72583 */
int soHandleSIndirect (SOSuperBlock *p_sb, SOInode *p_inode, uint32_t clustInd, uint32_t op, uint32_t *p_outVal)
{
	/*---------------------	Variables	---------------------*/
	uint32_t error_status;							/* error  */
	uint32_t index;									/* for index */
	uint32_t rel_position = clustInd - N_DIRECT;	/* index of cluster in direct references */
	SODataClust *p_dc;								/* pointer to cluster of direct */
	/*---------------------	Validation	---------------------*/
	if (p_sb == NULL || p_inode == NULL) { return -EINVAL; }	/*---------------------	Code		---------------------*/
	switch(op){
		case GET:
			/* Cluster in reference doesn't exist */
			if (p_inode -> i1 == NULL_CLUSTER) { *p_outVal = NULL_CLUSTER; }
			else
			{
				/* Load content of cluster */
				if ((error_status = soLoadDirRefClust( p_inode -> i1 * BLOCKS_PER_CLUSTER + (p_sb -> dzone_start))) != 0 ) { return error_status; }
				if ((p_dc = soGetDirRefClust()) == NULL ) { return -EIO; }
				/* Returns the reference to the cluster */
				*p_outVal = p_dc -> ref[rel_position];
			}
			break;
		case ALLOC:
			/* Cluster in reference doesn't exist */
			if (p_inode -> i1 == NULL_CLUSTER)
			{
				/* Checks if there are 2 free dataClusters */
				if (p_sb -> dzone_free <= 1) { return -ENOSPC; }
				if ((error_status = soAllocDataCluster(p_outVal)) != 0 ) { return error_status; }
				/* Puts in the reference to the cluster and increments the cluster count associated to inode */
				p_inode -> i1 = *p_outVal; p_inode -> clucount++;
				/* Loads the content of the cluster */
				if ((error_status = soLoadDirRefClust(p_sb -> dzone_start + BLOCKS_PER_CLUSTER * p_inode -> i1)) != 0 ) { return error_status; }
				if ((p_dc = soGetDirRefClust()) == NULL) { return -EIO; }
				/* Sets all references to NULL_CLUSTER */
				for (index = 0; index < RPC; index++) { p_dc -> ref[index] = NULL_CLUSTER; }
				/* Stores the cluster */
				if ((error_status = soStoreDirRefClust()) != 0 ) { return error_status; }
				/* Allocates the data cluster in use */
				if ((error_status = soAllocDataCluster(p_outVal)) != 0 ) { return error_status; }
				/* Loads the content of the references cluster */
				if ((error_status = soLoadDirRefClust(p_sb -> dzone_start + BLOCKS_PER_CLUSTER * p_inode -> i1)) != 0 ) { return error_status; }
				if ((p_dc = soGetDirRefClust()) == NULL) { return -EIO; }
				/* Puts in the reference to the cluster and increments the cluster count associated to inode */
				p_dc -> ref[rel_position] = *p_outVal; p_inode -> clucount++;
				/* Stores the cluster */
				if ((error_status = soStoreDirRefClust()) != 0 ) { return error_status; }
			}
			else
			{
				if (p_sb -> dzone_free <= 0) { return -ENOSPC; }
				/* Loads the content of the references cluster */
				if ((error_status = soLoadDirRefClust(p_sb -> dzone_start + BLOCKS_PER_CLUSTER * p_inode -> i1)) != 0 ) { return error_status; }
				if ((p_dc = soGetDirRefClust()) == NULL) { return -EIO; }
				/* Checks if there is a Cluster */
				if (p_dc -> ref[rel_position] == NULL_CLUSTER)
				{
					/* Allocates one */
					if ((error_status = soAllocDataCluster(p_outVal)) != 0 ) { return error_status; }
					/* Puts in the reference to the cluster and increments the cluster count associated to inode */
					p_dc -> ref[rel_position] = *p_outVal; p_inode -> clucount++;
					/* Stores the cluster */
					if ((error_status = soStoreDirRefClust()) != 0 ) { return error_status; }
				}
				else { return -EDCARDYIL; }
			}
			break;
		case FREE:
			if (p_inode -> i1 != NULL_CLUSTER)
			{
				/* Loads the content of the references cluster */
				if ((error_status = soLoadDirRefClust(p_sb -> dzone_start + BLOCKS_PER_CLUSTER * p_inode->i1)) != 0 ) { return error_status; }
				if ((p_dc = soGetDirRefClust()) == NULL) { return -EIO; }
				/* Checks if is NULL_CLUSTER */
				if ((p_dc -> ref[rel_position]) == NULL_CLUSTER ) { return -EDCNOTIL; }
				/* Free the cluster */
				if ((error_status = soFreeDataCluster(p_dc -> ref[rel_position])) != 0 ) { return error_status; }
				/* Removes the association and decrements the number of clusters associated to inode */
				p_dc -> ref[rel_position] = NULL_CLUSTER; p_inode -> clucount--;
				/* Stores the cluster */
				if ((error_status = soStoreDirRefClust()) != 0 ) { return error_status; }
				/* Checks if the there are any references left, if not removes the cluster */
				for (index = 0; index < RPC; index++) { if (p_dc -> ref[index] != NULL_CLUSTER) { break; } }
				if (index == RPC)
				{
					if ((error_status = soFreeDataCluster(p_inode -> i1)) != 0) { return error_status; }
					/* Sets to NULL_CLUSTER and decrements the number of clusters associated to inode */
					p_inode -> i1 = NULL_CLUSTER; p_inode -> clucount--;
				}
			}
			else { return -EDCNOTIL; }
			break;
		default:
			return -EINVAL;
			break;
	}
	return 0;
}

/**
 *  \brief Handle of a file data cluster which belongs to the double indirect references list.
 *
 *  \param p_sb pointer to a buffer where the superblock data is stored
 *  \param p_inode pointer to a buffer which stores the inode contents
 *  \param clustInd index to the list of direct references belonging to the inode which is referred
 *  \param op operation to be performed (GET, ALLOC, FREE)
 *  \param p_outVal pointer to a location where the logical number of the data cluster is to be stored (GET / ALLOC);
 *                  in the other case (FREE) it is not used (it should be set to \c NULL)
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the requested operation is invalid
 *  \return -\c EDCARDYIL, if the referenced data cluster is already in the list of direct references (ALLOC)
 *  \return -\c EDCNOTIL, if the referenced data cluster is not in the list of direct references (FREE)
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */



/* Author: Miguel Ferreira 72583, Fragata */
int soHandleDIndirect (SOSuperBlock *p_sb, SOInode *p_inode, uint32_t clustInd, uint32_t op, uint32_t *p_outVal)
{
  if (p_sb == NULL || p_inode == NULL)
    return -EINVAL;
  


  uint32_t status;    
  uint32_t i;       
  uint32_t tmp_clust;  //valor de um cluster de referencias indiretas
  
  //posicao dentro da tabela de single indirect references
  uint32_t indDInd = (clustInd-N_DIRECT-RPC)/RPC; 
  //posicao dentro da tabela de direct references
  uint32_t indSInd = (clustInd-N_DIRECT-RPC)%RPC;
  
  //ponteiros para um cluster de double-indirect-refs, single-indirect-refs e direct-refs
  SODataClust *p_dInd,*p_sInd,*p_dc;

  
  
  
  switch(op)
  {
    case GET:
    
      if (p_inode->i2 == NULL_CLUSTER)
        *p_outVal = NULL_CLUSTER;  //nao existe cluster na posiciao
      else
      { //busca a tabela de refs ind duplas
        if((status=soLoadSngIndRefClust(p_inode->i2* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
          return status;
        p_dInd = soGetSngIndRefClust();
        
        //guarda em tmp tal cluster
        tmp_clust = p_dInd->ref[indDInd];
        
        //se nao existe tabela de refs single ind
        if(tmp_clust==NULL_CLUSTER)
          *p_outVal = NULL_CLUSTER;
        else
        { //refs para a tabela de refs single ind
          if((status=soLoadDirRefClust(tmp_clust* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
            return status;

          if ((p_sInd = soGetDirRefClust()) == NULL)  return -EIO;
          //a tabela de single ref ind nao existe
          *p_outVal = p_sInd->ref[indSInd];

        }
      }
      break;
    case ALLOC:
      
      if (p_inode->i2 == NULL_CLUSTER)
      {
          //e preciso 3 DC para guardar
          if(p_sb->dzone_free <= 2)
            return -ENOSPC; 
          
        
        //alocar um DC para criar a tabela de refs indirectas e mete tudo a NULL-CLUSER
        if ((status = soAllocDataCluster(p_outVal)) != 0) return status;                
        
          p_inode->i2=*p_outVal;
          p_inode->clucount++;
          
        if ((status = soLoadSngIndRefClust(p_sb->dzone_start + BLOCKS_PER_CLUSTER * p_inode->i2)) != 0)
          return status;
          
        if ((p_dc = soGetSngIndRefClust()) == NULL)  return -EIO;
          
        for (i = 0;i<RPC;i++)
          p_dc->ref[i] = NULL_CLUSTER;
          
        if ((status = soStoreSngIndRefClust()) != 0)  return status;  
        
          
        //aloca um DC para criar a tabela de refs directas e mete o valor desse DC na posicao certa (ind_DInd)
        //na tabela de refs indirctas duplas
        if((status=soAllocDataCluster(&tmp_clust))!=0)               
        { if(status == -ENOSPC )
          return status;
        }
        
        
        p_inode->clucount++;
        
        if((status=soLoadSngIndRefClust(p_inode->i2* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
          return status;
        if ((p_dInd = soGetSngIndRefClust()) == NULL)
          return -EIO;
          
        p_dInd->ref[indDInd] = tmp_clust;

        if((status=soStoreSngIndRefClust())!=0)
          return status;
        
        
        //mete o que esta na tabela de refs directas a NULL CLUSTER
        if((status=soLoadDirRefClust(tmp_clust* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
          return status;
        if ((p_sInd = soGetDirRefClust()) == NULL)
          return -EIO;
        
        for (i=0; i<RPC; i++)
          p_sInd->ref[i] = NULL_CLUSTER;

        if((status=soStoreDirRefClust())!=0)
          return status;
        
        
        
        //alloc um DC para lá guardar a tabela de refs directas
        if((status=soAllocDataCluster(p_outVal))!=0)
          return status;
        
        p_inode -> clucount++;

        if((status=soLoadSngIndRefClust(p_inode->i2* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
          return status;

        if ((p_dInd = soGetSngIndRefClust()) == NULL)
          return -EIO;
      
        if((status=soLoadDirRefClust(tmp_clust* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
          return status;

        if ((p_sInd = soGetDirRefClust()) == NULL)
          return -EIO;
        
        p_sInd->ref[indSInd] = *p_outVal;

        if((status=soStoreDirRefClust())!=0)
          return status;

        if((status=soStoreSngIndRefClust())!=0)
          return status;
        
      }
      else
      { 
          if(p_sb->dzone_free <= 1 )return -ENOSPC;
        
        
        if((status=soLoadSngIndRefClust(p_inode->i2* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
          return status;

        if ((p_dInd = soGetSngIndRefClust()) == NULL)
          return -EIO;
        
        tmp_clust=p_dInd->ref[indDInd];
        
        if (tmp_clust == NULL_CLUSTER)
        { 
          if((status=soStoreSngIndRefClust())!=0)  return status;
          
          if((status=soAllocDataCluster(p_outVal))!=0) return status;           
          tmp_clust= *p_outVal;
          
          p_inode -> clucount++;
          
          
          if((status=soLoadSngIndRefClust(p_inode->i2* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
            return status;

          if ((p_dInd = soGetSngIndRefClust()) == NULL)  return -EIO;
    
          p_dInd->ref[indDInd] = tmp_clust;

          if((status=soStoreSngIndRefClust())!=0)  return status;
         
          
          
          if((status=soLoadDirRefClust(tmp_clust* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
            return status;

          if ((p_sInd = soGetDirRefClust()) == NULL)
            return -EIO;
          
          for (i=0; i<RPC; i++)
            p_sInd->ref[i] = NULL_CLUSTER;

          if((status=soStoreDirRefClust())!=0)
            return status;
          
          
          //aloca um dc para la guardar a tabela de refs directas
          if((status=soAllocDataCluster(p_outVal))!=0)  return status;
            
          
          p_inode -> clucount++;

          if((status=soLoadSngIndRefClust(p_inode->i2* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
            return status;

          if ((p_dInd = soGetSngIndRefClust()) == NULL)  return -EIO;

          if((status=soLoadDirRefClust(tmp_clust* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
            return status;

          if ((p_sInd = soGetDirRefClust()) == NULL)  return -EIO;
        
          p_sInd->ref[indSInd] = *p_outVal;

          if((status=soStoreDirRefClust())!=0)  return status;
          if((status=soStoreSngIndRefClust())!=0)  return status;
          
        }
        else //tmp_clust != NULL_CLUSTER
        { 
          if((status=soLoadDirRefClust(tmp_clust* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
            return status;
            
          if ((p_sInd = soGetDirRefClust()) == NULL) return -EIO;
          
          
          if (p_sInd->ref[indSInd] != NULL_CLUSTER )  return -EDCARDYIL;
          
          
          if((status=soStoreDirRefClust())!=0)  return status;
          if((status=soStoreSngIndRefClust())!=0)  return status;
          
          
          if((status=soAllocDataCluster(p_outVal))!=0)  return status;  
          
          p_inode -> clucount++;
          
          
          if((status=soLoadSngIndRefClust(p_inode->i2* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
            return status;
          if ((p_dInd = soGetSngIndRefClust()) == NULL)
            return -EIO;  
          if((status=soLoadDirRefClust(tmp_clust* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
            return status;
          if ((p_sInd = soGetDirRefClust()) == NULL)
            return -EIO;

          p_sInd->ref[indSInd] = *p_outVal;

          if((status=soStoreDirRefClust())!=0)  return status;

          if((status=soStoreSngIndRefClust())!=0) return status;
        }   
      }
      break;

    case FREE:
      
      if(p_inode->i2 != NULL_CLUSTER)
      { 
        if((status=soLoadSngIndRefClust(p_inode->i2* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
          return status;
        if ((p_dInd = soGetSngIndRefClust()) == NULL)
          return -EIO;
        
        tmp_clust=p_dInd->ref[indDInd];
        
        if (tmp_clust != NULL_CLUSTER)
        {
          if((status=soLoadDirRefClust(tmp_clust* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
            return status;
          if ((p_sInd = soGetDirRefClust()) == NULL)
            return -EIO;
          
          if (p_sInd->ref[indSInd] == NULL_CLUSTER )  return -EDCNOTIL; 
          
          if((status=soFreeDataCluster(p_sInd->ref[indSInd]))!=0)
            return status;
          
           
          p_sInd->ref[indSInd]=NULL_CLUSTER;
          
        
          p_inode->clucount--;

          if((status=soStoreDirRefClust())!=0)
            return status;
          
          
          if((status=soLoadDirRefClust(tmp_clust* BLOCKS_PER_CLUSTER + (p_sb->dzone_start)))!=0)
            return status;

          if ((p_sInd = soGetDirRefClust()) == NULL)
            return -EIO;
          
          for (i = 0; i < RPC; i++)
          {
            if (p_sInd-> ref[i] != NULL_CLUSTER )
              break;
          }

          if (i == RPC)
          {
            if((status=soFreeDataCluster(p_dInd -> ref[indDInd]))!=0)
              return status;
      
            p_dInd->ref[indDInd] = NULL_CLUSTER;
            p_inode -> clucount--;    
          }
          
          if((status=soStoreSngIndRefClust())!=0)  return status;
          
          if ((status = soLoadSngIndRefClust( p_inode -> i2  * BLOCKS_PER_CLUSTER + (p_sb->dzone_start))) != 0)
            return status;

          if ((p_dInd = soGetSngIndRefClust()) == NULL)
            return -EIO;

          for (i = 0; i < RPC; i++)
          {
            if (p_dInd-> ref[i] != NULL_CLUSTER )
              break;  
          }

          if (i == RPC)
          {
            if((status=soFreeDataCluster(p_inode -> i2))!=0)
              return status;

            p_inode -> i2 = NULL_CLUSTER;
            p_inode -> clucount--;
          }
          
        }
        else
          return -EDCNOTIL;
      }
      else
        return -EDCNOTIL;
      break;
    default:
      return -EINVAL; 
      break;
  }
  
  return 0;

}
