/**
 *  \file sofs_basicconsist.h (interface file)
 *
 *  \brief Set of operations to check the basic consistency of the file system SOFS15 internal data structures.
 *
 *
 *  The aim is to provide a library with functionalities to check metadata consistency issues when the file system is
 *  in operation.
 *
 *  The operations are:
 *      \li get file system magic number
 *      \li quick check of the superblock metadata
 *      \li quick check of the table of inodes metadata
 *      \li quick check of the data zone metadata
 *      \li quick check of a free inode
 *      \li quick check of an inode in use
 *      \li quick check of the list of data cluster references belonging to an inode in use
 *      \li quick check of the allocation status of a data cluster
 *      \li quick check of the contents of a directory
 *      \li get error message.
 *
 *  \author António Rui Borges - September 2010 / September 2012
 *
 *  \remarks In case an error occurs, all functions return a negative value which is the symmetric of the system error
 *           or the local error that better represents the error cause. Local errors are out of the range of the
 *           system errors and their codes are listed bellow.
 */

#ifndef SOFS_BASICCONSIST_H_
#define SOFS_BASICCONSIST_H_

#include <stdbool.h>

#include "sofs_superblock.h"
#include "sofs_inode.h"

/** \brief status allocated of a data cluster */
#define ALLOC_CLT    0
/** \brief status free of a data cluster */
#define FREE_CLT     1

/* List of error codes */
/** \brief superblock header data is inconsistent */
#define ESBHINVAL            512
/** \brief table of inodes metadata in the superblock is inconsistent */
#define ESBTINPINVAL         513
/** \brief double-linked list of free inodes is inconsistent */
#define ETINDLLINVAL         514
/** \brief free inode is inconsistent */
#define EFININVAL            515
/** \brief data zone metadata in the superblock is inconsistent */
#define ESBDZINVAL           516
/** \brief free data clusters caches in the superblock are inconsistent */
#define ESBFCCINVAL          517
/** \brief table of references to free data clusters is inconsistent */
#define EFCTINVAL            518
/** \brief inode in use is inconsistent */
#define EIUININVAL           519
/** \brief data cluster is not allocated */
#define EDCNALINVAL          520
/** \brief list of data cluster references belonging to the inode is inconsistent */
#define ELDCININVAL          521
/** \brief directory is inconsistent */
#define EDIRINVAL            522
/** \brief directory entry is inconsistent */
#define EDEINVAL             523
/** \brief data cluster in the list of direct references for the given index */
#define EDCARDYIL            524
/** \brief data cluster not in the list of direct references for the given index */
#define EDCNOTIL             525
/** \brief path is relative and it is not a symbolic link */
#define ERELPATH             526

/**
 *  \brief Get file system magic number.
 *
 *  The value stored in the <tt>magic</tt> field of the superblock is returned.
 *
 *  \param p_sb pointer to a buffer where the superblock data is stored
 *
 *  \return <em>magic number</em>, on success
 *  \return \c 0xFFFF, if the pointer to the superblock is \c NULL
 */

extern uint32_t soGetMagicNumber (SOSuperBlock *p_sb);

/**
 *  \brief Get error message.
 *
 *  The error message matching a given error code is fetched.
 *
 *  \param code error code
 *
 *  \return <em>message error</em>, on success
 *  \return <em>unknown</em>, if the error code is out of range
 */

extern const char *soGetErrorMessage (int code);

/**
 *  \brief Quick check of the superblock metadata.
 *
 *  The file system layout as described in the superblock header is checked first. Then follows the checking of the
 *  metadata of the table of inodes and the data zone.
 *
 *  \param p_sb pointer to a buffer where the superblock data is stored
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the pointer to the superblock is \c NULL
 *  \return -\c ESBHINVAL, if the superblock header data is inconsistent
 *  \return -\c ESBTINPINVAL, if the table of inodes metadata in the superblock is inconsistent
 *  \return -\c ETINDLLINVAL, if the double-linked list of free inodes is inconsistent
 *  \return -\c EFININVAL, if the free inode is inconsistent
 *  \return -\c ESBDZINVAL, if the data zone metadata in the superblock is inconsistent
 *  \return -\c ESBFCCINVAL, if the free data clusters caches in the superblock are inconsistent
 *  \return -\c EFCDLLINVAL, if the double-linked list of free data clusters is inconsistent
 *  \return -\c EDCINVAL, if the data cluster header is inconsistent
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on reading or writing
 *  \return -\c ELIBBAD, if the buffercache is inconsistent or the superblock or a data block was not previously loaded
 *                       on a previous store operation
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

extern int soQCheckSuperBlock (SOSuperBlock *p_sb);

/**
 *  \brief Quick check of the table of inodes metadata.
 *
 *  Both the associated fields in the superblock and the table of inodes are checked for consistency. All fields must
 *  have legal values. In particular, the number of free inodes computed through inspection of the list of free
 *  inodes must match the value stored in the related field of the superblock.
 *
 *  \param p_sb pointer to a buffer where the superblock data is stored
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the pointer is \c NULL
 *  \return -\c ESBTINPINVAL, if the table of inodes metadata in the superblock is inconsistent
 *  \return -\c ETINDLLINVAL, if the double-linked list of free inodes is inconsistent
 *  \return -\c EFININVAL, if the free inode is inconsistent
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on reading or writing
 *  \return -\c ELIBBAD, if the buffercache is inconsistent or the superblock or a data block was not previously loaded
 *                       on a previous store operation
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

extern int soQCheckInT (SOSuperBlock *p_sb);

/**
 *  \brief Quick check of the data zone metadata.
 *
 *  Both the associated fields in the superblock, the two data clusters and the table of references to free data clusters
 *  are checked for consistency. All fields must have legal values. In particular, the number of free data clusters
 *  computed through inspection of the two free data cluster caches and the table of references to free data clusters must
 *  match the value stored in the related field of the superblock. The contents of the two caches and the table of
 *  references are also checked: the references stored there must be legal.
 *
 *  \param p_sb pointer to a buffer where the superblock data is stored
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the pointer is \c NULL
 *  \return -\c ESBDZINVAL, if the data zone metadata in the superblock is inconsistent
 *  \return -\c ESBFCCINVAL, if the free data clusters caches in the superblock are inconsistent
 *  \return -\c EFCTINVAL, if the table of references to free data clusters is inconsistent
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on reading or writing
 *  \return -\c ELIBBAD, if the buffercache is inconsistent or the superblock or a data block was not previously loaded
 *                       on a previous store operation
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

extern int soQCheckDZ (SOSuperBlock *p_sb);

/**
 *  \brief Quick check of a free inode.
 *
 *  The contents of the <tt>mode</tt> and <tt>refcount</tt> fields of the inode are checked for consistency.
 *
 *  \param p_inode pointer to a buffer where the inode is stored
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the pointer is \c NULL
 *  \return -\c EFININVAL, if the free inode is inconsistent
 */

