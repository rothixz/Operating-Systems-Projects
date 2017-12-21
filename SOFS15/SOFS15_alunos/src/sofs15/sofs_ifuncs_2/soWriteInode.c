/**
 *  \file soWriteInode.c (implementation file)
 *
 *  \author ---
 */

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "sofs_probe.h"
#include "sofs_buffercache.h"
#include "sofs_superblock.h"
#include "sofs_inode.h"
#include "sofs_basicoper.h"
#include "sofs_basicconsist.h"

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

int soWriteInode (SOInode *p_inode, uint32_t nInode)
{
  soColorProbe (512, "07;31", "soWriteInode (%p, %"PRIu32")\n", p_inode, nInode);

  int error;
  SOSuperBlock* p_sb;

  /* Validacao do superbloco e ter ponteiro para tal */
  if((error=soLoadSuperBlock())!=0) return error;
  p_sb = soGetSuperBlock();

  /* Validacao do inode */
  if(p_inode == NULL) return -EINVAL;
  if(nInode>=p_sb->itotal) return -EINVAL;

  uint32_t NBLK, offset;
  SOInode *p_write;

  /* Carregar para p_write */
  if((error = soConvertRefInT(nInode, &NBLK, &offset)) != 0 ) return error;
  if((error = soLoadBlockInT(NBLK)) != 0 ) return error;
  p_write = soGetBlockInT();
  p_write += offset;

  /* Verificacao de consistencia */ 

  if((error = soQCheckInodeIU(p_sb, p_inode)) != 0) return error;

  memcpy(p_write, p_inode, sizeof(SOInode));  

  /* Set time of last file modification and of the last file access */
  p_write->vD1.atime = p_write->vD2.mtime = time(NULL);

  if((error = soStoreBlockInT()) != 0) return error;

  return 0;
}