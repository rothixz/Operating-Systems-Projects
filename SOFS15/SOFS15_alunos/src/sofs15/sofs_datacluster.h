/**
 *  \file sofs_datacluster.h (interface file)
 *
 *  \brief Definition of the data cluster data type.
 *
 *  It specifies the file system metadata which describes the data cluster content.
 *
 *  \author António Rui Borges - August 2012
 */

#ifndef SOFS_DATACLUSTER_H_
#define SOFS_DATACLUSTER_H_

#include <stdint.h>

#include "sofs_const.h"
#include "sofs_direntry.h"

/** \brief reference to a null data cluster */
#define NULL_CLUSTER ((uint32_t)(~0UL))

/** \brief number of data cluster references per block */
#define RPB (BLOCK_SIZE / sizeof (uint32_t))

/** \brief size of the byte stream per data cluster */
#define BSLPC (CLUSTER_SIZE)

/** \brief number of data cluster references per data cluster */
#define RPC (CLUSTER_SIZE / sizeof (uint32_t))

/** \brief number of directory entries per data cluster */
#define DPC (CLUSTER_SIZE / sizeof (SODirEntry))

/**
 *  \brief Definition of the data cluster data type.
 *
 *  It describes the different interpretations for the information content of a data cluster in use.
 *
 *  It may either contain:
 *     \li a stream of bytes
 *     \li a sub-array of data cluster references
 *     \li a sub-array of directory entries.
 */

typedef union soDataClust
{
   /** \brief byte stream */
    unsigned char data[BSLPC];
   /** \brief sub-array of data cluster references */
    uint32_t ref[RPC];
   /** \brief sub-array of directory entries */
    SODirEntry de[DPC];
} SODataClust;

#endif /* SOFS_DATACLUSTER_H_ */
