/**
 *  \file sofs_ifuncs_2.h (interface file)
 *
 *  \brief Set of operations to manage inodes: level 2 of the internal file system organization.
 *
 *  The aim is to provide an unique description of the functions that operate at this level.
 *
 *  The operations are:
 *      \li read specific inode data from the table of inodes
 *      \li write specific inode data to the table of inodes
 *      \li check the inode access permissions against a given operation.
 *
 *  \author Artur Carneiro Pereira September 2008
 *  \author Miguel Oliveira e Silva September 2009
 *  \author António Rui Borges - September 2010 / September 2012
 *
 *  \remarks In case an error occurs, all functions return a negative value which is the symmetric of the system error
 *           or the local error that better represents the error cause. Local errors are out of the range of the
 *           system errors.
 */

#ifndef SOFS_IFUNCS_2_H_
#define SOFS_IFUNCS_2_H_

#include <stdint.h>

#include "sofs_inode.h"

/** \brief performing a read operation */
#define R  0x0004
/** \brief performing a write operation */
#define W  0x0002
/** \brief performing an execute operation */
#define X  0x0001

/**
 *  \brief Read specific inode data from the table of inodes.
 *
 *  The inode must be in use and belong to one of the legal file types.
 *  Upon reading, the <em>time of last file access</em> field is set to current time.
 *
 *  \param p_inode pointer to the buffer where inode data must be read into
 *  \param nInode number of the inode to be read from
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>buffer pointer</em> is \c NULL or the <em>inode number</em> is out of range
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

extern int soReadInode (SOInode *p_inode, uint32_t nInode);

/**
 *  \brief Write specific inode data to the table of inodes.
 *
 *  The inode must be in use and belong to one of the legal file types.
 *  Upon writing, the <em>time of last file modification</em> and <em>time of last file access</em> fields are set to
 *  current time.
 *
 *  \param p_inode pointer to the buffer containing the data to be written from
 *  \param nInode number of the inode to be written into
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>buffer pointer</em> is \c NULL or the <em>inode number</em> is out of range
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

extern int soWriteInode (SOInode *p_inode, uint32_t nInode);

/**
 *  \brief Check the inode access rights against a given operation.
 *
 *  The inode must to be in use and belong to one of the legal file types.
 *  It checks if the inode mask permissions allow a given operation to be performed.
 *
 *  When the calling process is <em>root</em>, access to reading and/or writing is always allowed and access to
 *  execution is allowed provided that either <em>user</em>, <em>group</em> or <em>other</em> have got execution
 *  permission.
 *
 *  \param nInode number of the inode
 *  \param opRequested operation to be performed:
 *                    a bitwise combination of R, W, and X
 *
 *  \return <tt>0 (zero)</tt>, on success
 *  \return -\c EINVAL, if the <em>inode number</em> is out of range or no operation of the defined class is described
 *  \return -\c EACCES, if the operation is denied
 *  \return -\c EIUININVAL, if the inode in use is inconsistent
 *  \return -\c ELDCININVAL, if the list of data cluster references belonging to an inode is inconsistent
 *  \return -\c EBADF, if the device is not already opened
 *  \return -\c EIO, if it fails reading or writing
 *  \return -\c ELIBBAD, if some kind of inconsistency was detected at some internal storage lower level
 *  \return -<em>other specific error</em> issued by \e lseek system call
 */

extern int soAccessGranted (uint32_t nInode, uint32_t opRequested);

#endif /* SOFS_IFUNCS_2_H_ */
