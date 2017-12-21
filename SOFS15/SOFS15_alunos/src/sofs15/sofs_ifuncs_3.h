/**
 *  \file sofs_ifuncs_3.h (interface file)
 *
 *  \brief Set of operations to manage data clusters: level 3 of the internal file system organization.
 *
 *  The aim is to provide an unique description of the functions that operate at this level.
 *
 *  The operations are:
 *      \li read a specific data cluster
 *      \li write to a specific data cluster
 *      \li handle a file data cluster
 *      \li free all data clusters from the list of references starting at a given point.
 *
 *  \author Artur Carneiro Pereira September 2008
 *  \author Miguel Oliveira e Silva September 2009
 *  \author António Rui Borges - September 2010 / October 2012
 *
 *  \remarks In case an error occurs, all functions return a negative value which is the symmetric of the system error
 *           or the local error that better represents the error cause. Local errors are out of the range of the
 *           system errors.
 */

#ifndef SOFS_IFUNCS_3_H_
#define SOFS_IFUNCS_3_H_

#include <stdint.h>

#include "sofs_inode.h"

/** \brief operation get the logical number (or reference) of the referred data cluster */
#define GET         0
/** \brief operation  allocate a new data cluster and include it in the list of references of the inode which
 *                    describes the file */
#define ALLOC       1
/** \brief operation free the referred data cluster and dissociate it from the list of references of the inode
 *                   which describes the file */
#define FREE        2

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

extern int soReadFileCluster (uint32_t nInode, uint32_t clustInd, void *buff);

/**
 *  \brief Write a specific data cluster.
 *
 *  Data is written into a specific data cluster which is supposed to belong to an inode associated to a file (a regular
 *  file, a directory or a symbolic link). Thus, the inode must be in use and belong to one of the legal file types.
 *
 *  If the referred cluster has not been allocated yet, it will be allocated now so that the data can be stored as its
 *  contents.
 *
 *  \param nInode number of the inode associated to the file
 *  \param clustInd index to the list of direct references belonging to the inode where the reference to the data cluster
 *                  whose contents is to be written is stored
 *  \param buff pointer to the buffer where data must be written from
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>inode number</em> or the <em>index to the list of direct references</em> are out of
 *                      range or the <em>pointer to the buffer area</em> is \c NULL
 *  \return -\c ENOSPC, if there are no free data clusters
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

extern int soWriteFileCluster (uint32_t nInode, uint32_t clustInd, void *buff);

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
 *    \li ALLOC:      allocate a new data cluster and include it in list of references of the the inode which
 *                    describes the file
 *    \li FREE:       free the referred data cluster and dissociate it from the list of references of the inode
 *                    which describes the file.
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
 *  \return -\c ENOSPC, if there are no free data clusters
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c EDCARDYIL, if the referenced data cluster is already in the list of direct references (ALLOC)
 *  \return -\c EDCNOTIL, if the referenced data cluster is not in the list of direct references (FREE)
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

extern int soHandleFileCluster (uint32_t nInode, uint32_t clustInd, uint32_t op, uint32_t *p_outVal);

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

extern int soHandleFileClusters (uint32_t nInode, uint32_t clustIndIn);

#endif /* SOFS_IFUNCS_3_H_ */
