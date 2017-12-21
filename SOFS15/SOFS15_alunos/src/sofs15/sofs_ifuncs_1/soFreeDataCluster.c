/**
 *  \file soFreeDataCluster.c (implementation file)
 *
 *  \author António Rui Borges - September 2012
 */

/* Autor: Miguel Ferreira 72583 */

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

int soDeplete (SOSuperBlock *p_sb);

/**
 *  \brief Free the referenced data cluster.
 *
 *  The cluster is inserted into the insertion cache of free data cluster references. If the cache is full, it has to be
 *  depleted before the insertion may take place. It has to have been previouly allocated.
 *
 *  Notice that the first data cluster, supposed to belong to the file system root directory, can never be freed.
 *
 *  \param nClust logical number of the data cluster
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, the <em>data cluster number</em> is out of range
 *  \return -\c EDCNALINVAL, if the data cluster has not been previously allocated
 *  \return -\c ESBDZINVAL, if the data zone metadata in the superblock is inconsistent
 *  \return -\c ESBFCCINVAL, if the free data clusters caches in the superblock are inconsistent
 *  \return -\c EFCTINVAL, if the table of references to free data clusters is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soFreeDataCluster (uint32_t nClust)
{
    soColorProbe (614, "07;33", "soFreeDataCluster (%"PRIu32")\n", nClust);

    int error_status;
    SOSuperBlock* p_sb;
    uint32_t clust_status;

    /* Argument validation */
    if ( (error_status = soLoadSuperBlock()) != 0 ) return error_status;
    if ( (p_sb = soGetSuperBlock()) == NULL) return -EIO; 
    if ( (error_status = soQCheckSuperBlock(p_sb)) != 0)  return error_status;
    if ( nClust < 1 || nClust > p_sb -> dzone_total - 1 ) return -EINVAL;
    if ( (error_status = soQCheckStatDC(p_sb, nClust, &clust_status)) != 0 ) return error_status;
    if ( clust_status == FREE_CLT ) return -EDCNALINVAL;

    /* Free Cluster */
    if ( p_sb -> dzone_insert.cache_idx == DZONE_CACHE_SIZE ) /* Cache is full, it has to be depleted */
        if ( (error_status = soDeplete(p_sb)) != 0 )
            return error_status;
    p_sb -> dzone_insert.cache[ p_sb -> dzone_insert.cache_idx ] = nClust;
    p_sb -> dzone_insert.cache_idx += 1;
    p_sb -> dzone_free += 1;

    if ( (error_status = soStoreSuperBlock()) != 0 ) return error_status;
    return 0;
}

/**
 *  \brief Deplete the insertion cache of references to free data clusters.
 *
 *  \param p_sb pointer to a buffer where the superblock data is stored
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

int soDeplete (SOSuperBlock *p_sb)
{
    uint32_t index;
    uint32_t cycle;
    uint32_t nBlk;
    uint32_t offset;
    uint32_t *ref;
    uint32_t error_status;

    index = p_sb->tbfreeclust_tail;

    for ( cycle = 0; cycle < p_sb->dzone_insert.cache_idx; cycle++ )
    {
        if ( (error_status = soConvertRefFCT(index, &nBlk, &offset)) != 0 ) return error_status;
        if ( (error_status = soLoadBlockFCT(nBlk)) != 0 ) return error_status;
        if ( (ref = soGetBlockFCT()) == NULL ) return -EIO;


        ref[offset] = p_sb->dzone_insert.cache[cycle];
        p_sb -> dzone_insert.cache[cycle] = NULL_CLUSTER;
        index = (index + 1) % p_sb -> dzone_total;

        if ( (error_status = soStoreBlockFCT()) != 0 ) return error_status;

    }

    p_sb -> dzone_insert.cache_idx = 0;
    p_sb -> tbfreeclust_tail = index;

    return 0;
}