extern int soQCheckFInode (SOInode *p_inode);

/**
 *  \brief Quick check of an inode in use.
 *
 *  The contents of all the fields of the inode, except <tt>owner</tt>, <tt>group</tt>, <tt>size</tt> and
 *  <tt>refcount</tt>, are checked for consistency. Only legal values are allowed.
 *
 *  \param p_sb pointer to a buffer where the superblock is stored
 *  \param p_inode pointer to a buffer where the inode is stored
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if any of the pointers is \c NULL
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on reading or writing
 *  \return -\c ELIBBAD, if the buffercache is inconsistent or the superblock or a data block was not previously loaded
 *                       on a previous store operation
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

extern int soQCheckInodeIU (SOSuperBlock *p_sb, SOInode *p_inode);

/**
 *  \brief Quick check of the list of data cluster references belonging to an inode in use.
 *
 *  All the clusters referenced in the list must have legal logical numbers and be allocated and the <tt>clucount</tt>
 *  field must have a value set in accordance to the filling of the list.
 *
 *  \param p_sb pointer to a buffer where the superblock is stored
 *  \param p_inode pointer to a buffer where the inode is stored
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if either of the pointers is \c NULL
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to the inode is inconsistent
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on reading or writing
 *  \return -\c ELIBBAD, if the buffercache is inconsistent or the superblock or a data block was not previously loaded
 *                       on a previous store operation
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

extern int soQCheckLRDC (SOSuperBlock *p_sb, SOInode *p_inode);

/**
 *  \brief Quick check of the allocation status of a data cluster.
 *
 *  The caches and the table of references to free data clusters are searched to check if they include the reference to
 *  the data cluster. If the reference is there, the data cluster is marked as free; otherwise, it marked as allocated.
 *
 *  The status is returned in the following way
 *     \li <tt>ALLOC_CLT</tt>, if the data cluster is allocated
 *     \li <tt>FREE_CLT</tt>, if the data cluster is free.
 *
 *  \param p_sb pointer to a buffer where the superblock is stored
 *  \param nClust logical number of the data cluster
 *  \param p_stat pointer to a location where the allocation status is stored on success
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if either of the pointers is \c NULL or the logical number of the data cluster is out of range
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on reading or writing
 *  \return -\c ELIBBAD, if the buffercache is inconsistent or the superblock or a data block was not previously loaded
 *                       on a previous store operation
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

extern int soQCheckStatDC (SOSuperBlock *p_sb, uint32_t nClust, uint32_t *p_stat);

/**
 *  \brief Quick check of the contents of a directory.
 *
 *  The inode, which is passed as an argument, should be in use and represent a directory.
 *  The contents of the entries of the directory are checked for consistency. Only legal values are allowed, meaning
 *  that each directory entry is either in use or is free; the first two entries must have names "." and ".." respectively.
 *
 *  \param p_sb pointer to a buffer where the superblock is stored
 *  \param p_inode pointer to a buffer where the inode (supposedly represents a directory) is stored
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if any of the pointers is \c NULL
 *  \return -\c ENOTDIR, if it is not a directory
 *  \return -\c EDIRINVAL, if it the directory is inconsistent
 *  \return -\c EDEINVAL, if it the directory entry is inconsistent
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails on reading or writing
 *  \return -\c ELIBBAD, if the buffercache is inconsistent or the superblock or a data block was not previously loaded
 *                       on a previous store operation
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

extern int soQCheckDirCont (SOSuperBlock *p_sb, SOInode *p_inode);

#endif /* SOFS_BASICCONSIST_H_ */
